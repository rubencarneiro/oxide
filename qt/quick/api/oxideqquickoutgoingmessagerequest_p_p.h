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

#ifndef _OXIDE_QT_QUICK_API_OUTGOING_MESSAGE_REQUEST_P_P_H_
#define _OXIDE_QT_QUICK_API_OUTGOING_MESSAGE_REQUEST_P_P_H_

#include <QJSValue>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_outgoing_message_request_adapter.h"

class OxideQQuickOutgoingMessageRequest;

class OxideQQuickOutgoingMessageRequestPrivate Q_DECL_FINAL :
    public oxide::qt::OutgoingMessageRequestAdapter {
  Q_DECLARE_PUBLIC(OxideQQuickOutgoingMessageRequest)

 public:
  OxideQQuickOutgoingMessageRequestPrivate(
      OxideQQuickOutgoingMessageRequest* q);

  static OxideQQuickOutgoingMessageRequestPrivate* get(
      OxideQQuickOutgoingMessageRequest* request);

  QJSValue reply_callback;
  QJSValue error_callback;

 private:
  void OnReceiveReply(const QVariant& args) Q_DECL_FINAL;
  void OnReceiveError(int error, const QString& msg) Q_DECL_FINAL;
};

#endif // _OXIDE_QT_QUICK_API_OUTGOING_MESSAGE_REQUEST_P_P_H_
