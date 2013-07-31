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

#ifndef _OXIDE_QT_CLIENT_QQUICK_WEB_VIEW_H_
#define _OXIDE_QT_CLIENT_QQUICK_WEB_VIEW_H_

#include <QQuickItem>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <QUrl>

#include "shared/common/oxide_export.h"

QT_BEGIN_NAMESPACE
class QQmlComponent;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class OxideQQuickWebViewPrivate;

class OXIDE_EXPORT OxideQQuickWebView : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
  Q_PROPERTY(QString title READ title NOTIFY titleChanged)
  Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY commandsUpdated)
  Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY commandsUpdated)
  Q_PROPERTY(bool incognito READ incognito WRITE setIncognito)
  Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

  Q_PROPERTY(QQmlComponent* popupMenu READ popupMenu WRITE setPopupMenu NOTIFY popupMenuChanged)

  Q_DECLARE_PRIVATE(OxideQQuickWebView)

 public:
  OxideQQuickWebView(QQuickItem* parent = NULL);
  virtual ~OxideQQuickWebView();

  void classBegin();
  void componentComplete();

  QUrl url() const;
  void setUrl(const QUrl& url);

  QString title() const;

  bool canGoBack() const;
  bool canGoForward() const;

  bool incognito() const;
  void setIncognito(bool incognito);

  bool loading() const;

  QQmlComponent* popupMenu() const;
  void setPopupMenu(QQmlComponent* popup_menu);

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
  void popupMenuChanged();

 private Q_SLOTS:
  void visibilityChangedListener();

 private:
  virtual void geometryChanged(const QRectF& newGeometry,
                               const QRectF& oldGeometry);

  QScopedPointer<OxideQQuickWebViewPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickWebView)

#endif // _OXIDE_QT_CLIENT_QQUICK_WEB_VIEW_H_
