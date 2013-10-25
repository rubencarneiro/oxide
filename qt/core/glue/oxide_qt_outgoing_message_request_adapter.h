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

#ifndef _OXIDE_QT_CORE_GLUE_OUTGOING_MESSAGE_REQUEST_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_OUTGOING_MESSAGE_REQUEST_ADAPTER_H_

#include <string>

#include <QScopedPointer>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QString;
class QVariant;
QT_END_NAMESPACE

namespace oxide {

class OutgoingMessageRequest;

namespace qt {

class OutgoingMessageRequestAdapterPrivate;

class Q_DECL_EXPORT OutgoingMessageRequestAdapter {
 public:
  virtual ~OutgoingMessageRequestAdapter();

  oxide::OutgoingMessageRequest* GetRequest();

 protected:
  OutgoingMessageRequestAdapter();

 private:
  void ReceiveReplyCallback(const std::string& args);
  void ReceiveErrorCallback(int error, const std::string& msg);

  virtual void OnReceiveReply(const QVariant& args) = 0;
  virtual void OnReceiveError(int error, const QString& msg) = 0;

  QScopedPointer<OutgoingMessageRequestAdapterPrivate> priv_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_OUTGOING_MESSAGE_REQUEST_ADAPTER_H_
