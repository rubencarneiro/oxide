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

#include "oxide_content_client.h"

#include "base/memory/singleton.h"

#include "oxide/browser/oxide_content_browser_client.h"
#include "oxide/browser/oxide_global_settings.h"

namespace oxide {

ContentBrowserClient* ContentClient::browser() {
  return static_cast<ContentBrowserClient *>(
      content::ContentClient::browser());
}

std::string ContentClient::GetProduct() const {
  return GlobalSettings::GetProduct();
}

std::string ContentClient::GetUserAgent() const {
  return GlobalSettings::GetUserAgent();
}

// static
ContentClient* ContentClient::GetInstance() {
  // XXX: Port-specific derivates of ContentClient will provide their own
  //      GetInstance() which will be called from here
  return Singleton<ContentClient>::get();
}

} // namespace oxide
