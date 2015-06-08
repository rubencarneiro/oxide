// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_WEB_VIEW_CONTENTS_HELPER_H_
#define _OXIDE_SHARED_BROWSER_WEB_VIEW_CONTENTS_HELPER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/supports_user_data.h"

#include "shared/browser/oxide_browser_context_observer.h"
#include "shared/browser/oxide_web_preferences_observer.h"

namespace content {
class RenderViewHost;
class WebContents;
}

namespace oxide {

class BrowserContext;
class WebPreferences;

class WebViewContentsHelper final : private BrowserContextObserver,
                                    private WebPreferencesObserver,
                                    private base::SupportsUserData::Data {
 public:
  WebViewContentsHelper(content::WebContents* contents,
                        content::WebContents* opener);

  static WebViewContentsHelper* FromWebContents(content::WebContents* contents);
  static WebViewContentsHelper* FromRenderViewHost(content::RenderViewHost* rvh);

  content::WebContents* GetWebContents() const;
  BrowserContext* GetBrowserContext() const;

  WebPreferences* GetWebPreferences() const;
  void SetWebPreferences(WebPreferences* preferences);

  void WebContentsAdopted();

 private:
  ~WebViewContentsHelper();

  void UpdateWebPreferences();

  // BrowserContextObserver implementation
  void NotifyPopupBlockerEnabledChanged() final;

  // WebPreferencesObserver implementation
  void WebPreferencesValueChanged() final;

  scoped_refptr<BrowserContext> context_;
  content::WebContents* web_contents_;

  bool owns_web_preferences_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebViewContentsHelper);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_CONTENTS_HELPER_H_
