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

#ifndef _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_IMPL_H_
#define _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_IMPL_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/synchronization/lock.h"

#include "oxide_browser_context.h"
#include "oxide_user_script_master.h"

namespace oxide {

class OffTheRecordBrowserContextImpl;

class BrowserContextIODataImpl FINAL : public BrowserContextIOData {
 public:
  BrowserContextIODataImpl(const base::FilePath& path,
                           const base::FilePath& cache_path);

  base::FilePath GetPath() const FINAL;
  base::FilePath GetCachePath() const FINAL;

  std::string GetAcceptLangs() const FINAL;
  void SetAcceptLangs(const std::string& langs);

  std::string GetUserAgent() const FINAL;
  void SetUserAgent(const std::string& user_agent);

  bool IsOffTheRecord() const FINAL;

 private:
  mutable base::Lock lock_;

  base::FilePath path_;
  base::FilePath cache_path_;

  std::string user_agent_;

  std::string accept_langs_;

  DISALLOW_COPY_AND_ASSIGN(BrowserContextIODataImpl);
};

class BrowserContextImpl FINAL : public BrowserContext {
 public:
  BrowserContext* GetOffTheRecordContext() FINAL;
  BrowserContext* GetOriginalContext() FINAL;

  void SetAcceptLangs(const std::string& langs) FINAL;

  std::string GetProduct() const FINAL;
  void SetProduct(const std::string& product) FINAL;

  void SetUserAgent(const std::string& user_agent) FINAL;

  UserScriptMaster& UserScriptManager() FINAL;

 private:
  friend class BrowserContext;

  BrowserContextImpl(const base::FilePath& path,
                     const base::FilePath& cache_path);

  scoped_ptr<OffTheRecordBrowserContextImpl> otr_context_;
  std::string product_;
  bool default_user_agent_string_;
  UserScriptMaster user_script_manager_;

  DISALLOW_COPY_AND_ASSIGN(BrowserContextImpl);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_BROWSER_CONTEXT_IMPL_H_ 
