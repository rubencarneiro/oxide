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

#ifndef OXIDE_QTQUICK_WEB_FRAME
#define OXIDE_QTQUICK_WEB_FRAME

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QtGlobal>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtQml/QQmlListProperty>
#include <QtQml/QtQml>

#include <OxideQtQuick/oxideqquickglobal.h>

class OxideQQuickScriptMessageHandler;
class OxideQQuickScriptMessageRequest;
class OxideQQuickWebFramePrivate;
class OxideQQuickWebViewPrivate;

class OXIDE_QTQUICK_EXPORT OxideQQuickWebFrame : public QObject {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)
  Q_PROPERTY(OxideQQuickWebFrame* parentFrame READ parentFrame)
  Q_PROPERTY(QQmlListProperty<OxideQQuickWebFrame> childFrames READ childFrames NOTIFY childFramesChanged)
  Q_PROPERTY(QQmlListProperty<OxideQQuickScriptMessageHandler> messageHandlers READ messageHandlers NOTIFY messageHandlersChanged)
  Q_ENUMS(ChildFrameChangedType)

  Q_DECLARE_PRIVATE(OxideQQuickWebFrame)

 public:
  ~OxideQQuickWebFrame() Q_DECL_OVERRIDE;

  QUrl url() const;

  OxideQQuickWebFrame* parentFrame() const;
  QQmlListProperty<OxideQQuickWebFrame> childFrames();

  QQmlListProperty<OxideQQuickScriptMessageHandler> messageHandlers();
  Q_INVOKABLE void addMessageHandler(OxideQQuickScriptMessageHandler* handler);
  Q_INVOKABLE void removeMessageHandler(OxideQQuickScriptMessageHandler* handler);

  Q_INVOKABLE OxideQQuickScriptMessageRequest*
      sendMessage(const QUrl& context,
                  const QString& msg_id,
                  const QVariant& payload = QVariant());
  Q_INVOKABLE void sendMessageNoReply(const QUrl& context,
                                      const QString& msg_id,
                                      const QVariant& payload = QVariant());

 Q_SIGNALS:
  void urlChanged();
  void childFramesChanged();
  void messageHandlersChanged();

 private:
  OxideQQuickWebFrame();

  QScopedPointer<OxideQQuickWebFramePrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickWebFrame);

#endif // OXIDE_QTQUICK_WEB_FRAME
