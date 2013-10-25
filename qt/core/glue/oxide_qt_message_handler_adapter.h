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

#ifndef _OXIDE_QT_CORE_GLUE_MESSAGE_HANDLER_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_MESSAGE_HANDLER_ADAPTER_H_

#include <string>

#include <QList>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>

class OxideQIncomingMessage;
class OxideQQuickWebFrame;

namespace oxide {
namespace qt {

class MessageHandlerAdapterPrivate;

class Q_DECL_EXPORT MessageHandlerAdapter {
 public:
  virtual ~MessageHandlerAdapter();

  QString msgId() const;
  void setMsgId(const QString& id);

  QList<QString> worldIds() const;
  void setWorldIds(const QList<QString>& ids);

  void attachHandler();
  void detachHandler();

 protected:
  MessageHandlerAdapter();

 private:
  friend class MessageHandlerAdapterPrivate;

  virtual bool OnReceiveMessage(OxideQIncomingMessage* message,
                                OxideQQuickWebFrame* frame,
                                QString& error) = 0;

  QScopedPointer<MessageHandlerAdapterPrivate> priv_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_MESSAGE_HANDLER_ADAPTER_H_
