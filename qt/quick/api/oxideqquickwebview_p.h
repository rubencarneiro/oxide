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

class OxideQGeolocationPermissionRequest;
class OxideQLoadEvent;
class OxideQNavigationRequest;
class OxideQNewViewRequest;
class OxideQWebPreferences;
class OxideQQuickNavigationHistory;
class OxideQQuickScriptMessageHandler;
class OxideQQuickWebContext;
class OxideQQuickWebFrame;
class OxideQQuickWebView;
class OxideQQuickWebViewPrivate;
class OxideQDownloadRequest;
class OxideQSecurityStatus;

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

  Q_ENUMS(LogMessageSeverityLevel);

  Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
  Q_PROPERTY(QString title READ title NOTIFY titleChanged)
  Q_PROPERTY(QUrl icon READ icon NOTIFY iconChanged)
  Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navigationHistoryChanged)
  Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navigationHistoryChanged)
  Q_PROPERTY(bool incognito READ incognito WRITE setIncognito NOTIFY incognitoChanged)
  Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
  Q_PROPERTY(bool fullscreen READ fullscreen WRITE setFullscreen NOTIFY fullscreenChanged)
  Q_PROPERTY(int loadProgress READ loadProgress NOTIFY loadProgressChanged)
  Q_PROPERTY(OxideQQuickWebFrame* rootFrame READ rootFrame NOTIFY rootFrameChanged)
  Q_PROPERTY(QQmlListProperty<OxideQQuickScriptMessageHandler> messageHandlers READ messageHandlers NOTIFY messageHandlersChanged)

  Q_PROPERTY(qreal viewportWidth READ viewportWidth NOTIFY viewportWidthChanged)
  Q_PROPERTY(qreal viewportHeight READ viewportHeight NOTIFY viewportHeightChanged)
  Q_PROPERTY(qreal contentWidth READ contentWidth NOTIFY contentWidthChanged)
  Q_PROPERTY(qreal contentHeight READ contentHeight NOTIFY contentHeightChanged)
  Q_PROPERTY(qreal contentX READ contentX NOTIFY contentXChanged)
  Q_PROPERTY(qreal contentY READ contentY NOTIFY contentYChanged)

  Q_PROPERTY(QQmlComponent* popupMenu READ popupMenu WRITE setPopupMenu NOTIFY popupMenuChanged)

  Q_PROPERTY(QQmlComponent* alertDialog READ alertDialog WRITE setAlertDialog NOTIFY alertDialogChanged)
  Q_PROPERTY(QQmlComponent* confirmDialog READ confirmDialog WRITE setConfirmDialog NOTIFY confirmDialogChanged)
  Q_PROPERTY(QQmlComponent* promptDialog READ promptDialog WRITE setPromptDialog NOTIFY promptDialogChanged)
  Q_PROPERTY(QQmlComponent* beforeUnloadDialog READ beforeUnloadDialog WRITE setBeforeUnloadDialog NOTIFY beforeUnloadDialogChanged)

  Q_PROPERTY(QQmlComponent* filePicker READ filePicker WRITE setFilePicker NOTIFY filePickerChanged)

  Q_PROPERTY(OxideQQuickWebContext* context READ context WRITE setContext NOTIFY contextChanged)
  Q_PROPERTY(OxideQWebPreferences* preferences READ preferences WRITE setPreferences NOTIFY preferencesChanged)

  Q_PROPERTY(OxideQQuickNavigationHistory* navigationHistory READ navigationHistory CONSTANT)
  Q_PROPERTY(OxideQSecurityStatus* securityStatus READ securityStatus CONSTANT)

  Q_PROPERTY(OxideQNewViewRequest* request READ request WRITE setRequest)

  Q_DECLARE_PRIVATE(OxideQQuickWebView)

 public:
  OxideQQuickWebView(QQuickItem* parent = NULL);
  virtual ~OxideQQuickWebView();

  enum LogMessageSeverityLevel {
    LogSeverityVerbose = -1,
    LogSeverityInfo,
    LogSeverityWarning,
    LogSeverityError,
    LogSeverityErrorReport,
    LogSeverityFatal
  };

  void componentComplete();

  QUrl url() const;
  void setUrl(const QUrl& url);

  QString title() const;

  QUrl icon() const;

  bool canGoBack() const;
  bool canGoForward() const;

  bool incognito() const;
  void setIncognito(bool incognito);

  bool loading() const;

  bool fullscreen() const;
  void setFullscreen(bool fullscreen);

  int loadProgress() const;

  OxideQQuickWebFrame* rootFrame() const;

  QQmlListProperty<OxideQQuickScriptMessageHandler> messageHandlers();
  Q_INVOKABLE void addMessageHandler(OxideQQuickScriptMessageHandler* handler);
  Q_INVOKABLE void removeMessageHandler(OxideQQuickScriptMessageHandler* handler);

  qreal viewportWidth() const;
  qreal viewportHeight() const;
  qreal contentWidth() const;
  qreal contentHeight() const;
  qreal contentX() const;
  qreal contentY() const;

  QQmlComponent* popupMenu() const;
  void setPopupMenu(QQmlComponent* popup_menu);

  QQmlComponent* alertDialog() const;
  void setAlertDialog(QQmlComponent* alert_dialog);

  QQmlComponent* confirmDialog() const;
  void setConfirmDialog(QQmlComponent* confirm_dialog);

  QQmlComponent* promptDialog() const;
  void setPromptDialog(QQmlComponent* prompt_dialog);

  QQmlComponent* beforeUnloadDialog() const;
  void setBeforeUnloadDialog(QQmlComponent* before_unload_dialog);

  QQmlComponent* filePicker() const;
  void setFilePicker(QQmlComponent* file_picker);

  OxideQQuickWebContext* context() const;
  void setContext(OxideQQuickWebContext* context);

  OxideQWebPreferences* preferences();
  void setPreferences(OxideQWebPreferences* prefs);

  OxideQQuickNavigationHistory* navigationHistory();
  OxideQSecurityStatus* securityStatus();

  OxideQNewViewRequest* request() const;
  void setRequest(OxideQNewViewRequest* request);

  static OxideQQuickWebViewAttached* qmlAttachedProperties(QObject* object);

 public Q_SLOTS:
  void goBack();
  void goForward();
  void stop();
  void reload();
  void loadHtml(const QString& html, const QUrl& baseUrl = QUrl());

 Q_SIGNALS:
  void urlChanged();
  void titleChanged();
  void iconChanged();
  void navigationHistoryChanged();
  void incognitoChanged();
  void loadingChanged(OxideQLoadEvent* loadEvent);
  void fullscreenChanged();
  void loadProgressChanged();
  void rootFrameChanged();
  void frameAdded(OxideQQuickWebFrame* frame);
  void frameRemoved(OxideQQuickWebFrame* frame);
  void popupMenuChanged();
  void alertDialogChanged();
  void confirmDialogChanged();
  void promptDialogChanged();
  void beforeUnloadDialogChanged();
  void filePickerChanged();
  void contextChanged();
  void preferencesChanged();
  void messageHandlersChanged();
  void viewportWidthChanged();
  void viewportHeightChanged();
  void contentWidthChanged();
  void contentHeightChanged();
  void contentXChanged();
  void contentYChanged();
  void fullscreenRequested(bool fullscreen);
  void navigationRequested(OxideQNavigationRequest *request);
  void newViewRequested(OxideQNewViewRequest* request);
  void geolocationPermissionRequested(const QJSValue& request);
  void javaScriptConsoleMessage(LogMessageSeverityLevel level,
                                const QString& message,
                                int lineNumber,
                                const QString& sourceId);
  void downloadRequested(OxideQDownloadRequest* request);

 private:
  Q_PRIVATE_SLOT(d_func(), void contextConstructed());
  Q_PRIVATE_SLOT(d_func(), void contextWillBeDestroyed());

  Q_PRIVATE_SLOT(d_func(), void onWindowChanged(QQuickWindow*));

  void connectNotify(const QMetaMethod& signal) Q_DECL_FINAL;
  void disconnectNotify(const QMetaMethod& signal) Q_DECL_FINAL;

  void geometryChanged(const QRectF& newGeometry,
                       const QRectF& oldGeometry) Q_DECL_FINAL;
  void itemChange(QQuickItem::ItemChange change,
                  const QQuickItem::ItemChangeData& value) Q_DECL_FINAL;
  QSGNode* updatePaintNode(
      QSGNode* oldNode,
      UpdatePaintNodeData * updatePaintNodeData) Q_DECL_FINAL;
  void updatePolish() Q_DECL_FINAL;

  QScopedPointer<OxideQQuickWebViewPrivate> d_ptr;
};

QML_DECLARE_TYPE(OxideQQuickWebView)
QML_DECLARE_TYPEINFO(OxideQQuickWebView, QML_HAS_ATTACHED_PROPERTIES)

#endif // _OXIDE_QT_QUICK_API_WEB_VIEW_P_H_
