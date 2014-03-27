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

#include "oxide_qt_render_widget_host_view.h"
#include "oxide_qt_web_frame.h"
#include "oxide_qt_web_popup_menu.h"

namespace oxide {
namespace qt {

WebView::WebView(WebViewAdapter* adapter) :
    adapter(adapter) {}

size_t WebView::GetScriptMessageHandlerCount() const {
  return adapter->message_handlers().size();
}

oxide::ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(
    size_t index) const {
  return &ScriptMessageHandlerAdapterPrivate::get(
      adapter->message_handlers().at(index))->handler;
}

content::RenderWidgetHostView* WebView::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host) {
  return new RenderWidgetHostView(
      render_widget_host,
      adapter->CreateRenderWidgetHostViewDelegate());
}

gfx::Rect WebView::GetContainerBounds() {
  QRect bounds = adapter->GetContainerBounds();
  return gfx::Rect(bounds.x(),
                   bounds.y(),
                   bounds.width(),
                   bounds.height());
}

oxide::WebPopupMenu* WebView::CreatePopupMenu(content::RenderViewHost* rvh) {
  return new WebPopupMenu(adapter->CreateWebPopupMenuDelegate(), rvh);
}

void WebView::FrameAdded(oxide::WebFrame* frame) {
  adapter->FrameAdded(static_cast<WebFrame *>(frame)->GetAdapter());
}

void WebView::FrameRemoved(oxide::WebFrame* frame) {
  adapter->FrameRemoved(static_cast<WebFrame *>(frame)->GetAdapter());
}

void WebView::OnURLChanged() {
  adapter->URLChanged();
}

void WebView::OnTitleChanged() {
  adapter->TitleChanged();
}

void WebView::OnCommandsUpdated() {
  adapter->CommandsUpdated();
}

void WebView::OnLoadProgressChanged(double progress) {
  adapter->LoadProgressChanged(progress);
}

void WebView::OnLoadStarted(const GURL& validated_url,
                            bool is_error_frame) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStarted);
  adapter->LoadEvent(&event);
}

void WebView::OnLoadStopped(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStopped);
  adapter->LoadEvent(&event);
}

void WebView::OnLoadFailed(const GURL& validated_url,
                           int error_code,
                           const std::string& error_description) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeFailed,
      OxideQLoadEventPrivate::ChromeErrorCodeToOxideErrorCode(error_code),
      QString::fromStdString(error_description));
  adapter->LoadEvent(&event);
}

void WebView::OnLoadSucceeded(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeSucceeded);
  adapter->LoadEvent(&event);
}

void WebView::OnNavigationEntryCommitted() {
  adapter->NavigationEntryCommitted();
}

void WebView::OnNavigationListPruned(bool from_front, int count) {
  adapter->NavigationListPruned(from_front, count);
}

void WebView::OnNavigationEntryChanged(int index) {
  adapter->NavigationEntryChanged(index);
}

void WebView::OnWebPreferencesChanged() {
  adapter->WebPreferencesChanged();
}

oxide::WebFrame* WebView::CreateWebFrame(content::FrameTreeNode* node) {
  return new WebFrame(adapter->CreateWebFrame(), node, this);
}

// static
WebView* WebView::Create(WebViewAdapter* adapter) {
  return new WebView(adapter);
}

} // namespace qt
} // namespace oxide
