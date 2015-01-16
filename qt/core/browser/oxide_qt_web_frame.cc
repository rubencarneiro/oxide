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

#include <QObject>

#include "base/logging.h"

#include "qt/core/browser/oxide_qt_script_message_handler.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter.h"
#include "shared/browser/oxide_script_message_request_impl_browser.h"

#include "oxide_qt_script_message_request.h"
#include "oxide_qt_web_view.h"

namespace oxide {
namespace qt {

WebFrame::~WebFrame() {}

const oxide::ScriptMessageHandler* WebFrame::GetScriptMessageHandlerAt(
    size_t index) const {
  return ScriptMessageHandler::FromAdapter(
      adapter_->message_handlers_.at(index))->handler();
}

size_t WebFrame::GetScriptMessageHandlerCount() const {
  return adapter_->message_handlers_.size();
}

void WebFrame::DidCommitNewURL() {
  adapter_->URLCommitted();
}

void WebFrame::OnChildAdded(oxide::WebFrame* child) {
  QObject* child_api = static_cast<WebFrame *>(child)->api_handle();
  child_api->setParent(api_handle());

  WebView* v = static_cast<WebView*>(view());
  if (v) {
    v->FrameAdded(child);
  }
}

void WebFrame::OnChildRemoved(oxide::WebFrame* child) {
  QObject* child_api = static_cast<WebFrame *>(child)->api_handle();
  child_api->setParent(NULL);

  WebView* v = static_cast<WebView*>(view());
  if (v) {
    v->FrameRemoved(child);
  }
}

WebFrame::WebFrame(WebFrameAdapter* adapter,
                   content::RenderFrameHost* render_frame_host,
                   oxide::WebView* view)
    : oxide::WebFrame(render_frame_host, view),
      api_handle_(adapterToQObject(adapter)),
      adapter_(adapter) {
  adapter->frame_ = this;
}

bool WebFrame::SendMessage(const GURL& context,
                           const std::string& msg_id,
                           const std::string& args,
                           ScriptMessageRequest* req) {
  scoped_ptr<oxide::ScriptMessageRequestImplBrowser> smr =
      oxide::WebFrame::SendMessage(context, msg_id, args);
  if (!smr) {
    return false;
  }

  req->SetRequest(smr.Pass());
  return true;
}

} // namespace qt
} // namespace oxide
