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
#include "content/public/browser/web_contents_delegate.h"

#include "shared/browser/oxide_browser_context_observer.h"
#include "shared/browser/oxide_web_preferences_observer.h"

namespace content {
class RenderViewHost;
class WebContents;
}

namespace oxide {

class BrowserContext;
class WebPreferences;
class WebViewContentsHelperDelegate;

class WebViewContentsHelper final : private BrowserContextObserver,
                                    private WebPreferencesObserver,
                                    private base::SupportsUserData::Data,
                                    private content::WebContentsDelegate {
 public:
  static void Attach(content::WebContents* contents,
                     content::WebContents* opener = NULL);

  static WebViewContentsHelper* FromWebContents(content::WebContents* contents);
  static WebViewContentsHelper* FromRenderViewHost(content::RenderViewHost* rvh);

  void SetDelegate(WebViewContentsHelperDelegate* delegate);

  BrowserContext* GetBrowserContext() const;

  WebPreferences* GetWebPreferences() const;
  void SetWebPreferences(WebPreferences* preferences);

  void TakeWebContentsOwnershipAndClosePage(
      scoped_ptr<content::WebContents> web_contents);

 private:
  WebViewContentsHelper(content::WebContents* contents);

  ~WebViewContentsHelper();

  void UpdateWebPreferences();

  // BrowserContextObserver implementation
  void NotifyPopupBlockerEnabledChanged() final;

  // WebPreferencesObserver implementation
  void WebPreferencesDestroyed() final;
  void WebPreferencesValueChanged() final;
  void WebPreferencesAdopted() final;

  // content::WebContentsDelegate implementation
  void CloseContents(content::WebContents* source) final;

  scoped_refptr<BrowserContext> context_;
  content::WebContents* web_contents_;
  WebViewContentsHelperDelegate* delegate_;

  // WebPreferences are normally owned by the public object exposed to
  // the embedder. However, we create an internal WebPreferences instance
  // at construction time that is initially owned by us until it is
  // "adopted" by the embedder
  bool owns_web_preferences_;

  // When deleting the WebView, we take ownership of the WebContents
  // whilst we wait for the unload handler to finish. This allows us to
  // run the unload handler completely transparently to the application
  scoped_ptr<content::WebContents> web_contents_holder_during_close_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebViewContentsHelper);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_CONTENTS_HELPER_H_
