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

#include "base/strings/utf_string_conversions.h"
#include "url/gurl.h"

#include "qt/core/browser/oxide_qt_render_widget_host_view.h"
#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/browser/oxide_qt_web_popup_menu.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter.h"
#include "qt/core/glue/oxide_qt_web_view_adapter.h"
#include "qt/core/glue/private/oxide_qt_javascript_dialog_closed_callback_p.h"

#include "oxide_qt_message_handler_adapter_p.h"
#include "oxide_qt_web_frame_adapter_p.h"

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

void WebViewAdapterPrivate::OnCommandsUpdated() {
  a->CommandsUpdated();
}

void WebViewAdapterPrivate::OnRootFrameChanged() {
  a->RootFrameChanged();
}

void WebViewAdapterPrivate::OnLoadStarted(const GURL& validated_url,
                                          bool is_error_frame) {
  a->LoadStarted(QUrl(QString::fromStdString(validated_url.spec())));
}

void WebViewAdapterPrivate::OnLoadStopped(const GURL& validated_url) {
  a->LoadStopped(QUrl(QString::fromStdString(validated_url.spec())));
}

void WebViewAdapterPrivate::OnLoadFailed(const GURL& validated_url,
                                         int error_code,
                                         const std::string& error_description) {
  a->LoadFailed(QUrl(QString::fromStdString(validated_url.spec())),
                error_code, QString::fromStdString(error_description));
}

void WebViewAdapterPrivate::OnLoadSucceeded(const GURL& validated_url) {
  a->LoadSucceeded(QUrl(QString::fromStdString(validated_url.spec())));
}

oxide::WebFrame* WebViewAdapterPrivate::CreateWebFrame() {
  return new WebFrame(a->CreateWebFrame());
}

// static
WebViewAdapterPrivate* WebViewAdapterPrivate::Create(WebViewAdapter* adapter) {
  return new WebViewAdapterPrivate(adapter);
}

size_t WebViewAdapterPrivate::GetMessageHandlerCount() const {
  return a->message_handlers().size();
}

oxide::MessageHandler* WebViewAdapterPrivate::GetMessageHandlerAt(
    size_t index) const {
  return &MessageHandlerAdapterPrivate::get(
      a->message_handlers().at(index))->handler();
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

void WebViewAdapterPrivate::RunJavaScriptDialog(
    const GURL& origin_url,
    const std::string& accept_lang,
    content::JavaScriptMessageType javascript_message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const content::JavaScriptDialogManager::DialogClosedCallback& callback,
    bool* did_suppress_message) {
  if (javascript_message_type == content::JAVASCRIPT_MESSAGE_TYPE_ALERT) {
    a->RunJavaScriptAlert(
        QUrl(QString::fromStdString(origin_url.spec())),
        QString::fromStdString(accept_lang),
        QString::fromStdString(base::UTF16ToUTF8(message_text)),
        JavaScriptDialogClosedCallbackPrivate::CreateCallbackWrapper(callback),
        did_suppress_message);
  } else {
    // TODO: handle other types of message (CONFIRM and PROMPT)
    callback.Run(false, default_prompt_text);
  }
}

} // namespace qt
} // namespace oxide
