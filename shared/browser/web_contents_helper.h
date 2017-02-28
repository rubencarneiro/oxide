// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#include <memory>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/web_contents_user_data.h"

#include "shared/browser/oxide_user_agent_settings_observer.h"
#include "shared/browser/screen_observer.h"
#include "shared/browser/web_contents_unique_ptr.h"
#include "shared/browser/web_preferences.h"
#include "shared/common/oxide_shared_export.h"

namespace content {
class RenderFrameHost;
class RenderViewHost;
class WebContents;
}

namespace oxide {

class BrowserContext;
class WebContentsClient;

class OXIDE_SHARED_EXPORT WebContentsHelper
    : public content::WebContentsUserData<WebContentsHelper>,
      public ScreenObserver,
      public UserAgentSettingsObserver {
 public:
  static WebContentsUniquePtr CreateWebContents(
      const content::WebContents::CreateParams& params);

  static void CreateForWebContents(content::WebContents* contents,
                                   content::WebContents* opener);

  static WebContentsHelper* FromRenderFrameHost(content::RenderFrameHost* rfh);
  static WebContentsHelper* FromRenderViewHost(content::RenderViewHost* rvh);

  static bool IsContextInUse(BrowserContext* context);

  WebContentsClient* client() const { return client_; }
  void SetClient(WebContentsClient* client);

  content::WebContents* GetWebContents() const;
  BrowserContext* GetBrowserContext() const;

  WebPreferences* GetPreferences() const;
  void SetPreferences(WebPreferences* preferences);

  void SyncWebPreferences();

 private:
  friend class content::WebContentsUserData<WebContentsHelper>;

  WebContentsHelper(content::WebContents* contents);
  ~WebContentsHelper() override;

  void SyncRendererPreferences();

  // ScreenObserver
  void OnDisplayPropertiesChanged(const display::Display& display) override;
  void OnShellModeChanged() override;

  // UserAgentSettingsObserver implementation
  void NotifyPopupBlockerEnabledChanged() override;
  void NotifyDoNotTrackChanged() override;
  void NotifyAcceptLanguagesChanged() override;

  content::WebContents* web_contents_;

  WebContentsClient* client_ = nullptr;

  scoped_refptr<WebPreferences> preferences_;

  // |preferences_| must outlive this
  std::unique_ptr<WebPreferences::Subscription> prefs_change_subscription_;

  DISALLOW_COPY_AND_ASSIGN(WebContentsHelper);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_CONTENTS_HELPER_H_
