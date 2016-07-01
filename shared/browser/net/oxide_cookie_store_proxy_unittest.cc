// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/synchronization/condition_variable.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/time/time.h"
#include "net/cookies/cookie_constants.h"
#include "net/cookies/cookie_monster.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

#include "oxide_cookie_store_proxy.h"

namespace oxide {

namespace {

void InitializeCookieStore(CookieStoreOwner* store_owner,
                           scoped_refptr<net::CookieMonsterDelegate> delegate,
                           base::Lock* lock,
                           base::ConditionVariable* cv) {
  store_owner->set_store(
      base::MakeUnique<net::CookieMonster>(nullptr, delegate.get()));
  base::AutoLock al(*lock);
  cv->Signal();
}

void TeardownCookieStore(CookieStoreOwner* store_owner,
                         base::Lock* lock,
                         base::ConditionVariable* cv) {
  store_owner->set_store(nullptr);
  base::AutoLock al(*lock);
  cv->Signal();
}

}

class CookieStoreCallbackBase {
 public:
  CookieStoreCallbackBase()
      : called_(false) {
    thread_checker_.DetachFromThread();
  }

  virtual ~CookieStoreCallbackBase() = default;

  bool WaitForCallback() {
    EXPECT_TRUE(thread_checker_.CalledOnValidThread());

    if (called_) {
      return true;
    }

    base::RunLoop run_loop;
    quit_closure_ = run_loop.QuitClosure();
    base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
        FROM_HERE,
        quit_closure_,
        base::TimeDelta::FromSeconds(5));
    run_loop.Run();

    quit_closure_.Reset();

    return called_;
  }

 protected:
  void Reset() {
    EXPECT_TRUE(thread_checker_.CalledOnValidThread());
    called_ = false;
  }

  void NotifyCalled() {
    EXPECT_TRUE(thread_checker_.CalledOnValidThread());
    ASSERT_FALSE(called_);
    called_ = true;

    if (quit_closure_.is_null()) {
      return;
    }

    quit_closure_.Run();
  }

 private:
  base::ThreadChecker thread_checker_;
  bool called_;
  base::Closure quit_closure_;
};

template <typename T, typename S  = T>
class CookieStoreCallback : public CookieStoreCallbackBase {
 public:
  CookieStoreCallback()
      : sequence_(0),
        weak_ptr_factory_(this) {}

  ~CookieStoreCallback() override = default;

  base::Callback<void(T)> MakeCallback() {
    result_ = S();
    Reset();
    return base::Bind(&CookieStoreCallback<T, S>::OnCallback,
                      GetWeakPtr(),
                      ++sequence_);
  }

  const S& result() const { return result_; }

 private:
  base::WeakPtr<CookieStoreCallback<T, S>> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  void OnCallback(int sequence, T result) {
    if (sequence != sequence_) {
      return;
    }
    result_ = result;
    NotifyCalled();
  }

  int sequence_;
  S result_;
  base::WeakPtrFactory<CookieStoreCallback<T, S>> weak_ptr_factory_;
};

class CookieStoreClosure : public CookieStoreCallbackBase {
 public:
  CookieStoreClosure()
      : sequence_(0),
        weak_ptr_factory_(this) {}

  ~CookieStoreClosure() override = default;

  base::Closure MakeCallback() {
    Reset();
    return base::Bind(&CookieStoreClosure::OnCallback,
                      GetWeakPtr(),
                      ++sequence_);
  }

 private:
  base::WeakPtr<CookieStoreClosure> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  void OnCallback(int sequence) {
    if (sequence != sequence_) {
      return;
    }
    NotifyCalled();
  }

  int sequence_;
  base::WeakPtrFactory<CookieStoreClosure> weak_ptr_factory_;
};

class CookieStoreProxyTest : public testing::Test {
 public:
  CookieStoreProxyTest() = default;

 protected:
  net::CookieStore* store() const { return store_.get(); }

  using CookieChange =
      std::tuple<net::CanonicalCookie,
                 bool,
                 net::CookieMonsterDelegate::ChangeCause>;
  using CookieChangeVector = std::vector<CookieChange>;

  const CookieChangeVector& cookie_changes() const { return cookie_changes_; }

 private:

