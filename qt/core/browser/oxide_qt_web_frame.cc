// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include <map>
#include <utility>

#include <QObject>
#include <QString>
#include <QVariant>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "url/gurl.h"

#include "shared/browser/oxide_script_message_request_impl_browser.h"
#include "shared/browser/oxide_web_frame.h"

#include "oxide_qt_script_message_handler.h"
#include "oxide_qt_script_message_request.h"
#include "oxide_qt_variant_value_converter.h"

namespace oxide {
namespace qt {

namespace {

base::LazyInstance<std::map<uintptr_t, WebFrame*>> g_frame_map =
    LAZY_INSTANCE_INITIALIZER;

}

WebFrame::~WebFrame() {
  auto it = g_frame_map.Get().find(reinterpret_cast<uintptr_t>(frame_));
  CHECK(it != g_frame_map.Get().end());
  g_frame_map.Get().erase(it);
}

const oxide::ScriptMessageHandler* WebFrame::GetScriptMessageHandlerAt(
    size_t index) const {
  return ScriptMessageHandler::FromProxyHandle(
      message_handlers_.at(index))->handler();
}

size_t WebFrame::GetScriptMessageHandlerCount() const {
  return message_handlers_.size();
}

void WebFrame::setClient(WebFrameProxyClient* client) {
  DCHECK(!client_ && client);
  client_ = client;
}

QUrl WebFrame::url() const {
  return QUrl(QString::fromStdString(frame_->GetURL().spec()));
}

WebFrameProxyHandle* WebFrame::parent() const {
  WebFrame* parent = WebFrame::FromSharedWebFrame(frame_->parent());
  if (!parent) {
    return nullptr;
  }

  return parent->handle();
}

QList<WebFrameProxyHandle*> WebFrame::childFrames() const {
  QList<WebFrameProxyHandle*> rv;

  const std::vector<oxide::WebFrame*>& children = frame_->GetChildFrames();
  for (auto c : children) {
    WebFrame* child = WebFrame::FromSharedWebFrame(c);
    if (!child) {
      // This can happen in WebView::FromDeleted, when we call
      // WebFrameProxyClient::ChildFramesChanged. The application might refresh
      // the list of children here, when oxide::WebFrame still exists
      continue;
    }
    rv.push_back(child->handle());
  }

  return rv;
}

bool WebFrame::sendMessage(const QUrl& context,
                           const QString& msg_id,
                           const QVariant& payload,
                           ScriptMessageRequestProxyHandle* req) {
  scoped_ptr<base::Value> payload_value(
      VariantValueConverter::FromVariantValue(payload));

  scoped_ptr<oxide::ScriptMessageRequestImplBrowser> smr =
      frame_->SendMessage(GURL(context.toString().toStdString()),
                          msg_id.toStdString(),
                          std::move(payload_value));
  if (!smr) {
    return false;
  }

  ScriptMessageRequest::FromProxyHandle(req)->SetRequest(std::move(smr));

  return true;
}

void WebFrame::sendMessageNoReply(const QUrl& context,
                                  const QString& msg_id,
                                  const QVariant& payload) {
  frame_->SendMessageNoReply(
      GURL(context.toString().toStdString()),
      msg_id.toStdString(),
      VariantValueConverter::FromVariantValue(payload));
}

QList<ScriptMessageHandlerProxyHandle*>& WebFrame::messageHandlers() {
  return message_handlers_;
}

WebFrame::WebFrame(oxide::WebFrame* frame)
    : frame_(frame),
      client_(nullptr) {
  CHECK(g_frame_map.Get().find(reinterpret_cast<uintptr_t>(frame)) ==
        g_frame_map.Get().end());
  auto rv = g_frame_map.Get().insert(
      std::make_pair(reinterpret_cast<uintptr_t>(frame),
                     this));
  DCHECK(rv.second);
}

// static
WebFrame* WebFrame::FromProxyHandle(WebFrameProxyHandle* handle) {
  return static_cast<WebFrame*>(handle->proxy_.data());
}

// static
WebFrame* WebFrame::FromSharedWebFrame(oxide::WebFrame* frame) {
  DCHECK(frame);
  auto it = g_frame_map.Get().find(reinterpret_cast<uintptr_t>(frame));
  return it == g_frame_map.Get().end() ? nullptr : it->second;
}

// static
WebFrame* WebFrame::FromRenderFrameHost(content::RenderFrameHost* host) {
  return FromSharedWebFrame(oxide::WebFrame::FromRenderFrameHost(host));
}

} // namespace qt
} // namespace oxide
