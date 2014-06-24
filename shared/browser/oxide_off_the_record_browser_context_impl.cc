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

#include "oxide_off_the_record_browser_context_impl.h"

#include "base/files/file_path.h"
#include "base/logging.h"

#include "oxide_browser_context_impl.h"

namespace oxide {

OffTheRecordBrowserContextIODataImpl::OffTheRecordBrowserContextIODataImpl(
    BrowserContextIOData* original_io_data) :
    original_io_data_(original_io_data) {}

net::StaticCookiePolicy::Type
OffTheRecordBrowserContextIODataImpl::GetCookiePolicy() const {
  return original_io_data_->GetCookiePolicy();
}

content::CookieStoreConfig::SessionCookieMode
OffTheRecordBrowserContextIODataImpl::GetSessionCookieMode() const {
  return content::CookieStoreConfig::EPHEMERAL_SESSION_COOKIES;
}

bool OffTheRecordBrowserContextIODataImpl::IsPopupBlockerEnabled() const {
  return original_io_data_->IsPopupBlockerEnabled();
}

base::FilePath
OffTheRecordBrowserContextIODataImpl::GetPath() const {
  return original_io_data_->GetPath();
}

base::FilePath
OffTheRecordBrowserContextIODataImpl::GetCachePath() const {
  return original_io_data_->GetCachePath();
}

std::string
OffTheRecordBrowserContextIODataImpl::GetAcceptLangs() const {
  return original_io_data_->GetAcceptLangs();
}

std::string
OffTheRecordBrowserContextIODataImpl::GetUserAgent() const {
  return original_io_data_->GetUserAgent();
}

bool OffTheRecordBrowserContextIODataImpl::IsOffTheRecord() const {
  return true;
}

OffTheRecordBrowserContextImpl::OffTheRecordBrowserContextImpl(
    BrowserContextImpl* original_context) :
    BrowserContext(new OffTheRecordBrowserContextIODataImpl(
                      original_context->io_data())),
    original_context_(original_context) {
  DCHECK(original_context_);
}

BrowserContext* OffTheRecordBrowserContextImpl::GetOffTheRecordContext() {
  return this;
}

BrowserContext* OffTheRecordBrowserContextImpl::GetOriginalContext() {
  return original_context_;
}

void OffTheRecordBrowserContextImpl::SetAcceptLangs(const std::string& langs) {
  original_context_->SetAcceptLangs(langs);
}

std::string OffTheRecordBrowserContextImpl::GetProduct() const {
  return original_context_->GetProduct();
}

void OffTheRecordBrowserContextImpl::SetProduct(const std::string& product) {
  original_context_->SetProduct(product);
}

void OffTheRecordBrowserContextImpl::SetUserAgent(
    const std::string& user_agent) {
  original_context_->SetUserAgent(user_agent);
}

void OffTheRecordBrowserContextImpl::SetCookiePolicy(
    net::StaticCookiePolicy::Type policy) {
  original_context_->SetCookiePolicy(policy);
}

void OffTheRecordBrowserContextImpl::SetIsPopupBlockerEnabled(bool enabled) {
  original_context_->SetIsPopupBlockerEnabled(enabled);
}

UserScriptMaster& OffTheRecordBrowserContextImpl::UserScriptManager() {
  return original_context_->UserScriptManager();
}

bool OffTheRecordBrowserContextImpl::GetDevtoolsEnabled() const {
  return original_context_->GetDevtoolsEnabled();
}

int OffTheRecordBrowserContextImpl::GetDevtoolsPort() const {
  return original_context_->GetDevtoolsPort();
}


} // namespace oxide
