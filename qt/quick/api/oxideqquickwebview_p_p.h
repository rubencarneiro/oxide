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

#ifndef _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
#define _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_

#include <QScopedPointer>
#include <QSharedPointer>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_web_view_adapter.h"

class OxideQQuickMessageHandler;
class OxideQQuickWebContext;
class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlComponent;
template <typename T> class QQmlListProperty;
QT_END_NAMESPACE

struct InitData {
  InitData() : incognito(false) {}

  bool incognito;
  QUrl url;
};

class OxideQQuickWebViewPrivate Q_DECL_FINAL :
     public oxide::qt::WebViewAdapter {
  Q_DECLARE_PUBLIC(OxideQQuickWebView)
  OXIDE_QT_DECLARE_ADAPTER

 public:
  OxideQQuickWebViewPrivate(OxideQQuickWebView* view);
  ~OxideQQuickWebViewPrivate();

  oxide::qt::WebFrameTreeDelegate* CreateWebFrameTreeDelegate() Q_DECL_FINAL;
  oxide::qt::RenderWidgetHostViewDelegate* CreateRenderWidgetHostViewDelegate() Q_DECL_FINAL;

  void URLChanged() Q_DECL_FINAL;
  void TitleChanged() Q_DECL_FINAL;
  void CommandsUpdated() Q_DECL_FINAL;

  void RootFrameChanged() Q_DECL_FINAL;

  void LoadStarted(const QUrl& url) Q_DECL_FINAL;
  void LoadStopped(const QUrl& url) Q_DECL_FINAL;
  void LoadFailed(const QUrl& url,
                  int error_code,
                  const QString& error_description) Q_DECL_FINAL;
  void LoadSucceeded(const QUrl& url) Q_DECL_FINAL;

  QRectF GetContainerBounds() Q_DECL_FINAL;

  void componentComplete();

  static void messageHandler_append(
      QQmlListProperty<OxideQQuickMessageHandler>* prop,
      OxideQQuickMessageHandler* message_handler);
  static int messageHandler_count(
      QQmlListProperty<OxideQQuickMessageHandler>* prop);
  static OxideQQuickMessageHandler* messageHandler_at(
      QQmlListProperty<OxideQQuickMessageHandler>* prop,
      int index);
  static void messageHandler_clear(
      QQmlListProperty<OxideQQuickMessageHandler>* prop);

  static OxideQQuickWebViewPrivate* get(OxideQQuickWebView* web_view);

  void addAttachedPropertyTo(QObject* object);

  InitData* init_props() { return init_props_.data(); }

  OxideQQuickWebContext* context;
  QQmlComponent* popup_menu;

 private:
  QScopedPointer<InitData> init_props_;
  QSharedPointer<OxideQQuickWebContext> default_context_;
  OxideQQuickWebView* q_ptr;
};

#endif // _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
