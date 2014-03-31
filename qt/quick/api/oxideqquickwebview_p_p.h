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

class OxideQQuickScriptMessageHandler;
class OxideQQuickWebContext;
class OxideQQuickWebContextPrivate;
class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlComponent;
template <typename T> class QQmlListProperty;
QT_END_NAMESPACE

class OxideQQuickWebViewPrivate Q_DECL_FINAL :
     public oxide::qt::WebViewAdapter {
  Q_DECLARE_PUBLIC(OxideQQuickWebView)

 public:
  ~OxideQQuickWebViewPrivate();

  static OxideQQuickWebViewPrivate* get(OxideQQuickWebView* web_view);

  void addAttachedPropertyTo(QObject* object);

 private:
  OxideQQuickWebViewPrivate(OxideQQuickWebView* view);

  oxide::qt::WebPopupMenuDelegate* CreateWebPopupMenuDelegate() Q_DECL_FINAL;

  void OnInitialized(bool orig_incognito,
                     oxide::qt::WebContextAdapter* orig_context) Q_DECL_FINAL;

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
  bool IsVisible() const Q_DECL_FINAL;

  void OnWebPreferencesChanged() Q_DECL_FINAL;

  void FrameAdded(oxide::qt::WebFrameAdapter* frame) Q_DECL_FINAL;
  void FrameRemoved(oxide::qt::WebFrameAdapter* frame) Q_DECL_FINAL;

  bool CanCreateWindows() const Q_DECL_FINAL;

  void NewViewRequested(OxideQNewViewRequest* request) Q_DECL_FINAL;

  void completeConstruction();

  static void messageHandler_append(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
      OxideQQuickScriptMessageHandler* message_handler);
  static int messageHandler_count(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop);
  static OxideQQuickScriptMessageHandler* messageHandler_at(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop,
      int index);
  static void messageHandler_clear(
      QQmlListProperty<OxideQQuickScriptMessageHandler>* prop);

  void contextInitialized();
  void contextWillBeDestroyed();
  void attachContextSignals(OxideQQuickWebContextPrivate* context);
  void detachContextSignals(OxideQQuickWebContextPrivate* context);

  QSharedPointer<OxideQQuickWebContext> default_context_;
  int load_progress_;
  bool constructed_;
  OxideQQuickNavigationHistory navigation_history_;
  QQmlComponent* popup_menu_;
};

#endif // _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
