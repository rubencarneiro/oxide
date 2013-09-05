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

#ifndef _OXIDE_QT_LIB_API_QWEB_FRAME_P_H_
#define _OXIDE_QT_LIB_API_QWEB_FRAME_P_H_

#include <QList>
#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"

class OxideQMessageHandlerBase;
class OxideQOutgoingMessageRequestBase;
class OxideQQuickMessageHandler;
class OxideQQuickWebFrame;
class OxideQWebFrameBase;

QT_BEGIN_NAMESPACE
template <typename T> class QQmlListProperty;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class WebFrame;
class WebFrameQQuick;

class QWebFrameBasePrivate {
 public:
  virtual ~QWebFrameBasePrivate();

  WebFrame* owner() const { return owner_; }
  QList<OxideQMessageHandlerBase *>& message_handlers() {
    return message_handlers_;
  }
  QList<OxideQOutgoingMessageRequestBase *>& outgoing_message_requests() {
    return outgoing_message_requests_;
  }

  static QWebFrameBasePrivate* get(OxideQWebFrameBase* web_frame);

  void addOutgoingMessageRequest(OxideQOutgoingMessageRequestBase* request);
  void removeOutgoingMessageRequest(OxideQOutgoingMessageRequestBase* request);

 protected:
  QWebFrameBasePrivate(WebFrame* owner);

 private:
  WebFrame* owner_;
  QList<OxideQMessageHandlerBase *> message_handlers_;
  QList<OxideQOutgoingMessageRequestBase *> outgoing_message_requests_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(QWebFrameBasePrivate);
};

class QQuickWebFramePrivate FINAL : public QWebFrameBasePrivate {
 public:
  static QQuickWebFramePrivate* Create(WebFrameQQuick* owner);

  static int childFrame_count(QQmlListProperty<OxideQQuickWebFrame>* prop);
  static OxideQQuickWebFrame* childFrame_at(
      QQmlListProperty<OxideQQuickWebFrame>* prop, int index);

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
  QQuickWebFramePrivate(WebFrameQQuick* owner);

  DISALLOW_IMPLICIT_CONSTRUCTORS(QQuickWebFramePrivate);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_LIB_API_QWEB_FRAME_P_H_
