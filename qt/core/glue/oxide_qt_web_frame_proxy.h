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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_FRAME_PROXY_H_
#define _OXIDE_QT_CORE_GLUE_WEB_FRAME_PROXY_H_

#include <QList>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_proxy_handle.h"

QT_BEGIN_NAMESPACE
class QString;
class QVariant;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ScriptMessageHandlerProxy;
class ScriptMessageRequestProxy;
class WebFrame;
class WebFrameProxy;
class WebFrameProxyClient;

OXIDE_Q_DECL_PROXY_HANDLE(ScriptMessageHandlerProxy);
OXIDE_Q_DECL_PROXY_HANDLE(ScriptMessageRequestProxy);
OXIDE_Q_DECL_PROXY_HANDLE(WebFrameProxy);

class Q_DECL_EXPORT WebFrameProxy {
  OXIDE_Q_DECL_PROXY_FOR(WebFrame);
 public:
  static WebFrameProxy* create(WebFrameProxyClient* client);
  virtual ~WebFrameProxy();

  virtual QUrl url() const = 0;

  virtual WebFrameProxyHandle* parent() const = 0;
  virtual int childFrameCount() const = 0;
  virtual WebFrameProxyHandle* childFrameAt(int index) const = 0;

  virtual bool sendMessage(const QUrl& context,
                           const QString& msg_id,
                           const QVariant& args,
                           ScriptMessageRequestProxyHandle* req) = 0;
  virtual void sendMessageNoReply(const QUrl& context,
                                  const QString& msg_id,
                                  const QVariant& args) = 0;

  virtual QList<ScriptMessageHandlerProxyHandle*>& messageHandlers() = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_FRAME_PROXY_H_
