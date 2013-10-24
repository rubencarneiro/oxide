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

#ifndef _OXIDE_QT_QUICK_API_WEB_FRAME_P_P_H_
#define _OXIDE_QT_QUICK_API_WEB_FRAME_P_P_H_

#include <QList>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"

class OxideQQuickMessageHandler;
class OxideQQuickOutgoingMessageRequest;
class OxideQQuickWebFrame;

QT_BEGIN_NAMESPACE
template <typename T> class QQmlListProperty;
QT_END_NAMESPACE

namespace oxide {
namespace qt {
class WebFrame;
}
}

class OxideQQuickWebFramePrivate FINAL {
 public:
  static OxideQQuickWebFramePrivate* Create(oxide::qt::WebFrame* owner);
  ~OxideQQuickWebFramePrivate();

  oxide::qt::WebFrame* owner() const { return owner_; }
  QList<OxideQQuickMessageHandler *>& message_handlers() {
    return message_handlers_;
  }
  QList<OxideQQuickOutgoingMessageRequest *>& outgoing_message_requests() {
    return outgoing_message_requests_;
  }
  QList<OxideQQuickWebFrame *>& children() {
    return children_;
  }

  static OxideQQuickWebFramePrivate* get(OxideQQuickWebFrame* web_frame);

  static int childFrame_count(QQmlListProperty<OxideQQuickWebFrame>* prop);
  static OxideQQuickWebFrame* childFrame_at(
      QQmlListProperty<OxideQQuickWebFrame>* prop, int index);

  void addOutgoingMessageRequest(OxideQQuickOutgoingMessageRequest* request);
  void removeOutgoingMessageRequest(OxideQQuickOutgoingMessageRequest* request);

  static void messageHandler_append(
      QQmlListProperty<OxideQQuickMessageHandler>* prop,
      OxideQQuickMessageHandler* value);
  static int messageHandler_count(
      QQmlListProperty<OxideQQuickMessageHandler>* prop);
  static OxideQQuickMessageHandler* messageHandler_at(
      QQmlListProperty<OxideQQuickMessageHandler>* prop,
      int index);
  static void messageHandler_clear(
      QQmlListProperty<OxideQQuickMessageHandler>* prop);

 private:
  OxideQQuickWebFramePrivate(oxide::qt::WebFrame* owner);

  oxide::qt::WebFrame* owner_;
  QList<OxideQQuickMessageHandler *> message_handlers_;
  QList<OxideQQuickOutgoingMessageRequest *> outgoing_message_requests_;
  // We keep this separate to QObject becase we want a way to track child frames quickly
  QList<OxideQQuickWebFrame *> children_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(OxideQQuickWebFramePrivate);
};

#endif // _OXIDE_QT_QUICK_API_WEB_FRAME_P_P_H_
