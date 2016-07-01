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

#ifndef _OXIDE_SHARED_BROWSER_COOKIE_STORE_PROXY_H_
#define _OXIDE_SHARED_BROWSER_COOKIE_STORE_PROXY_H_

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "net/cookies/cookie_store.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

class OXIDE_SHARED_EXPORT CookieStoreOwner {
 public:
  CookieStoreOwner();
  ~CookieStoreOwner();

  base::WeakPtr<CookieStoreOwner> GetWeakPtr();

  net::CookieStore* store() const { return store_.get(); }
  void set_store(std::unique_ptr<net::CookieStore> store) {
    store_ = std::move(store);
  }

 private:
  std::unique_ptr<net::CookieStore> store_;

  base::WeakPtrFactory<CookieStoreOwner> weak_ptr_factory_;
};

class OXIDE_SHARED_EXPORT CookieStoreProxy : public net::CookieStore {
 public:
  CookieStoreProxy(
      base::WeakPtr<CookieStoreOwner> store_owner,
      scoped_refptr<base::SingleThreadTaskRunner> client_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> cookie_task_runner);
  ~CookieStoreProxy() override;

  void SetCookieWithOptionsAsync(const GURL& url,
                                 const std::string& cookie_line,
                                 const net::CookieOptions& options,
                                 const SetCookiesCallback& callback) override;
  void SetCookieWithDetailsAsync(const GURL& url,
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
                                 bool enforce_strict_secure,
                                 net::CookiePriority priority,
                                 const SetCookiesCallback& callback) override;
  void GetCookiesWithOptionsAsync(const GURL& url,
                                  const net::CookieOptions& options,
                                  const GetCookiesCallback& callback) override;
  void GetCookieListWithOptionsAsync(
      const GURL& url,
      const net::CookieOptions& options,
      const GetCookieListCallback& callback) override;
  void GetAllCookiesAsync(const GetCookieListCallback& callback) override;
  void DeleteCookieAsync(const GURL& url,
                         const std::string& cookie_name,
                         const base::Closure& callback) override;
  void DeleteCanonicalCookieAsync(const net::CanonicalCookie& cookie,
                                  const DeleteCallback& callback) override;
  void DeleteAllCreatedBetweenAsync(const base::Time& delete_begin,
                                    const base::Time& delete_end,
                                    const DeleteCallback& callback) override;
  void DeleteAllCreatedBetweenWithPredicateAsync(
      const base::Time& delete_begin,
      const base::Time& delete_end,
      const CookiePredicate& predicate,
      const DeleteCallback& callback) override;
  void DeleteSessionCookiesAsync(const DeleteCallback& callback) override;
  void FlushStore(const base::Closure& callback) override;
  void SetForceKeepSessionState() override;
  std::unique_ptr<CookieChangedSubscription> AddCallbackForCookie(
      const GURL& url,
      const std::string& name,
      const CookieChangedCallback& callback) override;
  bool IsEphemeral() override;

 private:
  bool IsOnClientThread() const;

  void PostTaskToCookieThread(const base::Closure& task);

  base::Closure WrapClosure(const base::Closure& callback);
  void RunClosure(base::Closure callback);

  template <typename T>
  void RunCallback(base::Callback<void(T)> callback, T result) {
    DCHECK(IsOnClientThread());
    callback.Run(result);
  }

  template <typename T>
  static void CallbackThunk(
      base::WeakPtr<CookieStoreProxy> cookie_store,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner,
      base::Callback<void(T)> callback,
      T result) {
    task_runner->PostTask(FROM_HERE,
                          base::Bind(&CookieStoreProxy::RunCallback<T>,
                                     cookie_store,
                                     callback,
                                     result));
  }

  template <typename T>
  base::Callback<void(T)> WrapCallback(
      const base::Callback<void(T)>& callback) {
    if (callback.is_null()) {
      return base::Callback<void(T)>();
    }

    return base::Bind(&CookieStoreProxy::CallbackThunk<T>,
                      weak_ptr_factory_.GetWeakPtr(),
                      client_task_runner_,
                      callback);
  }

  class Core;
  scoped_refptr<Core> core_;

  scoped_refptr<base::SingleThreadTaskRunner> client_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> cookie_task_runner_;

  base::WeakPtrFactory<CookieStoreProxy> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(CookieStoreProxy);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COOKIE_STORE_PROXY_H_
