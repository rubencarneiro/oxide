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

#include "oxide_cookie_store_ui_proxy.h"

#include <string>

#include "base/bind.h"
#include "base/synchronization/lock.h"
#include "content/public/browser/browser_thread.h"

namespace oxide {

class CookieStoreUIProxy::Core : public base::RefCountedThreadSafe<Core> {
 public:
  Core(net::CookieStore* store);

  void DetachFromUI();

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
      bool enforce_strict_secure,
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

 private:
  friend class base::RefCountedThreadSafe<Core>;
  ~Core();

  // Access the underlying cookie store, asserting that the caller is on the
  // IO thread
  net::CookieStore* GetStore() const;

  void SetCookieWithDetailsAsync_IO(
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
      bool enforce_strict_secure,
      net::CookiePriority priority,
      const net::CookieStore::SetCookiesCallback& callback);
  void SetCookiesResponse_IO(
      const net::CookieStore::SetCookiesCallback& callback,
      bool success);
  void SendSetCookiesResponse_UI(
      const net::CookieStore::SetCookiesCallback& callback,
      bool success);

  void GetCookiesWithOptionsAsync_IO(
      const GURL& url,
      const net::CookieOptions& options,
      const net::CookieStore::GetCookiesCallback& callback);
  void GetCookiesResponse_IO(
      const net::CookieStore::GetCookiesCallback& callback,
      const std::string& result);
  void SendGetCookiesResponse_UI(
      const net::CookieStore::GetCookiesCallback& callback,
      const std::string& result);

  void GetCookieListWithOptionsAsync_IO(
      const GURL& url,
      const net::CookieOptions& options,
      const net::CookieStore::GetCookieListCallback& callback);
  void GetAllCookiesAsync_IO(
      const net::CookieStore::GetCookieListCallback& callback);
  void GetCookieListResponse_IO(
      const net::CookieStore::GetCookieListCallback& callback,
      const net::CookieList& result);
  void SendGetCookieListResponse_UI(
      const net::CookieStore::GetCookieListCallback& callback,
      const net::CookieList& result);

  void DeleteAllCreatedBetweenAsync_IO(
      const base::Time& delete_begin,
      const base::Time& delete_end,
      const net::CookieStore::DeleteCallback& callback);
  void DeleteResponse_IO(
      const net::CookieStore::DeleteCallback& callback,
      int num_deleted);
  void SendDeleteResponse_UI(
      const net::CookieStore::DeleteCallback& callback,
      int num_deleted);

  void FlushStore_IO(const base::Closure& callback);
  void GenericResponse_IO(const base::Closure& callback);
  void SendGenericResponse_UI(const base::Closure& callback);

  // Lock for |store_unsafe_access_|. This doesn't provide any thread-safety
  // guarantees for net::CookieStore - it's only here for guarding the member
  // variable, which can be cleared from the UI thread
  mutable base::Lock store_lock_;

  net::CookieStore* store_unsafe_access_;

  DISALLOW_COPY_AND_ASSIGN(Core);
};

CookieStoreUIProxy::Core::~Core() {
  DCHECK(!store_unsafe_access_);
}

net::CookieStore* CookieStoreUIProxy::Core::GetStore() const {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  base::AutoLock lock(store_lock_);
  return store_unsafe_access_;
}

void CookieStoreUIProxy::Core::SetCookieWithDetailsAsync_IO(
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
    bool enforce_strict_secure,
    net::CookiePriority priority,
    const net::CookieStore::SetCookiesCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    SetCookiesResponse_IO(callback, false);
    return;
  }

  store->SetCookieWithDetailsAsync(
      url, name, value, domain, path, creation_time, expiration_time,
      last_access_time, secure, http_only, same_site, enforce_strict_secure,
      priority,
      base::Bind(&Core::SetCookiesResponse_IO, this, callback));
}

void CookieStoreUIProxy::Core::SetCookiesResponse_IO(
    const net::CookieStore::SetCookiesCallback& callback,
    bool success) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&Core::SendSetCookiesResponse_UI, this, callback, success));
}

void CookieStoreUIProxy::Core::SendSetCookiesResponse_UI(
    const net::CookieStore::SetCookiesCallback& callback,
    bool success) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  callback.Run(success);
}

