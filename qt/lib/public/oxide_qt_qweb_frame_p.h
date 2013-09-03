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

#ifndef _OXIDE_QT_LIB_PUBLIC_QWEB_FRAME_P_H_
#define _OXIDE_QT_LIB_PUBLIC_QWEB_FRAME_P_H_

#include <QList>

#include "base/basictypes.h"

namespace oxide {
namespace qt {

class QMessageHandler;
class QOutgoingMessageRequest;
class QWebFrame;
class WebFrame;

class QWebFramePrivate {
 public:
  virtual ~QWebFramePrivate();

  WebFrame* owner() const { return owner_; }
  QList<QMessageHandler *>& message_handlers() {
    return message_handlers_;
  }
  QList<QOutgoingMessageRequest *>& outgoing_message_requests() {
    return outgoing_message_requests_;
  }

  static QWebFramePrivate* get(QWebFrame* web_frame);

  void addOutgoingMessageRequest(QOutgoingMessageRequest* request);
  void removeOutgoingMessageRequest(QOutgoingMessageRequest* request);

 protected:
  QWebFramePrivate(WebFrame* owner);

 private:
  WebFrame* owner_;
  QList<QMessageHandler *> message_handlers_;
  QList<QOutgoingMessageRequest *> outgoing_message_requests_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(QWebFramePrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_PUBLIC_QWEB_FRAME_P_H_