  class CookieMonsterDelegate : public net::CookieMonsterDelegate {
   public:
    CookieMonsterDelegate(CookieStoreProxyTest* test)
        : test_(test) {}
    ~CookieMonsterDelegate() override = default;

   private:
    // net::CookieMonsterDelegate
    void OnCookieChanged(const net::CanonicalCookie& cookie,
                         bool removed,
                         ChangeCause cause) override;

    CookieStoreProxyTest* test_;
  };

  // testing::Test implementation
  void SetUp() override;
  void TearDown() override;

  void CookieChanged(const net::CanonicalCookie& cookie,
                     bool removed,
                     net::CookieMonsterDelegate::ChangeCause cause);

  scoped_refptr<CookieMonsterDelegate> cookie_monster_delegate_;

  std::unique_ptr<base::MessageLoop> message_loop_;
  std::unique_ptr<base::Thread> cookie_thread_;

  std::unique_ptr<CookieStoreOwner> store_owner_;
  std::unique_ptr<CookieStoreProxy> store_;

  CookieChangeVector cookie_changes_;

  DISALLOW_COPY_AND_ASSIGN(CookieStoreProxyTest);
};

void CookieStoreProxyTest::SetUp() {
  cookie_monster_delegate_ = new CookieMonsterDelegate(this);

  message_loop_ = base::MakeUnique<base::MessageLoop>();

  cookie_thread_ = base::MakeUnique<base::Thread>("TestCookieThread");
  cookie_thread_->Start();

  store_owner_ = base::MakeUnique<CookieStoreOwner>();
  store_ =
      base::MakeUnique<CookieStoreProxy>(store_owner_->GetWeakPtr(),
                                         base::ThreadTaskRunnerHandle::Get(),
                                         cookie_thread_->task_runner());

  base::Lock lock;
  base::ConditionVariable cv(&lock);

  base::AutoLock al(lock);

  cookie_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&InitializeCookieStore,
                 base::Unretained(store_owner_.get()),
                 cookie_monster_delegate_,
                 base::Unretained(&lock),
                 base::Unretained(&cv)));

  cv.Wait();
}

void CookieStoreProxyTest::TearDown() {
  base::Lock lock;
  base::ConditionVariable cv(&lock);

  base::AutoLock al(lock);

  cookie_thread_->task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&TeardownCookieStore,
                 base::Unretained(store_owner_.get()),
                 base::Unretained(&lock),
                 base::Unretained(&cv)));

  cv.Wait();

  cookie_thread_->Stop();

  base::RunLoop run_loop;
  run_loop.RunUntilIdle();
}

void CookieStoreProxyTest::CookieChanged(
    const net::CanonicalCookie& cookie,
    bool removed,
    net::CookieMonsterDelegate::ChangeCause cause) {
  cookie_changes_.push_back(std::make_tuple(cookie, removed, cause));
}

void CookieStoreProxyTest::CookieMonsterDelegate::OnCookieChanged(
    const net::CanonicalCookie& cookie,
    bool removed,
    ChangeCause cause) {
  test_->CookieChanged(cookie, removed, cause);
}

