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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_

#include <QKeyEvent>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"
#include "ui/base/ime/text_input_type.h"

#include "qt/core/base/oxide_qt_event_utils.h"
#include "shared/browser/oxide_javascript_dialog_manager.h"
#include "shared/browser/oxide_web_view.h"

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class WebViewAdapter;

class WebView FINAL : public oxide::WebView,
                      public base::SupportsWeakPtr<WebView> {
 public:
  static WebView* Create(WebViewAdapter* adapter);

  WebViewAdapter* adapter() const { return adapter_; }

  void HandleFocusEvent(QFocusEvent* event);
  void HandleInputMethodEvent(QInputMethodEvent* event);
  void HandleKeyEvent(QKeyEvent* event);
  void HandleMouseEvent(QMouseEvent* event);
  void HandleTouchEvent(QTouchEvent* event);
  void HandleWheelEvent(QWheelEvent* event);

  QVariant InputMethodQuery(Qt::InputMethodQuery query) const;

 private:
  friend class WebViewAdapter;

  WebView(WebViewAdapter* adapter);

  float GetDeviceScaleFactor() const;

  bool ShouldShowInputPanel() const;
  bool ShouldHideInputPanel() const;
  void SetInputPanelVisibility(bool visible);

  // WebView implementation
  void Init(oxide::WebView::Params* params) FINAL;

  blink::WebScreenInfo GetScreenInfo() const FINAL;
  gfx::Rect GetContainerBoundsPix() const FINAL;
  bool IsVisible() const FINAL;
  bool HasFocus() const FINAL;

  oxide::JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type,
      bool* did_suppress_message) FINAL;
  oxide::JavaScriptDialog* CreateBeforeUnloadDialog() FINAL;

  void FrameAdded(oxide::WebFrame* frame) FINAL;
  void FrameRemoved(oxide::WebFrame* frame) FINAL;

  bool CanCreateWindows() const FINAL;

  void UpdateCursor(const content::WebCursor& cursor) FINAL;
  void TextInputStateChanged(ui::TextInputType type,
                             bool show_ime_if_needed) FINAL;
  void FocusedNodeChanged(bool is_editable_node) FINAL;
  void ImeCancelComposition() FINAL;

  size_t GetScriptMessageHandlerCount() const FINAL;
  oxide::ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const FINAL;

  void OnURLChanged() FINAL;
  void OnTitleChanged() FINAL;
  void OnIconChanged(const GURL& icon) FINAL;
  void OnCommandsUpdated() FINAL;

  void OnLoadProgressChanged(double progress) FINAL;

  void OnLoadStarted(const GURL& validated_url,
                     bool is_error_frame) FINAL;
  void OnLoadStopped(const GURL& validated_url) FINAL;
  void OnLoadFailed(const GURL& validated_url,
                    int error_code,
                    const std::string& error_description) FINAL;
  void OnLoadSucceeded(const GURL& validated_url) FINAL;

  void OnNavigationEntryCommitted() FINAL;
  void OnNavigationListPruned(bool from_front, int count) FINAL;
  void OnNavigationEntryChanged(int index) FINAL;

  bool OnAddMessageToConsole(int32 level,
                             const base::string16& message,
                             int32 line_no,
                             const base::string16& source_id) FINAL;

  void OnToggleFullscreenMode(bool enter) FINAL;

  void OnWebPreferencesDestroyed() FINAL;

  void OnRequestGeolocationPermission(
      scoped_ptr<oxide::GeolocationPermissionRequest> request) FINAL;

  void OnUnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) FINAL;

  void OnFrameMetadataUpdated(const cc::CompositorFrameMetadata& old) FINAL;

  void OnDownloadRequested(const GURL& url,
			   const std::string& mimeType,
			   const bool shouldPrompt,
			   const base::string16& suggestedFilename,
			   const std::string& cookies,
			   const std::string& referrer) FINAL;

  bool ShouldHandleNavigation(const GURL& url,
                              WindowOpenDisposition disposition,
                              bool user_gesture) FINAL;

  oxide::WebFrame* CreateWebFrame(content::FrameTreeNode* node) FINAL;
  oxide::WebPopupMenu* CreatePopupMenu(content::RenderViewHost* rvh) FINAL;

  oxide::WebView* CreateNewWebView(const gfx::Rect& initial_pos,
                                   WindowOpenDisposition disposition) FINAL;

  oxide::FilePicker* CreateFilePicker(content::RenderViewHost* rvh) FINAL;

  void OnSwapCompositorFrame() FINAL;
  void OnEvictCurrentFrame() FINAL;

  WebViewAdapter* adapter_;

  TouchIDMap touch_id_map_;

  ui::TextInputType text_input_type_;
  bool show_ime_if_needed_;
  bool focused_node_is_editable_;

  bool has_input_method_state_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_
