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

#ifndef _OXIDE_QT_QUICK_API_MESSAGE_HANDLER_P_P_H_
#define _OXIDE_QT_QUICK_API_MESSAGE_HANDLER_P_P_H_

#include <QJSValue>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_message_handler_adapter.h"

class OxideQIncomingMessage;
class OxideQQuickMessageHandler;

class OxideQQuickMessageHandlerPrivate Q_DECL_FINAL :
    public oxide::qt::MessageHandlerAdapter {
  Q_DECLARE_PUBLIC(OxideQQuickMessageHandler)

 public:
  static OxideQQuickMessageHandlerPrivate* Create(
      OxideQQuickMessageHandler* q);

  void removeFromCurrentOwner();

  static OxideQQuickMessageHandlerPrivate* get(
      OxideQQuickMessageHandler* message_handler);

  QJSValue callback;

 private:
  OxideQQuickMessageHandlerPrivate(OxideQQuickMessageHandler* q);
  bool OnReceiveMessage(OxideQIncomingMessage* message,
                        OxideQQuickWebFrame* frame,
                        QString& error) Q_DECL_FINAL;

  OxideQQuickMessageHandler* q_ptr;

  Q_DISABLE_COPY(OxideQQuickMessageHandlerPrivate);
};

#endif // _OXIDE_QT_QUICK_API_MESSAGE_HANDLER_P_P_H_
