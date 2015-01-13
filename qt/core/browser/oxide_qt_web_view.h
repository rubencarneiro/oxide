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
#include <QObject>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"

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

class OxideQSecurityStatus;

namespace oxide {
namespace qt {

class WebContext;
class WebViewAdapter;

class WebView final : public QObject,
                      public oxide::WebView {
  Q_OBJECT

 public:
  static WebView* Create(WebViewAdapter* adapter);
  ~WebView();

  OxideQSecurityStatus* qsecurity_status() { return qsecurity_status_.get(); }

  void HandleFocusEvent(QFocusEvent* event);
  void HandleInputMethodEvent(QInputMethodEvent* event);
  void HandleKeyEvent(QKeyEvent* event);
  void HandleMouseEvent(QMouseEvent* event);
  void HandleTouchEvent(QTouchEvent* event);
  void HandleWheelEvent(QWheelEvent* event);

  QVariant InputMethodQuery(Qt::InputMethodQuery query) const;

  void SetCanTemporarilyDisplayInsecureContent(bool allow);
  void SetCanTemporarilyRunInsecureContent(bool allow);

  WebContext* GetContext() const;

 private Q_SLOTS:
  void OnInputPanelVisibilityChanged();

 private:
  friend class WebViewAdapter;

  WebView(WebViewAdapter* adapter);

  float GetDeviceScaleFactor() const;

  bool ShouldShowInputPanel() const;
  bool ShouldHideInputPanel() const;
  void SetInputPanelVisibility(bool visible);

  // WebView implementation
  void Init(oxide::WebView::Params* params) final;

  blink::WebScreenInfo GetScreenInfo() const final;
  gfx::Rect GetViewBoundsPix() const final;
  bool IsVisible() const final;
  bool HasFocus() const final;
  bool IsInputPanelVisible() const final;

  oxide::JavaScriptDialog* CreateJavaScriptDialog(
      content::JavaScriptMessageType javascript_message_type,
      bool* did_suppress_message) final;
  oxide::JavaScriptDialog* CreateBeforeUnloadDialog() final;

  void FrameAdded(oxide::WebFrame* frame) final;
  void FrameRemoved(oxide::WebFrame* frame) final;

  bool CanCreateWindows() const final;

  size_t GetScriptMessageHandlerCount() const final;
  const oxide::ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const final;

  void OnURLChanged() final;
  void OnTitleChanged() final;
  void OnIconChanged(const GURL& icon) final;
  void OnCommandsUpdated() final;

  void OnLoadingChanged() final;
  void OnLoadProgressChanged(double progress) final;

  void OnLoadStarted(const GURL& validated_url) final;
  void OnLoadRedirected(const GURL& url,
                        const GURL& original_url) final;
  void OnLoadCommitted(const GURL& url,
                       bool is_error_page) final;
  void OnLoadStopped(const GURL& validated_url) final;
  void OnLoadFailed(const GURL& validated_url,
                    int error_code,
                    const std::string& error_description) final;
  void OnLoadSucceeded(const GURL& validated_url) final;

  void OnNavigationEntryCommitted() final;
  void OnNavigationListPruned(bool from_front, int count) final;
  void OnNavigationEntryChanged(int index) final;

  bool OnAddMessageToConsole(int32 level,
                             const base::string16& message,
                             int32 line_no,
                             const base::string16& source_id) final;

  void OnToggleFullscreenMode(bool enter) final;

  void OnWebPreferencesDestroyed() final;

  void OnRequestGeolocationPermission(
      const GURL& origin,
      const GURL& embedder,
      scoped_ptr<oxide::SimplePermissionRequest> request) final;

  void OnUnhandledKeyboardEvent(
      const content::NativeWebKeyboardEvent& event) final;

  void OnFrameMetadataUpdated(const cc::CompositorFrameMetadata& old) final;

  void OnDownloadRequested(const GURL& url,
			   const std::string& mimeType,
			   const bool shouldPrompt,
			   const base::string16& suggestedFilename,
			   const std::string& cookies,
			   const std::string& referrer) final;

  bool ShouldHandleNavigation(const GURL& url,
                              WindowOpenDisposition disposition,
                              bool user_gesture) final;

  oxide::WebFrame* CreateWebFrame(content::FrameTreeNode* node) final;
  oxide::WebPopupMenu* CreatePopupMenu(content::RenderFrameHost* rfh) final;

  oxide::WebView* CreateNewWebView(const gfx::Rect& initial_pos,
                                   WindowOpenDisposition disposition) final;

  oxide::FilePicker* CreateFilePicker(content::RenderViewHost* rvh) final;

  void OnSwapCompositorFrame() final;
  void OnEvictCurrentFrame() final;

  void OnTextInputStateChanged() final;
  void OnFocusedNodeChanged() final;
  void OnSelectionBoundsChanged() final;
  void OnImeCancelComposition() final;
  void OnSelectionChanged() final;

  void OnUpdateCursor(const content::WebCursor& cursor) final;

  void OnSecurityStatusChanged(const oxide::SecurityStatus& old) final;
  void OnCertificateError(scoped_ptr<oxide::CertificateError> error) final;
  void OnContentBlocked() final;

  void OnPrepareToCloseResponse(bool proceed) final;
  void OnCloseRequested() final;

  WebViewAdapter* adapter_;

  bool has_input_method_state_;

  scoped_ptr<OxideQSecurityStatus> qsecurity_status_;

  UITouchEventFactory touch_event_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebView);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_VIEW_H_
