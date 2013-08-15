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

  net::SSLConfigService* ssl_config_service() const FINAL;
  net::HttpUserAgentSettings* http_user_agent_settings() const FINAL;

  base::FilePath GetPath() const FINAL;
  bool SetPath(const base::FilePath& path) FINAL;
  base::FilePath GetCachePath() const FINAL;
  bool SetCachePath(const base::FilePath& cache_path) FINAL;

  std::string GetAcceptLangs() const;
  void SetAcceptLangs(const std::string& langs);

  std::string GetProduct() const;
  void SetProduct(const std::string& product);

  std::string GetUserAgent() const;
  void SetUserAgent(const std::string& user_agent);

  bool IsOffTheRecord() const;

 private:
  BrowserContextIOData* original_io_data_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(OffTheRecordBrowserContextIODataImpl);
};

class OffTheRecordBrowserContextImpl FINAL : public BrowserContext {
 public:
  BrowserContext* GetOffTheRecordContext() FINAL;
  BrowserContext* GetOriginalContext() FINAL;

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
