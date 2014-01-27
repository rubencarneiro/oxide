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

#include "oxideqquicknavigationhistory_p.h"

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

  oxide::qt::RenderWidgetHostViewDelegate* CreateRenderWidgetHostViewDelegate() Q_DECL_FINAL;
  oxide::qt::WebPopupMenuDelegate* CreateWebPopupMenuDelegate() Q_DECL_FINAL;

  void URLChanged() Q_DECL_FINAL;
  void TitleChanged() Q_DECL_FINAL;
  void CommandsUpdated() Q_DECL_FINAL;

  void LoadProgressChanged(double progress) Q_DECL_FINAL;

  void LoadEvent(OxideQLoadEvent* event) Q_DECL_FINAL;

  void NavigationEntryCommitted() Q_DECL_FINAL;
  void NavigationListPruned(bool from_front, int count) Q_DECL_FINAL;
  void NavigationEntryChanged(int index) Q_DECL_FINAL;

  oxide::qt::WebFrameAdapter* CreateWebFrame() Q_DECL_FINAL;

  QRect GetContainerBounds() Q_DECL_FINAL;

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
  OxideQQuickNavigationHistory navigationHistory;
  QQmlComponent* popup_menu;

 private:
  QScopedPointer<InitData> init_props_;
  QSharedPointer<OxideQQuickWebContext> default_context_;
  int load_progress_;
  OxideQQuickWebView* q_ptr;
};

#endif // _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
