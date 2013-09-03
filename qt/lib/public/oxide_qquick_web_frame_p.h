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

#ifndef _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_FRAME_P_H_
#define _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_FRAME_P_H_

#include <QQmlListProperty>
#include <QtGlobal>
#include <QtQml>

#include "qt/lib/public/oxide_qt_qweb_frame.h"

class OxideQQuickMessageHandler;
class OxideQQuickOutgoingMessageRequest;
class OxideQQuickWebFramePrivate;

namespace oxide {
namespace qt {
class WebFrameQQuick;
}
}

class Q_DECL_EXPORT OxideQQuickWebFrame : public oxide::qt::QWebFrame {
  Q_OBJECT
  Q_PROPERTY(OxideQQuickWebFrame* parentFrame READ parentFrame NOTIFY parentFrameChanged)
  Q_PROPERTY(QQmlListProperty<OxideQQuickWebFrame> childFrames READ childFrames NOTIFY childFrameChanged)
  Q_PROPERTY(QQmlListProperty<OxideQQuickMessageHandler> messageHandlers READ messageHandlers)
  Q_ENUMS(ChildFrameChangedType)

  Q_DECLARE_PRIVATE(OxideQQuickWebFrame)

 public:
  enum ChildFrameChangedType {
    ChildAdded,
    ChildRemoved
  };

  OxideQQuickWebFrame(QObject* parent = NULL);
  virtual ~OxideQQuickWebFrame();

  OxideQQuickWebFrame* parentFrame() const;
  QQmlListProperty<OxideQQuickWebFrame> childFrames();

  QQmlListProperty<OxideQQuickMessageHandler> messageHandlers();

  Q_INVOKABLE OxideQQuickOutgoingMessageRequest*
      sendMessage(const QString& world_id,
                  const QString& msg_id,
                  const QString& args);
  Q_INVOKABLE void sendMessageNoReply(const QString& world_id,
                                      const QString& msg_id,
                                      const QString& args);

 Q_SIGNALS:
  void parentFrameChanged();
  void childFrameChanged(ChildFrameChangedType type,
                         OxideQQuickWebFrame* child_frame);

 protected:
  friend class oxide::qt::WebFrameQQuick;

  OxideQQuickWebFrame(oxide::qt::WebFrameQQuick* owner);
};

QML_DECLARE_TYPE(OxideQQuickWebFrame);

#endif // _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_FRAME_P_H_
