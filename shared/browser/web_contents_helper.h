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

#ifndef _OXIDE_SHARED_BROWSER_WEB_CONTENTS_HELPER_H_
#define _OXIDE_SHARED_BROWSER_WEB_CONTENTS_HELPER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/web_contents_user_data.h"

#include "shared/browser/oxide_browser_context_observer.h"
#include "shared/browser/oxide_web_preferences_observer.h"
#include "shared/browser/screen_observer.h"

namespace content {
class RenderViewHost;
class WebContents;
}

namespace oxide {

class BrowserContext;
class WebPreferences;

class WebContentsHelper
    : public content::WebContentsUserData<WebContentsHelper>,
      public BrowserContextObserver,
      public ScreenObserver,
      public WebPreferencesObserver {
 public:
  static void CreateForWebContents(content::WebContents* contents,
                                   content::WebContents* opener);

  static WebContentsHelper* FromRenderViewHost(content::RenderViewHost* rvh);

  static bool IsContextInUse(BrowserContext* context);

  content::WebContents* GetWebContents() const;
  BrowserContext* GetBrowserContext() const;

  WebPreferences* GetWebPreferences() const;
  void SetWebPreferences(WebPreferences* preferences);

  void WebContentsAdopted();

 private:
  friend class content::WebContentsUserData<WebContentsHelper>;

  WebContentsHelper(content::WebContents* contents);
  ~WebContentsHelper() override;

  void InitFromOpener(content::WebContents* opener);

  void UpdateWebPreferences();

  // BrowserContextObserver implementation
  void NotifyPopupBlockerEnabledChanged() override;
  void NotifyDoNotTrackChanged() override;

  // ScreenObserver
  void OnDisplayPropertiesChanged(const display::Display& display) override;
  void OnShellModeChanged() override;

  // WebPreferencesObserver implementation
  void WebPreferencesValueChanged() override;

  content::WebContents* web_contents_;

  bool owns_web_preferences_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsHelper);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTENTS_HELPER_H_
