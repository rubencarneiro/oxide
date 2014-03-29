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

#ifndef _OXIDE_QT_QUICK_API_WEB_VIEW_P_H_
#define _OXIDE_QT_QUICK_API_WEB_VIEW_P_H_

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

class OxideQLoadEvent;
class OxideQWebPreferences;
class OxideQQuickNavigationHistory;
class OxideQQuickScriptMessageHandler;
class OxideQQuickWebContext;
class OxideQQuickWebFrame;
class OxideQQuickWebView;
class OxideQQuickWebViewPrivate;

class OxideQQuickWebViewAttached : public QObject {
  Q_OBJECT
  Q_PROPERTY(OxideQQuickWebView* view READ view)

 public:
  OxideQQuickWebViewAttached(QObject* parent);
  virtual ~OxideQQuickWebViewAttached();

  OxideQQuickWebView* view() const;
  void setView(OxideQQuickWebView* view);

 private:
  OxideQQuickWebView* view_;
};

class OxideQQuickWebView : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
  Q_PROPERTY(QString title READ title NOTIFY titleChanged)
  Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationHistoryChanged)
  Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navigationHistoryChanged)
  Q_PROPERTY(bool incognito READ incognito WRITE setIncognito)
  Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
  Q_PROPERTY(int loadProgress READ loadProgress NOTIFY loadProgressChanged)
  Q_PROPERTY(OxideQQuickWebFrame* rootFrame READ rootFrame NOTIFY rootFrameChanged)
  Q_PROPERTY(QQmlListProperty<OxideQQuickScriptMessageHandler> messageHandlers READ messageHandlers NOTIFY messageHandlersChanged)

  Q_PROPERTY(QQmlComponent* popupMenu READ popupMenu WRITE setPopupMenu NOTIFY popupMenuChanged)

  Q_PROPERTY(OxideQQuickWebContext* context READ context WRITE setContext NOTIFY contextChanged)
  Q_PROPERTY(OxideQWebPreferences* preferences READ preferences WRITE setPreferences NOTIFY preferencesChanged)

  Q_PROPERTY(OxideQQuickNavigationHistory* navigationHistory READ navigationHistory CONSTANT)

  Q_DECLARE_PRIVATE(OxideQQuickWebView)

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

  int loadProgress() const;

  OxideQQuickWebFrame* rootFrame() const;

  QQmlListProperty<OxideQQuickScriptMessageHandler> messageHandlers();
  Q_INVOKABLE void addMessageHandler(OxideQQuickScriptMessageHandler* handler);
  Q_INVOKABLE void removeMessageHandler(OxideQQuickScriptMessageHandler* handler);

  QQmlComponent* popupMenu() const;
  void setPopupMenu(QQmlComponent* popup_menu);

  OxideQQuickWebContext* context() const;
  void setContext(OxideQQuickWebContext* context);

  OxideQWebPreferences* preferences();
  void setPreferences(OxideQWebPreferences* prefs);

  OxideQQuickNavigationHistory* navigationHistory();

  static OxideQQuickWebViewAttached* qmlAttachedProperties(QObject* object);

 public Q_SLOTS:
  void goBack();
  void goForward();
  void stop();
  void reload();

 Q_SIGNALS:
  void urlChanged();
  void titleChanged();
  void navigationHistoryChanged();
  void loadingChanged(OxideQLoadEvent* loadEvent);
  void loadProgressChanged();
  void rootFrameChanged();
  void frameAdded(OxideQQuickWebFrame* frame);
  void frameRemoved(OxideQQuickWebFrame* frame);
  void popupMenuChanged();
  void contextChanged();
  void preferencesChanged();
  void messageHandlersChanged();

 private:
  Q_PRIVATE_SLOT(d_func(), void contextInitialized());
  Q_PRIVATE_SLOT(d_func(), void contextWillBeDestroyed());

  void geometryChanged(const QRectF& newGeometry,
                       const QRectF& oldGeometry) Q_DECL_FINAL;
  void itemChange(QQuickItem::ItemChange change,
                  const QQuickItem::ItemChangeData& value) Q_DECL_FINAL;

  QScopedPointer<OxideQQuickWebViewPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickWebView)
QML_DECLARE_TYPEINFO(OxideQQuickWebView, QML_HAS_ATTACHED_PROPERTIES)

#endif // _OXIDE_QT_QUICK_API_WEB_VIEW_P_H_
