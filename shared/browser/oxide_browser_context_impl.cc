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

#include "oxide_global_settings.h"
#include "oxide_off_the_record_browser_context_impl.h"

namespace oxide {

BrowserContextImpl::BrowserContextImpl() :
    path_(GlobalSettings::GetDataPath()) {
  DCHECK(!GlobalSettings::GetDataPath().empty()) <<
      "We shouldn't have a persistent context without a data path!";
}

BrowserContext* BrowserContextImpl::GetOffTheRecordContext() {
  if (!otr_context_) {
    otr_context_.reset(new OffTheRecordBrowserContextImpl(this));
  }

  return otr_context_.get();
}

BrowserContext* BrowserContextImpl::GetOriginalContext() {
  return this;
}

base::FilePath BrowserContextImpl::GetPath() {
  return path_;
}

bool BrowserContextImpl::IsOffTheRecord() const {
  return false;
}

} // namespace oxide
