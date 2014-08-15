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

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/supports_user_data.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"

#include "shared/browser/oxide_browser_context_observer.h"
#include "shared/browser/oxide_web_preferences_observer.h"

namespace oxide {

class BrowserContext;
class WebPreferences;
class WebViewContentsHelperDelegate;

class WebViewContentsHelper FINAL : private BrowserContextObserver,
                                    private WebPreferencesObserver,
                                    private base::SupportsUserData::Data,
                                    private content::WebContentsDelegate,
                                    private content::WebContentsObserver {
 public:
  static void Attach(content::WebContents* contents,
                     content::WebContents* opener = NULL);

  static WebViewContentsHelper* FromWebContents(content::WebContents* contents);
  static WebViewContentsHelper* FromRenderViewHost(content::RenderViewHost* rvh);

  void SetDelegate(WebViewContentsHelperDelegate* delegate);

  BrowserContext* GetBrowserContext() const;

  WebPreferences* GetWebPreferences() const;
  void SetWebPreferences(WebPreferences* preferences);

 private:
  WebViewContentsHelper(content::WebContents* contents);

  ~WebViewContentsHelper();

  void UpdateWebPreferences();

  // BrowserContextObserver implementation
  void NotifyPopupBlockerEnabledChanged() FINAL;

  // WebPreferencesObserver implementation
  void WebPreferencesDestroyed() FINAL;
  void WebPreferencesValueChanged() FINAL;
  void WebPreferencesAdopted() FINAL;

  // content::WebContentsDelegate implementation
  content::WebContents* OpenURLFromTab(content::WebContents* source,
                                       const content::OpenURLParams& params) FINAL;
  void NavigationStateChanged(const content::WebContents* source,
                              content::InvalidateTypes changed_flags) FINAL;
  bool ShouldCreateWebContents(
      content::WebContents* source,
      int route_id,
      WindowContainerType window_container_type,
      const base::string16& frame_name,
      const GURL& target_url,
      const std::string& partition_id,
      content::SessionStorageNamespace* session_storage_namespace,
      WindowOpenDisposition disposition,
      bool user_gesture) FINAL;
  void HandleKeyboardEvent(content::WebContents* source,
                           const content::NativeWebKeyboardEvent& event) FINAL;
  void WebContentsCreated(content::WebContents* source,
                          int source_frame_id,
                          const base::string16& frame_name,
                          const GURL& target_url,
                          content::WebContents* new_contents) FINAL;
  void AddNewContents(content::WebContents* source,
                      content::WebContents* new_contents,
                      WindowOpenDisposition disposition,
                      const gfx::Rect& initial_pos,
                      bool user_gesture,
                      bool* was_blocked) FINAL;
  void LoadProgressChanged(content::WebContents* source, double progress) FINAL;
  bool AddMessageToConsole(content::WebContents* source,
               int32 level,
               const base::string16& message,
               int32 line_no,
               const base::string16& source_id) FINAL;
  content::JavaScriptDialogManager* GetJavaScriptDialogManager() FINAL;
  void RunFileChooser(content::WebContents* web_contents,
                      const content::FileChooserParams& params) FINAL;
  void ToggleFullscreenModeForTab(content::WebContents* source,
                                  bool enter) FINAL;
  bool IsFullscreenForTabOrPending(
      const content::WebContents* source) const FINAL;

  scoped_refptr<BrowserContext> context_;
  WebViewContentsHelperDelegate* delegate_;

  // WebPreferences are normally owned by the public object exposed to
  // the embedder. However, we create an internal WebPreferences instance
  // at construction time that is initially owned by us until it is
  // "adopted" by the embedder
  bool owns_web_preferences_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebViewContentsHelper);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_WEB_VIEW_CONTENTS_HELPER_H_
