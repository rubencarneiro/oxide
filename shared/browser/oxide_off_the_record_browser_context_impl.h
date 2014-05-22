// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_OFF_THE_RECORD_BROWSER_CONTEXT_IMPL_H_
#define _OXIDE_SHARED_BROWSER_OFF_THE_RECORD_BROWSER_CONTEXT_IMPL_H_

#include "oxide_browser_context.h"

#include "base/basictypes.h"
#include "base/compiler_specific.h"

namespace oxide {

class BrowserContextImpl;

class OffTheRecordBrowserContextIODataImpl FINAL :
    public BrowserContextIOData {
 public:
  OffTheRecordBrowserContextIODataImpl(BrowserContextIOData* original_io_data);

  net::StaticCookiePolicy::Type GetCookiePolicy() const FINAL;
  content::CookieStoreConfig::SessionCookieMode GetSessionCookieMode() const FINAL;

  bool IsPopupBlockerEnabled() const FINAL;

  base::FilePath GetPath() const FINAL;
  base::FilePath GetCachePath() const FINAL;

  std::string GetAcceptLangs() const FINAL;

  std::string GetUserAgent() const FINAL;

  bool IsOffTheRecord() const FINAL;

 private:
  BrowserContextIOData* original_io_data_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(OffTheRecordBrowserContextIODataImpl);
};

class OffTheRecordBrowserContextImpl FINAL : public BrowserContext {
 public:
  BrowserContext* GetOffTheRecordContext() FINAL;
  BrowserContext* GetOriginalContext() FINAL;

  void SetAcceptLangs(const std::string& langs) FINAL;

  std::string GetProduct() const FINAL;
  void SetProduct(const std::string& product) FINAL;

  void SetUserAgent(const std::string& user_agent) FINAL;
  void SetCookiePolicy(net::StaticCookiePolicy::Type policy) FINAL;
  void SetIsPopupBlockerEnabled(bool enabled) FINAL;

  UserScriptMaster& UserScriptManager() FINAL;

 private:
  friend class BrowserContext;
  friend class BrowserContextImpl;

  OffTheRecordBrowserContextImpl(BrowserContextImpl* original_context);

  BrowserContextImpl* original_context_;

  DISALLOW_COPY_AND_ASSIGN(OffTheRecordBrowserContextImpl); 
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_OFF_THE_RECORD_BROWSER_CONTEXT_IMPL_H_ 
