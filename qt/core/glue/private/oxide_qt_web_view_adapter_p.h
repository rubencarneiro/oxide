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

#ifndef _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_VIEW_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_VIEW_ADAPTER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"

#include "shared/browser/oxide_javascript_dialog_manager.h"
#include "shared/browser/oxide_web_view.h"

namespace oxide {
namespace qt {

class WebViewAdapter;

class WebViewAdapterPrivate FINAL : public oxide::WebView {
 public:
  static WebViewAdapterPrivate* Create(WebViewAdapter* adapter);

  size_t GetMessageHandlerCount() const FINAL;
  oxide::MessageHandler* GetMessageHandlerAt(size_t index) const FINAL;

  content::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) FINAL;

  gfx::Rect GetContainerBounds() FINAL;

  oxide::WebPopupMenu* CreatePopupMenu(content::RenderViewHost* rvh) FINAL;

  void RunJavaScriptDialog(
      const GURL& origin_url,
      const std::string& accept_lang,
      content::JavaScriptMessageType javascript_message_type,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback,
      bool* did_suppress_message) FINAL;
  void RunBeforeUnloadDialog(
      const base::string16& message_text,
      bool is_reload,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback) FINAL;
  bool HandleJavaScriptDialog(bool accept,
                              const base::string16* prompt_override) FINAL;

 private:
  WebViewAdapterPrivate(WebViewAdapter* adapter);

  void OnURLChanged() FINAL;
  void OnTitleChanged() FINAL;
  void OnCommandsUpdated() FINAL;

  void OnRootFrameChanged() FINAL;

  void OnLoadStarted(const GURL& validated_url,
                     bool is_error_frame) FINAL;
  void OnLoadStopped(const GURL& validated_url) FINAL;
  void OnLoadFailed(const GURL& validated_url,
                    int error_code,
                    const std::string& error_description) FINAL;
  void OnLoadSucceeded(const GURL& validated_url) FINAL;

  oxide::WebFrame* CreateWebFrame() FINAL;

  WebViewAdapter* a;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebViewAdapterPrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_PRIVATE_WEB_VIEW_ADAPTER_H_
