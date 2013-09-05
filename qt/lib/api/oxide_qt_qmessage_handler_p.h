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

#ifndef _OXIDE_QT_LIB_API_QMESSAGE_HANDLER_P_H_
#define _OXIDE_QT_LIB_API_QMESSAGE_HANDLER_P_H_

#include <QJSValue>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"

#include "shared/browser/oxide_message_handler.h"

#include "oxide_qquick_message_handler_p.h"

class OxideQIncomingMessage;
class OxideQMessageHandlerBase;
class OxideQWebFrameBase;

namespace oxide {

class IncomingMessage;

namespace qt {

class QMessageHandlerBasePrivate {
  Q_DECLARE_PUBLIC(OxideQMessageHandlerBase)

 public:
  virtual ~QMessageHandlerBasePrivate();

  const oxide::MessageHandler* handler() const {
    return &handler_;
  }
  oxide::MessageHandler* handler() {
    return &handler_;
  }

  static QMessageHandlerBasePrivate* get(
      OxideQMessageHandlerBase* message_handler);

  void removeFromCurrentOwner();

 protected:
  QMessageHandlerBasePrivate(OxideQMessageHandlerBase* q);

  OxideQMessageHandlerBase* q_ptr;

 private:
  void ReceiveMessageCallback(oxide::IncomingMessage* message);
  virtual void OnReceiveMessage(OxideQIncomingMessage* message,
                                OxideQWebFrameBase* frame) = 0;

  oxide::MessageHandler handler_;
  base::WeakPtrFactory<QMessageHandlerBasePrivate> weak_factory_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(QMessageHandlerBasePrivate);
};

class QQuickMessageHandlerPrivate FINAL : public QMessageHandlerBasePrivate {
  Q_DECLARE_PUBLIC(OxideQQuickMessageHandler)

 public:
  static QQuickMessageHandlerPrivate* Create(OxideQQuickMessageHandler* q);

  QJSValue callback_;

 private:
  QQuickMessageHandlerPrivate(OxideQQuickMessageHandler* q);
  void OnReceiveMessage(OxideQIncomingMessage* message,
                        OxideQWebFrameBase* frame) FINAL;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_API_QMESSAGE_HANDLER_P_H_
