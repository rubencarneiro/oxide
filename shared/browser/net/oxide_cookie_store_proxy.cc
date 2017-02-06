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

#include "oxide_cookie_store_proxy.h"

#include <string>

#include "base/synchronization/lock.h"
#include "content/public/browser/browser_thread.h"

namespace oxide {

CookieStoreOwner::CookieStoreOwner()
    : weak_ptr_factory_(this) {}

CookieStoreOwner::~CookieStoreOwner() {}

base::WeakPtr<CookieStoreOwner> CookieStoreOwner::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

class CookieStoreProxy::Core : public base::RefCountedThreadSafe<Core> {
 public:
  Core(base::WeakPtr<CookieStoreOwner> store_owneri,
       scoped_refptr<base::SingleThreadTaskRunner> cookie_task_runner);

  void SetCookieWithDetailsAsync(
      const GURL& url,
      const std::string& name,
      const std::string& value,
      const std::string& domain,
      const std::string& path,
      base::Time creation_time,
      base::Time expiration_time,
      base::Time last_access_time,
      bool secure,
      bool http_only,
      net::CookieSameSite same_site,
      net::CookiePriority priority,
      const net::CookieStore::SetCookiesCallback& callback);
  void GetCookiesWithOptionsAsync(
      const GURL& url,
      const net::CookieOptions& options,
      const net::CookieStore::GetCookiesCallback& callback);
  void GetCookieListWithOptionsAsync(
      const GURL& url,
      const net::CookieOptions& options,
      const net::CookieStore::GetCookieListCallback& callback);
  void GetAllCookiesAsync(
      const net::CookieStore::GetCookieListCallback& callback);
  void DeleteAllCreatedBetweenAsync(
      const base::Time& delete_begin,
      const base::Time& delete_end,
      const net::CookieStore::DeleteCallback& callback);
  void FlushStore(const base::Closure& callback);

  void DispatchTask(base::Closure task);

 private:
  friend class base::RefCountedThreadSafe<Core>;
  ~Core();

  net::CookieStore* GetStore() const;

  scoped_refptr<base::SingleThreadTaskRunner> cookie_task_runner_;
  base::WeakPtr<CookieStoreOwner> store_owner_;

  DISALLOW_COPY_AND_ASSIGN(Core);
};

CookieStoreProxy::Core::~Core() {}

net::CookieStore* CookieStoreProxy::Core::GetStore() const {
  DCHECK(cookie_task_runner_->RunsTasksOnCurrentThread());
  if (!store_owner_) {
    return nullptr;
  }

  return store_owner_->store();
}

CookieStoreProxy::Core::Core(
    base::WeakPtr<CookieStoreOwner> store_owner,
    scoped_refptr<base::SingleThreadTaskRunner> cookie_task_runner)
    : cookie_task_runner_(cookie_task_runner),
      store_owner_(store_owner) {}

void CookieStoreProxy::Core::SetCookieWithDetailsAsync(
    const GURL& url,
    const std::string& name,
    const std::string& value,
    const std::string& domain,
    const std::string& path,
    base::Time creation_time,
    base::Time expiration_time,
    base::Time last_access_time,
    bool secure,
    bool http_only,
    net::CookieSameSite same_site,
    net::CookiePriority priority,
    const net::CookieStore::SetCookiesCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    if (!callback.is_null()) {
      callback.Run(false);
    }
    return;
  }

  store->SetCookieWithDetailsAsync(
      url, name, value, domain, path, creation_time, expiration_time,
      last_access_time, secure, http_only, same_site, priority, callback);
}

void CookieStoreProxy::Core::GetCookiesWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const net::CookieStore::GetCookiesCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    callback.Run(std::string());
    return;
  }

  store->GetCookiesWithOptionsAsync(url, options, callback);
}

void CookieStoreProxy::Core::GetCookieListWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const net::CookieStore::GetCookieListCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    callback.Run(net::CookieList());
    return;
  }

  store->GetCookieListWithOptionsAsync(url, options, callback);
}

void CookieStoreProxy::Core::GetAllCookiesAsync(
    const net::CookieStore::GetCookieListCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    callback.Run(net::CookieList());
    return;
  }

  store->GetAllCookiesAsync(callback);
}

void CookieStoreProxy::Core::DeleteAllCreatedBetweenAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const DeleteCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    if (!callback.is_null()) {
      callback.Run(0);
    }
    return;
  }

  store->DeleteAllCreatedBetweenAsync(delete_begin, delete_end, callback);
}

void CookieStoreProxy::Core::FlushStore(const base::Closure& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    if (!callback.is_null()) {
      callback.Run();
    }
    return;
  }

  store->FlushStore(callback);
}

void CookieStoreProxy::Core::DispatchTask(base::Closure task) {
  // XXX(chrisccoulson): There might not be a cookie store yet. In this case,
  //  we should queue the task and dispatch it after the cookie store is
  //  initialized
  task.Run();
}

bool CookieStoreProxy::IsOnClientThread() const {
  return client_task_runner_->RunsTasksOnCurrentThread();
}

