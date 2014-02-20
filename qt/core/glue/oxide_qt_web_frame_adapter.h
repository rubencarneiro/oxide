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
#include <QScopedPointer>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_adapter_base.h"

QT_BEGIN_NAMESPACE
class QString;
class QVariant;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class MessageHandlerAdapter;
class OutgoingMessageRequestAdapter;
class WebFrameAdapterPrivate;

class Q_DECL_EXPORT WebFrameAdapter : public AdapterBase {
 public:
  ~WebFrameAdapter();

  QUrl url() const;

  bool sendMessage(const QString& world_id,
                   const QString& msg_id,
                   const QVariant& args,
                   OutgoingMessageRequestAdapter* req);
  void sendMessageNoReply(const QString& world_id,
                          const QString& msg_id,
                          const QVariant& args);

  QList<MessageHandlerAdapter *>& message_handlers() {
    return message_handlers_;
  }

  virtual void URLChanged() = 0;

 protected:
  WebFrameAdapter(QObject* q);

 private:
  friend class WebFrameAdapterPrivate;

  QList<MessageHandlerAdapter *> message_handlers_;
  QScopedPointer<WebFrameAdapterPrivate> priv;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_FRAME_ADAPTER_H_
