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

#ifndef _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_H_
#define _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_H_

#include <QQmlListProperty>
#include <QQuickItem>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QQmlComponent;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class OxideQQuickMessageHandler;
class OxideQQuickWebFrame;
class OxideQQuickWebViewContext;

namespace oxide {
namespace qt {
class QQuickWebViewPrivate;
}
}

class Q_DECL_EXPORT OxideQQuickWebView : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
  Q_PROPERTY(QString title READ title NOTIFY titleChanged)
  Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY commandsUpdated)
  Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY commandsUpdated)
  Q_PROPERTY(bool incognito READ incognito WRITE setIncognito)
  Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
  Q_PROPERTY(OxideQQuickWebFrame* rootFrame READ rootFrame NOTIFY rootFrameChanged)
  Q_PROPERTY(QQmlListProperty<OxideQQuickMessageHandler> messageHandlers READ messageHandlers NOTIFY messageHandlersChanged)

  Q_PROPERTY(QQmlComponent* popupMenu READ popupMenu WRITE setPopupMenu NOTIFY popupMenuChanged)

  Q_PROPERTY(OxideQQuickWebViewContext* context READ context WRITE setContext)

  Q_DECLARE_PRIVATE(oxide::qt::QQuickWebView)

 public:
  OxideQQuickWebView(QQuickItem* parent = NULL);
  virtual ~OxideQQuickWebView();

  void componentComplete();

  QUrl url() const;
  void setUrl(const QUrl& url);

  QString title() const;

  bool canGoBack() const;
  bool canGoForward() const;

  bool incognito() const;
  void setIncognito(bool incognito);

  bool loading() const;

  OxideQQuickWebFrame* rootFrame() const;

  QQmlListProperty<OxideQQuickMessageHandler> messageHandlers();
  Q_INVOKABLE void addMessageHandler(OxideQQuickMessageHandler* handler);
  Q_INVOKABLE void removeMessageHandler(OxideQQuickMessageHandler* handler);

  QQmlComponent* popupMenu() const;
  void setPopupMenu(QQmlComponent* popup_menu);

  OxideQQuickWebViewContext* context() const;
  void setContext(OxideQQuickWebViewContext* context);

 public Q_SLOTS:
  void goBack();
  void goForward();
  void stop();
  void reload();

 Q_SIGNALS:
  void urlChanged();
  void titleChanged();
  void commandsUpdated();
  void loadingChanged();
  void rootFrameChanged();
  void popupMenuChanged();
  void messageHandlersChanged();

 private Q_SLOTS:
  void visibilityChangedListener();

 private:
  virtual void geometryChanged(const QRectF& newGeometry,
                               const QRectF& oldGeometry);

  QScopedPointer<oxide::qt::QQuickWebViewPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickWebView)

#endif // _OXIDE_QT_LIB_PUBLIC_QQUICK_WEB_VIEW_H_
