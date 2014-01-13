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

#include "oxide_qt_content_main_delegate.h"

#include "base/lazy_instance.h"

#include "qt/core/browser/oxide_qt_content_browser_client.h"
#include "qt/core/common/oxide_qt_content_client.h"

namespace oxide {
namespace qt {

namespace {
base::LazyInstance<ContentBrowserClient> g_content_browser_client =
    LAZY_INSTANCE_INITIALIZER;
}

content::ContentBrowserClient*
ContentMainDelegate::CreateContentBrowserClient() {
  return g_content_browser_client.Pointer();
}

oxide::ContentClient* ContentMainDelegate::CreateContentClient() {
  return ContentClient::GetInstance();
}

ContentMainDelegate::ContentMainDelegate() {}

} // namespace qt

// static
ContentMainDelegate* ContentMainDelegate::Create() {
  return new qt::ContentMainDelegate();
}

} // namespace oxide
