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

#include "oxide_devtools_http_handler_delegate.h"

#include "content/public/browser/devtools_frontend_host.h"
#include "ui/base/resource/resource_bundle.h"

#include "grit/oxide_resources.h"

namespace oxide {

DevtoolsHttpHandlerDelegate::DevtoolsHttpHandlerDelegate() {}

DevtoolsHttpHandlerDelegate::~DevtoolsHttpHandlerDelegate() {}

std::string DevtoolsHttpHandlerDelegate::GetDiscoveryPageHTML() {
  return ui::ResourceBundle::GetSharedInstance().GetRawDataResource(
    IDR_OXIDE_DEVTOOLS_DISCOVERY_HTML_PAGE).as_string();
}

std::string DevtoolsHttpHandlerDelegate::GetFrontendResource(
    const std::string& path) {
  return content::DevToolsFrontendHost::GetFrontendResource(path).as_string();
}

}
