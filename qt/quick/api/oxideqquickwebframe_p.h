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

#ifndef _OXIDE_QT_QUICK_API_WEB_FRAME_P_H_
#define _OXIDE_QT_QUICK_API_WEB_FRAME_P_H_

#include <QObject>
#include <QQmlListProperty>
#include <QScopedPointer>
#include <QtGlobal>
#include <QtQml>
#include <QUrl>
#include <QVariant>

class OxideQQuickMessageHandler;
class OxideQQuickOutgoingMessageRequest;
class OxideQQuickWebFramePrivate;
class OxideQQuickWebViewPrivate;

class OxideQQuickWebFrame : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)
  Q_PROPERTY(OxideQQuickWebFrame* parentFrame READ parentFrame)
  Q_PROPERTY(QQmlListProperty<OxideQQuickWebFrame> childFrames READ childFrames NOTIFY childFramesChanged)
  Q_PROPERTY(QQmlListProperty<OxideQQuickMessageHandler> messageHandlers READ messageHandlers)
  Q_ENUMS(ChildFrameChangedType)

  Q_DECLARE_PRIVATE(OxideQQuickWebFrame)

 public:
  virtual ~OxideQQuickWebFrame();

  QUrl url() const;

  OxideQQuickWebFrame* parentFrame() const;
  QQmlListProperty<OxideQQuickWebFrame> childFrames();

  QQmlListProperty<OxideQQuickMessageHandler> messageHandlers();
  Q_INVOKABLE void addMessageHandler(OxideQQuickMessageHandler* handler);
  Q_INVOKABLE void removeMessageHandler(OxideQQuickMessageHandler* handler);

  Q_INVOKABLE OxideQQuickOutgoingMessageRequest*
      sendMessage(const QString& world_id,
                  const QString& msg_id,
                  const QVariant& args);
  Q_INVOKABLE void sendMessageNoReply(const QString& world_id,
                                      const QString& msg_id,
                                      const QVariant& args);

 Q_SIGNALS:
  void urlChanged();
  void childFramesChanged();
  void messageHandlersChanged();

 protected:
  friend class OxideQQuickWebViewPrivate;

  Q_DECL_HIDDEN OxideQQuickWebFrame();
  virtual void childEvent(QChildEvent* event);

  QScopedPointer<OxideQQuickWebFramePrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickWebFrame);

#endif // _OXIDE_QT_QUICK_API_WEB_FRAME_P_H_
