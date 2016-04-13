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

#ifndef _OXIDE_SHARED_BROWSER_COOKIE_STORE_UI_PROXY_H_
#define _OXIDE_SHARED_BROWSER_COOKIE_STORE_UI_PROXY_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "net/cookies/cookie_store.h"

#include "shared/common/oxide_shared_export.h"

namespace oxide {

class OXIDE_SHARED_EXPORT CookieStoreUIProxy : public net::CookieStore {
 public:
  CookieStoreUIProxy(net::CookieStore* store);
  ~CookieStoreUIProxy() override;

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
  scoped_ptr<CookieChangedSubscription> AddCallbackForCookie(
      const GURL& url,
      const std::string& name,
      const CookieChangedCallback& callback) override;
  bool IsEphemeral() override;

 private:
  class Core;
  scoped_refptr<Core> core_;

  DISALLOW_COPY_AND_ASSIGN(CookieStoreUIProxy);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_COOKIE_STORE_UI_PROXY_H_