void CookieStoreUIProxy::Core::GetCookiesWithOptionsAsync_IO(
    const GURL& url,
    const net::CookieOptions& options,
    const net::CookieStore::GetCookiesCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    GetCookiesResponse_IO(callback, std::string());
    return;
  }

  store->GetCookiesWithOptionsAsync(
      url, options,
      base::Bind(&Core::GetCookiesResponse_IO, this, callback));
}

void CookieStoreUIProxy::Core::GetCookiesResponse_IO(
    const net::CookieStore::GetCookiesCallback& callback,
    const std::string& result) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&Core::SendGetCookiesResponse_UI, this, callback, result));
}

void CookieStoreUIProxy::Core::SendGetCookiesResponse_UI(
    const net::CookieStore::GetCookiesCallback& callback,
    const std::string& result) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  callback.Run(result);
}

void CookieStoreUIProxy::Core::GetCookieListWithOptionsAsync_IO(
    const GURL& url,
    const net::CookieOptions& options,
    const net::CookieStore::GetCookieListCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    GetCookieListResponse_IO(callback, net::CookieList());
    return;
  }

  store->GetCookieListWithOptionsAsync(
      url, options,
      base::Bind(&Core::GetCookieListResponse_IO, this, callback));
}

void CookieStoreUIProxy::Core::GetAllCookiesAsync_IO(
    const net::CookieStore::GetCookieListCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    GetCookieListResponse_IO(callback, net::CookieList());
    return;
  }

  store->GetAllCookiesAsync(
      base::Bind(&Core::GetCookieListResponse_IO, this, callback));
}

void CookieStoreUIProxy::Core::GetCookieListResponse_IO(
    const net::CookieStore::GetCookieListCallback& callback,
    const net::CookieList& result) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&Core::SendGetCookieListResponse_UI, this, callback, result));
}

void CookieStoreUIProxy::Core::SendGetCookieListResponse_UI(
    const net::CookieStore::GetCookieListCallback& callback,
    const net::CookieList& result) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  callback.Run(result);
}

void CookieStoreUIProxy::Core::DeleteAllCreatedBetweenAsync_IO(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const net::CookieStore::DeleteCallback& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    DeleteResponse_IO(callback, 0);
    return;
  }

  store->DeleteAllCreatedBetweenAsync(
      delete_begin, delete_end,
      base::Bind(&Core::DeleteResponse_IO, this, callback));
}

void CookieStoreUIProxy::Core::DeleteResponse_IO(
    const net::CookieStore::DeleteCallback& callback,
    int num_deleted) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&Core::SendDeleteResponse_UI, this, callback, num_deleted));
}

void CookieStoreUIProxy::Core::SendDeleteResponse_UI(
    const net::CookieStore::DeleteCallback& callback,
    int num_deleted) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  callback.Run(num_deleted);
}

void CookieStoreUIProxy::Core::FlushStore_IO(const base::Closure& callback) {
  net::CookieStore* store = GetStore();
  if (!store) {
    return;
  }

  store->FlushStore(base::Bind(&Core::GenericResponse_IO, this, callback));
}

void CookieStoreUIProxy::Core::GenericResponse_IO(
    const base::Closure& callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::UI,
      FROM_HERE,
      base::Bind(&Core::SendGenericResponse_UI, this, callback));
}

void CookieStoreUIProxy::Core::SendGenericResponse_UI(
    const base::Closure& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  callback.Run();
}

CookieStoreUIProxy::Core::Core(net::CookieStore* store)
    : store_unsafe_access_(store) {}

void CookieStoreUIProxy::Core::DetachFromUI() {
  base::AutoLock lock(store_lock_);
  store_unsafe_access_ = nullptr;
}

void CookieStoreUIProxy::Core::SetCookieWithDetailsAsync(
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
    bool enforce_strict_secure,
    net::CookiePriority priority,
    const net::CookieStore::SetCookiesCallback& callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&Core::SetCookieWithDetailsAsync_IO,
                 this,
                 url, name, value, domain, path, creation_time,
                 expiration_time, last_access_time, secure, http_only,
                 same_site, enforce_strict_secure, priority, callback));
}

