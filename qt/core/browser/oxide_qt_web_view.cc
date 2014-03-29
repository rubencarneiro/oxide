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

#include "url/gurl.h"

#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqloadevent_p.h"
#include "qt/core/glue/oxide_qt_script_message_handler_adapter_p.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter_p.h"
#include "qt/core/glue/oxide_qt_web_view_adapter.h"

#include "oxide_qt_web_frame.h"
#include "oxide_qt_web_popup_menu.h"

namespace oxide {
namespace qt {

WebView::WebView(WebViewAdapter* adapter) :
    adapter_(adapter) {}

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

void WebView::FrameAdded(oxide::WebFrame* frame) {
  adapter_->FrameAdded(static_cast<WebFrame *>(frame)->GetAdapter());
}

void WebView::FrameRemoved(oxide::WebFrame* frame) {
  adapter_->FrameRemoved(static_cast<WebFrame *>(frame)->GetAdapter());
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

// static
WebView* WebView::Create(WebViewAdapter* adapter) {
  return new WebView(adapter);
}

} // namespace qt
} // namespace oxide
