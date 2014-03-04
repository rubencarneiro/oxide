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

#include "oxide_qt_web_view_adapter_p.h"

#include <QString>
#include <QUrl>

#include "url/gurl.h"

#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqloadevent_p.h"
#include "qt/core/browser/oxide_qt_render_widget_host_view.h"
#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/browser/oxide_qt_web_popup_menu.h"
#include "qt/core/glue/oxide_qt_script_message_handler_adapter_p.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter_p.h"
#include "qt/core/glue/oxide_qt_web_view_adapter.h"

namespace oxide {
namespace qt {

WebViewAdapterPrivate::WebViewAdapterPrivate(WebViewAdapter* adapter) :
    a(adapter) {}

void WebViewAdapterPrivate::OnURLChanged() {
  a->URLChanged();
}

void WebViewAdapterPrivate::OnTitleChanged() {
  a->TitleChanged();
}

void WebViewAdapterPrivate::OnIconChanged(const GURL& icon) {
  a->IconChanged(QUrl(QString::fromStdString(icon.spec())));
}

void WebViewAdapterPrivate::OnCommandsUpdated() {
  a->CommandsUpdated();
}

void WebViewAdapterPrivate::OnLoadProgressChanged(double progress) {
  a->LoadProgressChanged(progress);
}

void WebViewAdapterPrivate::OnLoadStarted(const GURL& validated_url,
                                          bool is_error_frame) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStarted);
  a->LoadEvent(&event);
}

void WebViewAdapterPrivate::OnLoadStopped(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStopped);
  a->LoadEvent(&event);
}

void WebViewAdapterPrivate::OnLoadFailed(const GURL& validated_url,
                                         int error_code,
                                         const std::string& error_description) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeFailed,
      OxideQLoadEventPrivate::ChromeErrorCodeToOxideErrorCode(error_code),
      QString::fromStdString(error_description));
  a->LoadEvent(&event);
}

void WebViewAdapterPrivate::OnLoadSucceeded(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeSucceeded);
  a->LoadEvent(&event);
}

void WebViewAdapterPrivate::OnNavigationEntryCommitted() {
  a->NavigationEntryCommitted();
}

void WebViewAdapterPrivate::OnNavigationListPruned(bool from_front, int count) {
  a->NavigationListPruned(from_front, count);
}

void WebViewAdapterPrivate::OnNavigationEntryChanged(int index) {
  a->NavigationEntryChanged(index);
}

void WebViewAdapterPrivate::OnWebPreferencesChanged() {
  a->WebPreferencesChanged();
}

oxide::WebFrame* WebViewAdapterPrivate::CreateWebFrame(
    content::FrameTreeNode* node) {
  return new WebFrame(a->CreateWebFrame(), node, this);
}

// static
WebViewAdapterPrivate* WebViewAdapterPrivate::Create(WebViewAdapter* adapter) {
  return new WebViewAdapterPrivate(adapter);
}

size_t WebViewAdapterPrivate::GetScriptMessageHandlerCount() const {
  return a->message_handlers().size();
}

oxide::ScriptMessageHandler* WebViewAdapterPrivate::GetScriptMessageHandlerAt(
    size_t index) const {
  return &ScriptMessageHandlerAdapterPrivate::get(
      a->message_handlers().at(index))->handler;
}

content::RenderWidgetHostView* WebViewAdapterPrivate::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host) {
  return new RenderWidgetHostView(
      render_widget_host,
      a->CreateRenderWidgetHostViewDelegate());
}

gfx::Rect WebViewAdapterPrivate::GetContainerBounds() {
  QRect bounds = a->GetContainerBounds();
  return gfx::Rect(bounds.x(),
                   bounds.y(),
                   bounds.width(),
                   bounds.height());
}

oxide::WebPopupMenu* WebViewAdapterPrivate::CreatePopupMenu(
    content::RenderViewHost* rvh) {
  return new WebPopupMenu(a->CreateWebPopupMenuDelegate(), rvh);
}

void WebViewAdapterPrivate::FrameAdded(oxide::WebFrame* frame) {
  a->FrameAdded(static_cast<WebFrame *>(frame)->GetAdapter());
}

void WebViewAdapterPrivate::FrameRemoved(oxide::WebFrame* frame) {
  a->FrameRemoved(static_cast<WebFrame *>(frame)->GetAdapter());
}

} // namespace qt
} // namespace oxide
