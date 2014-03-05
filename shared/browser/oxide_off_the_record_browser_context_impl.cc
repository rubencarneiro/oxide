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

base::FilePath
OffTheRecordBrowserContextIODataImpl::GetPath() const {
  return base::FilePath();
}

bool OffTheRecordBrowserContextIODataImpl::SetPath(
    const base::FilePath& path) {
  LOG(ERROR) << "Cannot set the data path for the OTR context";
  return false;
}

base::FilePath
OffTheRecordBrowserContextIODataImpl::GetCachePath() const {
  return base::FilePath();
}

bool OffTheRecordBrowserContextIODataImpl::SetCachePath(
    const base::FilePath& cache_path) {
  LOG(ERROR) << "Cannot set the cache path for the OTR context";
  return false;
}

std::string
OffTheRecordBrowserContextIODataImpl::GetAcceptLangs() const {
  return original_io_data_->GetAcceptLangs();
}

void OffTheRecordBrowserContextIODataImpl::SetAcceptLangs(
    const std::string& langs) {
  original_io_data_->SetAcceptLangs(langs);
}

std::string
OffTheRecordBrowserContextIODataImpl::GetProduct() const {
  return original_io_data_->GetProduct();
}

void OffTheRecordBrowserContextIODataImpl::SetProduct(
    const std::string& product) {
  original_io_data_->SetProduct(product);
}

std::string
OffTheRecordBrowserContextIODataImpl::GetUserAgent() const {
  return original_io_data_->GetUserAgent();
}

void OffTheRecordBrowserContextIODataImpl::SetUserAgent(
    const std::string& user_agent) {
  original_io_data_->SetUserAgent(user_agent);
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

UserScriptMaster& OffTheRecordBrowserContextImpl::UserScriptManager() {
  return original_context_->UserScriptManager();
}

} // namespace oxide