TEST_F(CookieStoreProxyTest, SetCookieWithDetailsAsync) {
  CookieStoreCallback<bool> callback;
  base::Time creation_time = base::Time::Now();
  base::Time expiration_time = creation_time + base::TimeDelta::FromDays(1);
  store()->SetCookieWithDetailsAsync(
      GURL("https://www.google.com/"),
      "foo", "bar", std::string(), std::string(),
      creation_time, expiration_time, base::Time(),
      false, false, net::CookieSameSite::NO_RESTRICTION,
      false, net::COOKIE_PRIORITY_DEFAULT, callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
  EXPECT_TRUE(callback.result());

  const CookieChangeVector& changes = cookie_changes();

  EXPECT_EQ(1U, changes.size());
  EXPECT_EQ(GURL("https://www.google.com/"), std::get<0>(changes[0]).Source());
  EXPECT_EQ("foo", std::get<0>(changes[0]).Name());
  EXPECT_EQ("bar", std::get<0>(changes[0]).Value());
  EXPECT_EQ("www.google.com", std::get<0>(changes[0]).Domain());
  EXPECT_EQ("/", std::get<0>(changes[0]).Path());
  EXPECT_EQ(creation_time, std::get<0>(changes[0]).CreationDate());
  EXPECT_EQ(creation_time, std::get<0>(changes[0]).LastAccessDate());
  EXPECT_FALSE(std::get<0>(changes[0]).IsSecure());
  EXPECT_FALSE(std::get<0>(changes[0]).IsHttpOnly());
  EXPECT_EQ(net::CookieSameSite::NO_RESTRICTION,
            std::get<0>(changes[0]).SameSite());
  EXPECT_EQ(net::COOKIE_PRIORITY_DEFAULT, std::get<0>(changes[0]).Priority());
  EXPECT_FALSE(std::get<1>(changes[0]));
  EXPECT_EQ(net::CookieMonsterDelegate::CHANGE_COOKIE_EXPLICIT,
            std::get<2>(changes[0]));
}

TEST_F(CookieStoreProxyTest, SetCookieWithDetailsAsyncNoCallback) {
  base::Time creation_time = base::Time::Now();
  base::Time expiration_time = creation_time + base::TimeDelta::FromDays(1);
  store()->SetCookieWithDetailsAsync(
      GURL("https://www.google.com/"),
      "foo", "bar", std::string(), std::string(),
      creation_time, expiration_time, base::Time(),
      false, false, net::CookieSameSite::NO_RESTRICTION,
      false, net::COOKIE_PRIORITY_DEFAULT,
      net::CookieStore::SetCookiesCallback());

  CookieStoreCallback<const net::CookieList&, net::CookieList> callback;
  store()->GetCookieListWithOptionsAsync(
      GURL("https://www.google.com/"),
      net::CookieOptions(),
      callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
  EXPECT_EQ(1U, callback.result().size());
  EXPECT_EQ(GURL("https://www.google.com/"), callback.result()[0].Source());
  EXPECT_EQ("foo", callback.result()[0].Name());
  EXPECT_EQ("bar", callback.result()[0].Value());
  EXPECT_EQ("www.google.com", callback.result()[0].Domain());
  EXPECT_EQ("/", callback.result()[0].Path());
  EXPECT_EQ(creation_time, callback.result()[0].CreationDate());
  EXPECT_EQ(creation_time, callback.result()[0].LastAccessDate());
  EXPECT_FALSE(callback.result()[0].IsSecure());
  EXPECT_FALSE(callback.result()[0].IsHttpOnly());
  EXPECT_EQ(net::CookieSameSite::NO_RESTRICTION,
            callback.result()[0].SameSite());
  EXPECT_EQ(net::COOKIE_PRIORITY_DEFAULT, callback.result()[0].Priority());
}

TEST_F(CookieStoreProxyTest, GetCookiesWithOptionsAsync) {
  CookieStoreCallback<bool> set_callback;
  base::Time creation_time = base::Time::Now();
  base::Time expiration_time = creation_time + base::TimeDelta::FromDays(1);
  store()->SetCookieWithDetailsAsync(
      GURL("https://www.google.com/"),
      "foo", "bar", std::string(), std::string(),
      creation_time, expiration_time, base::Time(),
      false, true, net::CookieSameSite::NO_RESTRICTION,
      false, net::COOKIE_PRIORITY_DEFAULT, set_callback.MakeCallback());
  EXPECT_TRUE(set_callback.WaitForCallback());

  net::CookieOptions options;
  options.set_do_not_update_access_time();
  options.set_include_httponly();

  CookieStoreCallback<const std::string&, std::string> callback;
  store()->GetCookiesWithOptionsAsync(
      GURL("https://www.example.com/"),
      options,
      callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
  EXPECT_EQ("", callback.result());

  store()->GetCookiesWithOptionsAsync(
      GURL("https://www.google.com/"),
      options,
      callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
  EXPECT_EQ("foo=bar", callback.result());

  options.set_exclude_httponly();

  store()->GetCookiesWithOptionsAsync(
      GURL("https://www.google.com/"),
      options,
      callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
  EXPECT_EQ("", callback.result());
}

TEST_F(CookieStoreProxyTest, GetCookieListWithOptionsAsync) {
  CookieStoreCallback<bool> set_callback;
  base::Time creation_time = base::Time::Now();
  base::Time expiration_time = creation_time + base::TimeDelta::FromDays(1);
  store()->SetCookieWithDetailsAsync(
      GURL("https://www.google.com/"),
      "foo", "bar", std::string(), std::string(),
      creation_time, expiration_time, base::Time(),
      false, true, net::CookieSameSite::NO_RESTRICTION,
      false, net::COOKIE_PRIORITY_DEFAULT, set_callback.MakeCallback());
  EXPECT_TRUE(set_callback.WaitForCallback());

  net::CookieOptions options;
  options.set_do_not_update_access_time();
  options.set_include_httponly();

  CookieStoreCallback<const net::CookieList&, net::CookieList> callback;
  store()->GetCookieListWithOptionsAsync(
      GURL("https://www.example.com/"),
      options,
      callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
  EXPECT_EQ(0U, callback.result().size());

  store()->GetCookieListWithOptionsAsync(
      GURL("https://www.google.com/"),
      options,
      callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
  EXPECT_EQ(1U, callback.result().size());
  EXPECT_EQ(GURL("https://www.google.com/"), callback.result()[0].Source());
  EXPECT_EQ("foo", callback.result()[0].Name());
  EXPECT_EQ("bar", callback.result()[0].Value());
  EXPECT_EQ("www.google.com", callback.result()[0].Domain());
  EXPECT_EQ("/", callback.result()[0].Path());
  EXPECT_EQ(creation_time, callback.result()[0].CreationDate());
  EXPECT_EQ(creation_time, callback.result()[0].LastAccessDate());
  EXPECT_FALSE(callback.result()[0].IsSecure());
  EXPECT_TRUE(callback.result()[0].IsHttpOnly());
  EXPECT_EQ(net::CookieSameSite::NO_RESTRICTION,
            callback.result()[0].SameSite());
  EXPECT_EQ(net::COOKIE_PRIORITY_DEFAULT, callback.result()[0].Priority());

  options.set_exclude_httponly();

  store()->GetCookieListWithOptionsAsync(
      GURL("https://www.google.com/"),
      options,
      callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
  EXPECT_EQ(0U, callback.result().size());
}

TEST_F(CookieStoreProxyTest, GetAllCookiesAsync) {
  CookieStoreCallback<bool> set_callback;
  base::Time creation_time = base::Time::Now();
  base::Time expiration_time = creation_time + base::TimeDelta::FromDays(1);
  store()->SetCookieWithDetailsAsync(
      GURL("https://www.google.com/"),
      "foo", "bar", std::string(), std::string(),
      creation_time, expiration_time, base::Time(),
      false, false, net::CookieSameSite::NO_RESTRICTION,
      false, net::COOKIE_PRIORITY_DEFAULT, set_callback.MakeCallback());
  EXPECT_TRUE(set_callback.WaitForCallback());

  CookieStoreCallback<const net::CookieList&, net::CookieList> callback;
  store()->GetAllCookiesAsync(callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
  EXPECT_EQ(1U, callback.result().size());
  EXPECT_EQ(GURL("https://www.google.com/"), callback.result()[0].Source());
  EXPECT_EQ("foo", callback.result()[0].Name());
  EXPECT_EQ("bar", callback.result()[0].Value());
  EXPECT_EQ("www.google.com", callback.result()[0].Domain());
  EXPECT_EQ("/", callback.result()[0].Path());
  EXPECT_EQ(creation_time, callback.result()[0].CreationDate());
  EXPECT_EQ(creation_time, callback.result()[0].LastAccessDate());
  EXPECT_FALSE(callback.result()[0].IsSecure());
  EXPECT_FALSE(callback.result()[0].IsHttpOnly());
  EXPECT_EQ(net::CookieSameSite::NO_RESTRICTION,
            callback.result()[0].SameSite());
  EXPECT_EQ(net::COOKIE_PRIORITY_DEFAULT, callback.result()[0].Priority());
}

TEST_F(CookieStoreProxyTest, FlushStore) {
  // XXX: We don't actually test that we call FlushStore on the underlying
  //  CookieStore
  CookieStoreClosure callback;
  store()->FlushStore(callback.MakeCallback());

  EXPECT_TRUE(callback.WaitForCallback());
}

TEST_F(CookieStoreProxyTest, FlushStoreNoCallback) {
  store()->FlushStore(base::Closure());
}

} // namespace oxide
