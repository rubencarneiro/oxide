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

BrowserContext* OffTheRecordBrowserContextImpl::GetOffTheRecordContext() {
  return this;
}

BrowserContext* OffTheRecordBrowserContextImpl::GetOriginalContext() {
  if (original_context_) {
    return original_context_;
  }

  return this;
}

base::FilePath OffTheRecordBrowserContextImpl::GetPath() {
  return base::FilePath();
}

bool OffTheRecordBrowserContextImpl::IsOffTheRecord() const {
  return true;
}

} // namespace oxide
