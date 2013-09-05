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

#ifndef _OXIDE_QT_LIB_API_PUBLIC_QQUICK_MESSAGE_HANDLER_H_
#define _OXIDE_QT_LIB_API_PUBLIC_QQUICK_MESSAGE_HANDLER_H_

#include <QJSValue>
#include <QtQml>

#include "oxide_q_message_handler_base.h"

namespace oxide {
namespace qt {
class QQuickMessageHandlerPrivate;
}
}

class Q_DECL_EXPORT OxideQQuickMessageHandler : public OxideQMessageHandlerBase {
  Q_OBJECT
  Q_PROPERTY(QJSValue callback READ callback WRITE setCallback NOTIFY callbackChanged)

  Q_DECLARE_PRIVATE(oxide::qt::QQuickMessageHandler)

 public:
  OxideQQuickMessageHandler(QObject* parent = NULL);
  virtual ~OxideQQuickMessageHandler();

  QJSValue callback() const;
  void setCallback(const QJSValue& callback);

 Q_SIGNALS:
  void callbackChanged();
};

QML_DECLARE_TYPE(OxideQQuickMessageHandler)

#endif // _OXIDE_QT_LIB_API_PUBLIC_QQUICK_MESSAGE_HANDLER_H_