void CookieStoreProxy::PostTaskToCookieThread(const base::Closure& task) {
  cookie_task_runner_->PostTask(FROM_HERE,
                                base::Bind(&Core::DispatchTask, core_, task));
}

base::Closure CookieStoreProxy::WrapClosure(const base::Closure& callback) {
  if (callback.is_null()) {
    return base::Closure();
  }

  return base::Bind(base::IgnoreResult(&base::TaskRunner::PostTask),
                    client_task_runner_,
                    FROM_HERE,
                    base::Bind(&CookieStoreProxy::RunClosure,
                               weak_ptr_factory_.GetWeakPtr(),
                               callback));
}

void CookieStoreProxy::RunClosure(base::Closure callback) {
  DCHECK(IsOnClientThread());
  callback.Run();
}

CookieStoreProxy::CookieStoreProxy(
    base::WeakPtr<CookieStoreOwner> store_owner,
    scoped_refptr<base::SingleThreadTaskRunner> client_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> cookie_task_runner)
    : core_(new Core(store_owner, cookie_task_runner)),
      client_task_runner_(client_task_runner),
      cookie_task_runner_(cookie_task_runner),
      weak_ptr_factory_(this) {}

CookieStoreProxy::~CookieStoreProxy() {}

void CookieStoreProxy::SetCookieWithOptionsAsync(
    const GURL& url,
    const std::string& cookie_line,
    const net::CookieOptions& options,
    const SetCookiesCallback& callback) {
  NOTIMPLEMENTED();
  callback.Run(false);
}

void CookieStoreProxy::SetCookieWithDetailsAsync(
    const GURL& url,
    const std::string& name,
    const std::string& value,
    const std::string& domain,
    const std::string& path,
    base::Time creation_time,
    base::Time expiration_time,
    base::Time last_access_time,
    bool secure,
    bool http_only,
    net::CookieSameSite same_site,
    net::CookiePriority priority,
    const SetCookiesCallback& callback) {
  DCHECK(IsOnClientThread());
  PostTaskToCookieThread(
      base::Bind(&Core::SetCookieWithDetailsAsync,
                 core_,
                 url, name, value, domain, path, creation_time,
                 expiration_time, last_access_time, secure, http_only,
                 same_site, priority, WrapCallback(callback)));
}

void CookieStoreProxy::GetCookiesWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const GetCookiesCallback& callback) {
  DCHECK(IsOnClientThread());

  if (callback.is_null()) {
    return;
  }

  PostTaskToCookieThread(
      base::Bind(&Core::GetCookiesWithOptionsAsync,
                 core_,
                 url, options, WrapCallback(callback)));
}

void CookieStoreProxy::GetCookieListWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const GetCookieListCallback& callback) {
  DCHECK(IsOnClientThread());

  if (callback.is_null()) {
    return;
  }

  PostTaskToCookieThread(
      base::Bind(&Core::GetCookieListWithOptionsAsync,
                 core_,
                 url, options, WrapCallback(callback)));
}

void CookieStoreProxy::GetAllCookiesAsync(
    const GetCookieListCallback& callback) {
  DCHECK(IsOnClientThread());

  if (callback.is_null()) {
    return;
  }

  PostTaskToCookieThread(
      base::Bind(&Core::GetAllCookiesAsync, core_, WrapCallback(callback)));
}

void CookieStoreProxy::DeleteCookieAsync(const GURL& url,
                                           const std::string& cookie_name,
                                           const base::Closure& callback) {
  NOTIMPLEMENTED();
}

void CookieStoreProxy::DeleteCanonicalCookieAsync(
    const net::CanonicalCookie& cookie,
    const DeleteCallback& callback) {
  NOTIMPLEMENTED();
  callback.Run(0);
}

void CookieStoreProxy::DeleteAllCreatedBetweenAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const DeleteCallback& callback) {
  DCHECK(IsOnClientThread());
  PostTaskToCookieThread(
      base::Bind(&Core::DeleteAllCreatedBetweenAsync,
                 core_,
                 delete_begin,
                 delete_end,
                 WrapCallback(callback)));
}

void CookieStoreProxy::DeleteAllCreatedBetweenWithPredicateAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const CookiePredicate& predicate,
    const DeleteCallback& callback) {
  NOTIMPLEMENTED();
  callback.Run(0);
}

void CookieStoreProxy::DeleteSessionCookiesAsync(
    const DeleteCallback& callback) {
  NOTIMPLEMENTED();
  callback.Run(0);
}

void CookieStoreProxy::FlushStore(const base::Closure& callback) {
  DCHECK(IsOnClientThread());
  PostTaskToCookieThread(
      base::Bind(&Core::FlushStore, core_, WrapClosure(callback)));
}

void CookieStoreProxy::SetForceKeepSessionState() {
  NOTIMPLEMENTED();
}

std::unique_ptr<net::CookieStore::CookieChangedSubscription>
CookieStoreProxy::AddCallbackForCookie(
    const GURL& url,
    const std::string& name,
    const CookieChangedCallback& callback) {
  NOTIMPLEMENTED();
  return nullptr;
}

bool CookieStoreProxy::IsEphemeral() {
  NOTIMPLEMENTED();
  return false;
}

} // namespace oxide