void CookieStoreUIProxy::Core::GetCookiesWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const net::CookieStore::GetCookiesCallback& callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&Core::GetCookiesWithOptionsAsync_IO,
                 this, url, options, callback));
}

void CookieStoreUIProxy::Core::GetCookieListWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const net::CookieStore::GetCookieListCallback& callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&Core::GetCookieListWithOptionsAsync_IO,
                 this,
                 url, options, callback));
}

void CookieStoreUIProxy::Core::GetAllCookiesAsync(
    const net::CookieStore::GetCookieListCallback& callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&Core::GetAllCookiesAsync_IO, this, callback));
}

void CookieStoreUIProxy::Core::DeleteAllCreatedBetweenAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const DeleteCallback& callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&Core::DeleteAllCreatedBetweenAsync_IO,
                 this,
                 delete_begin, delete_end, callback));
}

void CookieStoreUIProxy::Core::FlushStore(const base::Closure& callback) {
  content::BrowserThread::PostTask(
      content::BrowserThread::IO,
      FROM_HERE,
      base::Bind(&Core::FlushStore_IO, this, callback));
}

CookieStoreUIProxy::CookieStoreUIProxy(net::CookieStore* store)
    : core_(new Core(store)) {}

CookieStoreUIProxy::~CookieStoreUIProxy() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  core_->DetachFromUI();
}

void CookieStoreUIProxy::SetCookieWithOptionsAsync(
    const GURL& url,
    const std::string& cookie_line,
    const net::CookieOptions& options,
    const SetCookiesCallback& callback) {
  NOTIMPLEMENTED();
  callback.Run(false);
}

void CookieStoreUIProxy::SetCookieWithDetailsAsync(
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
    bool enforce_strict_secure,
    net::CookiePriority priority,
    const SetCookiesCallback& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  core_->SetCookieWithDetailsAsync(url, name, value, domain, path,
                                   creation_time, expiration_time,
                                   last_access_time, secure, http_only,
                                   same_site, enforce_strict_secure, priority,
                                   callback);
}

void CookieStoreUIProxy::GetCookiesWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const GetCookiesCallback& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  core_->GetCookiesWithOptionsAsync(url, options, callback);
}

void CookieStoreUIProxy::GetCookieListWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const GetCookieListCallback& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  core_->GetCookieListWithOptionsAsync(url, options, callback);
}

void CookieStoreUIProxy::GetAllCookiesAsync(
    const GetCookieListCallback& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  core_->GetAllCookiesAsync(callback);
}

void CookieStoreUIProxy::DeleteCookieAsync(const GURL& url,
                                           const std::string& cookie_name,
                                           const base::Closure& callback) {
  NOTIMPLEMENTED();
}

void CookieStoreUIProxy::DeleteCanonicalCookieAsync(
    const net::CanonicalCookie& cookie,
    const DeleteCallback& callback) {
  NOTIMPLEMENTED();
  callback.Run(0);
}

void CookieStoreUIProxy::DeleteAllCreatedBetweenAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const DeleteCallback& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  core_->DeleteAllCreatedBetweenAsync(delete_begin, delete_end, callback);
}

void CookieStoreUIProxy::DeleteAllCreatedBetweenWithPredicateAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const CookiePredicate& predicate,
    const DeleteCallback& callback) {
  NOTIMPLEMENTED();
  callback.Run(0);
}

void CookieStoreUIProxy::DeleteSessionCookiesAsync(
    const DeleteCallback& callback) {
  NOTIMPLEMENTED();
  callback.Run(0);
}

void CookieStoreUIProxy::FlushStore(const base::Closure& callback) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  core_->FlushStore(callback);
}

void CookieStoreUIProxy::SetForceKeepSessionState() {
  NOTIMPLEMENTED();
}

scoped_ptr<net::CookieStore::CookieChangedSubscription>
CookieStoreUIProxy::AddCallbackForCookie(
    const GURL& url,
    const std::string& name,
    const CookieChangedCallback& callback) {
  NOTIMPLEMENTED();
  return nullptr;
}

bool CookieStoreUIProxy::IsEphemeral() {
  NOTIMPLEMENTED();
  return false;
}

} // namespace oxide
