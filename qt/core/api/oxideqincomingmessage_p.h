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

#ifndef _OXIDE_QT_CORE_API_INCOMING_MESSAGE_P_H_
#define _OXIDE_QT_CORE_API_INCOMING_MESSAGE_P_H_

#include <QScopedPointer>
#include <QtGlobal>
#include <QVariant>

namespace oxide {
class IncomingMessage;
}

class OxideQIncomingMessage;

class OxideQIncomingMessagePrivate Q_DECL_FINAL {
 public:
  OxideQIncomingMessagePrivate();

  oxide::IncomingMessage* incoming() const {
    return incoming_.data();
  }

  QVariant args() const { return args_; }

  void Initialize(oxide::IncomingMessage* message);

  static OxideQIncomingMessagePrivate* get(OxideQIncomingMessage* q);

 private:
  QScopedPointer<oxide::IncomingMessage> incoming_;
  QVariant args_;
};

#endif // _OXIDE_QT_CORE_API_INCOMING_MESSAGE_P_H_
