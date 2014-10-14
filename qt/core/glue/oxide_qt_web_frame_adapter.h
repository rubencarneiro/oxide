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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_FRAME_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_WEB_FRAME_ADAPTER_H_

#include <QList>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_adapter_base.h"

QT_BEGIN_NAMESPACE
class QString;
class QVariant;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ScriptMessageHandlerAdapter;
class ScriptMessageRequestAdapter;
class WebFrame;

class Q_DECL_EXPORT WebFrameAdapter : public AdapterBase {
 public:
  ~WebFrameAdapter();

  static WebFrameAdapter* FromWebFrame(WebFrame* frame);

  QUrl url() const;

  bool sendMessage(const QUrl& context,
                   const QString& msg_id,
                   const QVariant& args,
                   ScriptMessageRequestAdapter* req);
  void sendMessageNoReply(const QUrl& context,
                          const QString& msg_id,
                          const QVariant& args);

  QList<ScriptMessageHandlerAdapter *>& messageHandlers();

 protected:
  WebFrameAdapter(QObject* q);

 private:
  friend class WebFrame;

  virtual void URLChanged() = 0;

  WebFrame* frame_;

  QList<ScriptMessageHandlerAdapter *> message_handlers_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_FRAME_ADAPTER_H_
