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

#include "oxide_qt_web_frame.h"

#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <QVariant>

#include "base/logging.h"
#include "url/gurl.h"

#include "qt/core/glue/oxide_qt_web_frame_proxy_client.h"
#include "shared/browser/oxide_script_message_request_impl_browser.h"

#include "oxide_qt_script_message_handler.h"
#include "oxide_qt_script_message_request.h"
#include "oxide_qt_web_view.h"

namespace oxide {
namespace qt {

WebFrame::~WebFrame() {}

const oxide::ScriptMessageHandler* WebFrame::GetScriptMessageHandlerAt(
    size_t index) const {
  return ScriptMessageHandler::FromProxyHandle(
      message_handlers_.at(index))->handler();
}

size_t WebFrame::GetScriptMessageHandlerCount() const {
  return message_handlers_.size();
}

void WebFrame::DidCommitNewURL() {
  client_->URLCommitted();
}

void WebFrame::Delete() {
  client_->DestroyFrame();
  // |this| has been destroyed
}

void WebFrame::OnChildAdded(oxide::WebFrame* child) {
  client_->ChildFramesChanged();

  WebView* v = WebView::FromView(view());
  if (v) {
    v->FrameAdded(child);
  }
}

void WebFrame::OnChildRemoved(oxide::WebFrame* child) {
  client_->ChildFramesChanged();

  if (!view()) {
    // Can be null when the view is being deleted
    return;
  }

  WebView* v = WebView::FromView(view());
  if (v) {
    v->FrameRemoved(child);
  }
}

void WebFrame::setClient(WebFrameProxyClient* client) {
  DCHECK(!client_ && client);
  client_ = client;
}

QUrl WebFrame::url() const {
  return QUrl(QString::fromStdString(GetURL().spec()));
}

WebFrameProxyHandle* WebFrame::parent() const {
  WebFrame* parent = static_cast<WebFrame*>(oxide::WebFrame::parent());
  if (!parent) {
    return nullptr;
  }

  return parent->handle();
}

int WebFrame::childFrameCount() const {
  return static_cast<int>(
      std::min(GetChildCount(),
               static_cast<size_t>(std::numeric_limits<int>::max())));
}

WebFrameProxyHandle* WebFrame::childFrameAt(int index) const {
  DCHECK_GE(index, 0);
  DCHECK_LT(static_cast<size_t>(index), GetChildCount());
  return static_cast<WebFrame*>(GetChildAt(index))->handle();
}

bool WebFrame::sendMessage(const QUrl& context,
                           const QString& msg_id,
                           const QVariant& args,
                           ScriptMessageRequestProxyHandle* req) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  scoped_ptr<oxide::ScriptMessageRequestImplBrowser> smr =
      SendMessage(GURL(context.toString().toStdString()),
                  msg_id.toStdString(),
                  QString(jsondoc.toJson()).toStdString());
  if (!smr) {
    return false;
  }

  ScriptMessageRequest::FromProxyHandle(req)->SetRequest(smr.Pass());

  return true;
}

void WebFrame::sendMessageNoReply(const QUrl& context,
                                  const QString& msg_id,
                                  const QVariant& args) {
  QJsonDocument jsondoc(QJsonDocument::fromVariant(args));

  SendMessageNoReply(
      GURL(context.toString().toStdString()),
      msg_id.toStdString(),
      QString(jsondoc.toJson()).toStdString());
}

QList<ScriptMessageHandlerProxyHandle*>& WebFrame::messageHandlers() {
  return message_handlers_;
}

WebFrame::WebFrame(content::RenderFrameHost* render_frame_host,
                   oxide::WebView* view)
    : oxide::WebFrame(render_frame_host, view),
      client_(nullptr) {}

// static
WebFrame* WebFrame::FromProxyHandle(WebFrameProxyHandle* handle) {
  return static_cast<WebFrame*>(handle->proxy_.data());
}

} // namespace qt
} // namespace oxide
