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
#include "base/memory/scoped_ptr.h"

#include "shared/browser/oxide_javascript_dialog_manager.h"
#include "shared/browser/oxide_web_view.h"

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
QT_END_NAMESPACE

class OxideQSecurityStatus;

namespace oxide {
namespace qt {

class InputMethodListener;
class WebViewAdapter;

class WebView FINAL : public oxide::WebView {
 public:
  static WebView* Create(WebViewAdapter* adapter);
  ~WebView();

  WebViewAdapter* adapter() const { return adapter_; }

  OxideQSecurityStatus* qsecurity_status() { return qsecurity_status_.get(); }

  void HandleFocusEvent(QFocusEvent* event);
  void HandleInputMethodEvent(QInputMethodEvent* event);
  void HandleKeyEvent(QKeyEvent* event);
  void HandleMouseEvent(QMouseEvent* event);
  void HandleTouchEvent(QTouchEvent* event);
  void HandleWheelEvent(QWheelEvent* event);

  QVariant InputMethodQuery(Qt::InputMethodQuery query) const;

 private:
  friend class InputMethodListener;
  friend class WebViewAdapter;

  WebView(WebViewAdapter* adapter);

  float GetDeviceScaleFactor() const;

  bool ShouldShowInputPanel() const;
  bool ShouldHideInputPanel() const;
  void SetInputPanelVisibility(bool visible);

  // WebView implementation
  void Init(oxide::WebView::Params* params) FINAL;

  void UpdateCursor(const content::WebCursor& cursor) FINAL;
  void ImeCancelComposition() FINAL;
  void SelectionChanged() FINAL;

  blink::WebScreenInfo GetScreenInfo() const FINAL;
  gfx::Rect GetViewBoundsPix() const FINAL;
  bool IsVisible() const FINAL;
  bool HasFocus() const FINAL;
  bool IsInputPanelVisible() const FINAL;

  oxide::JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type,
      bool* did_suppress_message) FINAL;
  oxide::JavaScriptDialog* CreateBeforeUnloadDialog() FINAL;

  void FrameAdded(oxide::WebFrame* frame) FINAL;
  void FrameRemoved(oxide::WebFrame* frame) FINAL;

  bool CanCreateWindows() const FINAL;

  size_t GetScriptMessageHandlerCount() const FINAL;
  oxide::ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const FINAL;

  void OnURLChanged() FINAL;
  void OnTitleChanged() FINAL;
  void OnIconChanged(const GURL& icon) FINAL;
  void OnCommandsUpdated() FINAL;

  void OnLoadingChanged() FINAL;
  void OnLoadProgressChanged(double progress) FINAL;

  void OnLoadStarted(const GURL& validated_url,
                     bool is_error_frame) FINAL;
  void OnLoadCommitted(const GURL& url) FINAL;
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
      const GURL& origin,
      const GURL& embedder,
      scoped_ptr<oxide::SimplePermissionRequest> request) FINAL;

  void OnUnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) FINAL;

  void OnFrameMetadataUpdated(const cc::CompositorFrameMetadata& old) FINAL;

  void OnDownloadRequested(const GURL& url,
			   const std::string& mimeType,
			   const bool shouldPrompt,
			   const base::string16& suggestedFilename,
			   const std::string& cookies,
			   const std::string& referrer) FINAL;

  void OnLoadRedirected(const GURL& url,
                        const GURL& original_url) FINAL;

  bool ShouldHandleNavigation(const GURL& url,
                              WindowOpenDisposition disposition,
                              bool user_gesture) FINAL;

  oxide::WebFrame* CreateWebFrame(content::FrameTreeNode* node) FINAL;
  oxide::WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh) FINAL;

  oxide::WebView* CreateNewWebView(const gfx::Rect& initial_pos,
                                   WindowOpenDisposition disposition) FINAL;

  oxide::FilePicker* CreateFilePicker(content::RenderViewHost* rvh) FINAL;

  void OnSwapCompositorFrame() FINAL;
  void OnEvictCurrentFrame() FINAL;

  void OnTextInputStateChanged() FINAL;
  void OnFocusedNodeChanged() FINAL;
  void OnSelectionBoundsChanged() FINAL;

  void OnSecurityStatusChanged(const oxide::SecurityStatus& old) FINAL;
  bool OnCertificateError(
      bool is_main_frame,
      oxide::CertError cert_error,
      const scoped_refptr<net::X509Certificate>& cert,
      const GURL& request_url,
      content::ResourceType resource_type,
      bool strict_enforcement,
      scoped_ptr<oxide::SimplePermissionRequest> request) FINAL;
  void OnContentBlocked() FINAL;

  WebViewAdapter* adapter_;

  bool has_input_method_state_;

  scoped_ptr<InputMethodListener> input_method_listener_;
  scoped_ptr<OxideQSecurityStatus> qsecurity_status_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_
