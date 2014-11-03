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

#ifndef _OXIDE_QT_CORE_BROWSER_CONTENT_BROWSER_CLIENT_H_
#define _OXIDE_QT_CORE_BROWSER_CONTENT_BROWSER_CLIENT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"

#include "shared/browser/oxide_content_browser_client.h"

namespace base {
template <typename Type> struct DefaultLazyInstanceTraits;
}

namespace oxide {
namespace qt {

class ContentBrowserClient final : public oxide::ContentBrowserClient {
  // Limit default constructor access to the lazy instance initializer
  friend struct base::DefaultLazyInstanceTraits<ContentBrowserClient>;
  ContentBrowserClient();

  // oxide::ContentBrowserClient implementation
  oxide::WebPreferences* CreateWebPreferences() final;
  oxide::BrowserMainParts::Delegate* CreateBrowserMainPartsDelegate() final;

  // content::ContentBrowserClient implementation
  content::LocationProvider* OverrideSystemLocationProvider() final;

  DISALLOW_COPY_AND_ASSIGN(ContentBrowserClient);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_CONTENT_BROWSER_CLIENT_H_
