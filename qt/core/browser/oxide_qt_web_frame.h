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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_FRAME_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_FRAME_H_

#include <QList>
#include <QtGlobal>

#include "base/macros.h"
#include "base/memory/scoped_ptr.h"

#include "qt/core/glue/oxide_qt_web_frame_proxy.h"
#include "shared/browser/oxide_web_frame.h"

namespace oxide {
namespace qt {

class ScriptMessageRequest;
class WebFrameProxyClient;
class WebView;

class WebFrame : public oxide::WebFrame,
                 public WebFrameProxy {
 public:
  WebFrame(content::RenderFrameHost* render_frame_host,
           WebView* view);

  static WebFrame* FromProxyHandle(WebFrameProxyHandle* handle);

 private:
  ~WebFrame() override;

  // oxide::ScriptMessageTarget implementation
  size_t GetScriptMessageHandlerCount() const override;
  const oxide::ScriptMessageHandler* GetScriptMessageHandlerAt(
      size_t index) const override;

  // oxide::WebFrame implementation
  void DidCommitNewURL() override;
  void Delete() override;
  void OnChildAdded(oxide::WebFrame* child) override;
  void OnChildRemoved(oxide::WebFrame* child) override;

  // WebFrameProxy implementation
  void setClient(WebFrameProxyClient* client) override;
  QUrl url() const override;
  WebFrameProxyHandle* parent() const override;
  int childFrameCount() const override;
  WebFrameProxyHandle* childFrameAt(int index) const override;
  bool sendMessage(const QUrl& context,
                   const QString& msg_id,
                   const QVariant& args,
                   ScriptMessageRequestProxyHandle* req) override;
  void sendMessageNoReply(const QUrl& context,
                          const QString& msg_id,
                          const QVariant& args) override;
  QList<ScriptMessageHandlerProxyHandle*>& messageHandlers() override;

  WebFrameProxyClient* client_;

  QList<ScriptMessageHandlerProxyHandle*> message_handlers_;

  DISALLOW_COPY_AND_ASSIGN(WebFrame);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_FRAME_H_
