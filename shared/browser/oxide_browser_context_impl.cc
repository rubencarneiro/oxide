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

#include "oxide_browser_context_impl.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "content/public/browser/browser_thread.h"
#include "webkit/common/user_agent/user_agent.h"
#include "webkit/common/user_agent/user_agent_util.h"

#include "shared/common/chrome_version.h"
#include "shared/common/oxide_content_client.h"

#include "oxide_http_user_agent_settings.h"
#include "oxide_off_the_record_browser_context_impl.h"
#include "oxide_ssl_config_service.h"

namespace oxide {

BrowserContextIODataImpl::BrowserContextIODataImpl(
    const base::FilePath& path,
    const base::FilePath& cache_path) :
    ssl_config_service_(new SSLConfigService()),
    http_user_agent_settings_(new HttpUserAgentSettings(this)),
    path_(path),
    cache_path_(cache_path),
    product_(base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING)),
    user_agent_(webkit_glue::BuildUserAgentFromProduct(product_)),
    default_user_agent_string_(true),
    // FIXME: Get from translations
    accept_langs_("en-us,en") {}

net::SSLConfigService*
BrowserContextIODataImpl::ssl_config_service() const {
  return ssl_config_service_.get();
}

net::HttpUserAgentSettings*
BrowserContextIODataImpl::http_user_agent_settings() const {
  return http_user_agent_settings_.get();
}

base::FilePath BrowserContextIODataImpl::GetPath() const {
  return path_;
}

base::FilePath BrowserContextIODataImpl::GetCachePath() const {
  if (cache_path_.empty()) {
    return GetPath();
  }

  return cache_path_;
}

// Called on IO thread and UI thread
std::string BrowserContextIODataImpl::GetAcceptLangs() const {
  base::AutoLock lock(lock_);
  // Force a copy now to avoid COW races
  return std::string(accept_langs_.c_str());
}

// Only called on UI thread
void BrowserContextIODataImpl::SetAcceptLangs(const std::string& langs) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  base::AutoLock lock(lock_);
  accept_langs_ = langs;
}

// Only called on UI thread
std::string BrowserContextIODataImpl::GetProduct() const {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  return product_;
}

// Only called on UI thread
void BrowserContextIODataImpl::SetProduct(const std::string& product) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  product_ = product.empty() ?
      base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING) : product;
  if (default_user_agent_string_) {
    SetUserAgent(std::string());
  }
}

// Called on IO thread and UI thread
std::string BrowserContextIODataImpl::GetUserAgent() const {
  base::AutoLock lock(lock_);
  // Force a copy now to avoid COW races
  return std::string(user_agent_.c_str());
}

// Only called on UI thread
void BrowserContextIODataImpl::SetUserAgent(
    const std::string& user_agent) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  base::AutoLock lock(lock_);
  user_agent_ = user_agent.empty() ?
      webkit_glue::BuildUserAgentFromProduct(product_) :
      user_agent;
  default_user_agent_string_ = user_agent.empty();
}

bool BrowserContextIODataImpl::IsOffTheRecord() const {
  if (GetPath().empty()) {
    return true;
  }

  return false;
}

BrowserContextImpl::BrowserContextImpl(const base::FilePath& path,
                                       const base::FilePath& cache_path) :
    BrowserContext(new BrowserContextIODataImpl(path, cache_path)),
    user_script_manager_(this) {}

BrowserContext* BrowserContextImpl::GetOffTheRecordContext() {
  if (!otr_context_) {
    otr_context_.reset(new OffTheRecordBrowserContextImpl(this));
  }

  return otr_context_.get();
}

BrowserContext* BrowserContextImpl::GetOriginalContext() {
  return this;
}

UserScriptMaster& BrowserContextImpl::UserScriptManager() {
  return user_script_manager_;
}

} // namespace oxide
