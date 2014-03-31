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

#include "oxide_qt_web_view.h"

#include <QString>
#include <QUrl>

#include "base/strings/utf_string_conversions.h"
#include "url/gurl.h"
#include "content/public/browser/navigation_controller.h"
#include "content/browser/web_contents/web_contents_impl.h"

#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqloadevent_p.h"
#include "qt/core/api/oxideqnewviewrequest.h"
#include "qt/core/api/oxideqnewviewrequest_p.h"
#include "qt/core/glue/oxide_qt_script_message_handler_adapter_p.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter.h"
#include "qt/core/glue/oxide_qt_web_view_adapter.h"

#include "oxide_qt_javascript_dialog.h"
#include "oxide_qt_web_frame.h"
#include "oxide_qt_web_popup_menu.h"

namespace oxide {
namespace qt {

WebView::WebView(WebViewAdapter* adapter) :
    adapter_(adapter) {}

bool WebView::Init(const oxide::WebView::Params& params) {
  if (!oxide::WebView::Init(params)) {
    return false;
  }

  if (adapter_->IsVisible()) {
    Shown();
  } else {
    Hidden();
  }

  adapter_->Initialized();
  return true;
}

size_t WebView::GetScriptMessageHandlerCount() const {
  return adapter_->message_handlers().size();
}

oxide::ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(
    size_t index) const {
  return &ScriptMessageHandlerAdapterPrivate::get(
      adapter_->message_handlers().at(index))->handler;
}

gfx::Rect WebView::GetContainerBounds() {
  QRect bounds = adapter_->GetContainerBounds();
  return gfx::Rect(bounds.x(),
                   bounds.y(),
                   bounds.width(),
                   bounds.height());
}

oxide::WebPopupMenu* WebView::CreatePopupMenu(content::RenderViewHost* rvh) {
  return new WebPopupMenu(adapter_->CreateWebPopupMenuDelegate(), rvh);
}

oxide::JavaScriptDialog* WebView::CreateJavaScriptDialog(
    content::JavaScriptMessageType javascript_message_type,
    bool* did_suppress_message) {
  JavaScriptDialogDelegate::Type type;
  switch (javascript_message_type) {
  case content::JAVASCRIPT_MESSAGE_TYPE_ALERT:
    type = JavaScriptDialogDelegate::TypeAlert;
    break;
  case content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
    type = JavaScriptDialogDelegate::TypeConfirm;
    break;
  case content::JAVASCRIPT_MESSAGE_TYPE_PROMPT:
    type = JavaScriptDialogDelegate::TypePrompt;
    break;
  default:
    Q_UNREACHABLE();
  }
  JavaScriptDialogDelegate* delegate = adapter_->CreateJavaScriptDialogDelegate(type);
  return new JavaScriptDialog(delegate, did_suppress_message);
}

oxide::JavaScriptDialog* WebView::CreateBeforeUnloadDialog() {
  JavaScriptDialogDelegate* delegate = adapter_->CreateBeforeUnloadDialogDelegate();
  bool did_suppress_message = false;
  return new JavaScriptDialog(delegate, &did_suppress_message);
}

void WebView::FrameAdded(oxide::WebFrame* frame) {
  adapter_->FrameAdded(static_cast<WebFrame *>(frame)->adapter());
}

void WebView::FrameRemoved(oxide::WebFrame* frame) {
  adapter_->FrameRemoved(static_cast<WebFrame *>(frame)->adapter());
}

bool WebView::CanCreateWindows() const {
  return adapter_->CanCreateWindows();
}

void WebView::OnURLChanged() {
  adapter_->URLChanged();
}

void WebView::OnTitleChanged() {
  adapter_->TitleChanged();
}

void WebView::OnCommandsUpdated() {
  adapter_->CommandsUpdated();
}

void WebView::OnLoadProgressChanged(double progress) {
  adapter_->LoadProgressChanged(progress);
}

void WebView::OnLoadStarted(const GURL& validated_url,
                            bool is_error_frame) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStarted);
  adapter_->LoadEvent(&event);
}

void WebView::OnLoadStopped(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStopped);
  adapter_->LoadEvent(&event);
}

void WebView::OnLoadFailed(const GURL& validated_url,
                           int error_code,
                           const std::string& error_description) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeFailed,
      OxideQLoadEventPrivate::ChromeErrorCodeToOxideErrorCode(error_code),
      QString::fromStdString(error_description));
  adapter_->LoadEvent(&event);
}

void WebView::OnLoadSucceeded(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeSucceeded);
  adapter_->LoadEvent(&event);
}

void WebView::OnNavigationEntryCommitted() {
  adapter_->NavigationEntryCommitted();
}

void WebView::OnNavigationListPruned(bool from_front, int count) {
  adapter_->NavigationListPruned(from_front, count);
}

void WebView::OnNavigationEntryChanged(int index) {
  adapter_->NavigationEntryChanged(index);
}

void WebView::OnWebPreferencesChanged() {
  adapter_->WebPreferencesChanged();
}

oxide::WebFrame* WebView::CreateWebFrame(content::FrameTreeNode* node) {
  return new WebFrame(adapter_->CreateWebFrame(), node, this);
}

oxide::WebView* WebView::CreateNewWebView(const GURL& target_url,
                                          const gfx::Rect& initial_pos,
                                          WindowOpenDisposition disposition,
                                          bool user_gesture) {
  OxideQNewViewRequest::Disposition d = OxideQNewViewRequest::DispositionNewWindow;

  switch (disposition) {
    case CURRENT_TAB:
      d = OxideQNewViewRequest::DispositionCurrentTab;
      break;
    case NEW_FOREGROUND_TAB:
      d = OxideQNewViewRequest::DispositionNewForegroundTab;
      break;
    case NEW_BACKGROUND_TAB:
      d = OxideQNewViewRequest::DispositionNewBackgroundTab;
      break;
    case NEW_POPUP:
      d = OxideQNewViewRequest::DispositionNewPopup;
      break;
    case NEW_WINDOW:
      d = OxideQNewViewRequest::DispositionNewWindow;
      break;
    default:
      NOTREACHED();
  }

  OxideQNewViewRequest request(QUrl(QString::fromStdString(target_url.spec())),
                               QRect(initial_pos.x(),
                                     initial_pos.y(),
                                     initial_pos.width(),
                                     initial_pos.height()),
                               d, user_gesture);

  adapter_->NewViewRequested(&request);

  return OxideQNewViewRequestPrivate::get(&request)->view.get();
}

content::WebContents* WebViewAdapterPrivate::OpenURLFromTab(content::WebContents* source,
                                                            const content::OpenURLParams& p) {
  if (a->NavigationRequested(QString::fromStdString(p.url.spec()))) {
    content::NavigationController& controller = web_contents_->GetController();

    content::NavigationController::LoadURLParams params(p.url);

    params.referrer = p.referrer;
    params.redirect_chain = p.redirect_chain;
    params.should_replace_current_entry = true;
    params.is_renderer_initiated = p.is_renderer_initiated;
    params.extra_headers = p.extra_headers;
    params.transition_type = p.transition;
    params.frame_tree_node_id = p.frame_tree_node_id;
    controller.LoadURLWithParams(params);

    return web_contents_.get();
  }

  return NULL;
}

// static
WebView* WebView::Create(WebViewAdapter* adapter) {
  return new WebView(adapter);
}

} // namespace qt
} // namespace oxide
