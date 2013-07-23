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

#include "oxide_global_settings.h"

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/strings/stringprintf.h"
#include "content/public/browser/browser_thread.h"
#include "webkit/common/user_agent/user_agent_util.h"

#include "oxide/browser/oxide_browser_process_main.h"
#include "oxide/common/chrome_version.h"
#include "oxide/common/oxide_content_client.h"

namespace oxide {

// static
GlobalSettings* GlobalSettings::GetInstance() {
  return Singleton<GlobalSettings>::get();
}

// static
std::string GlobalSettings::GetProduct() {
  DCHECK(!content::BrowserThread::IsWellKnownThread(content::BrowserThread::UI) ||
         content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  GlobalSettings* self = GetInstance();
  if (!self->product_.empty()) {
    return self->product_;
  }

  return base::StringPrintf("Chrome/%s", CHROME_VERSION_STRING);
}

// static
void GlobalSettings::SetProduct(const std::string& product) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  GetInstance()->product_ = product;
  ContentClient::MaybeUpdateUserAgent();
}

// static
std::string GlobalSettings::GetUserAgent() {
  DCHECK(!content::BrowserThread::IsWellKnownThread(content::BrowserThread::UI) ||
         content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  GlobalSettings* self = GetInstance();
  if (!self->user_agent_.empty()) {
    return self->user_agent_;
  }

  
  return webkit_glue::BuildUserAgentFromProduct(GetProduct());
}

// static
void GlobalSettings::SetUserAgent(const std::string& user_agent) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  GetInstance()->user_agent_ = user_agent;
  ContentClient::MaybeUpdateUserAgent();
}

// static
std::string GlobalSettings::GetDataPath() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  return GetInstance()->data_path_;
}

// static
bool GlobalSettings::SetDataPath(const std::string& data_path) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  GlobalSettings* self = GetInstance();
  if (BrowserProcessMain::Exists()) {
    LOG(ERROR) << "It's too late to set the data path";
    return false;
  }

  self->data_path_ = data_path;

  return true;
}

// static
std::string GlobalSettings::GetCachePath() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  return GetInstance()->cache_path_;
}

// static
bool GlobalSettings::SetCachePath(const std::string& cache_path) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  GlobalSettings* self = GetInstance();
  if (BrowserProcessMain::Exists()) {
    LOG(ERROR) << "It's too late to set the cache path";
    return false;
  }

  self->cache_path_ = cache_path;

  return true;
}

// static
std::string GlobalSettings::GetAcceptLangs() {
  GlobalSettings* self = GetInstance();

  {
    base::AutoLock lock(self->lock_);

    if (!self->accept_langs_.empty()) {
      return self->accept_langs_;
    }
  }

  // TODO: Lookup actual accept langs
  return std::string("en-us,en");
}

// static
void GlobalSettings::SetAcceptLangs(const std::string& accept_langs) {
  GlobalSettings* self = GetInstance();

  base::AutoLock lock(self->lock_);
  self->accept_langs_ = accept_langs;
}

} // namespace oxide
