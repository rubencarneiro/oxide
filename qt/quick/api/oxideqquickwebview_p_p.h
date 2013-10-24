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

#include <QList>
#include <QSharedPointer>
#include <QtGlobal>
#include <QUrl>

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"

#include "shared/browser/oxide_message_dispatcher_browser.h"
#include "shared/browser/oxide_web_view.h"

class OxideQQuickMessageHandler;
class OxideQQuickWebContext;
class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlComponent;
template <typename T> class QQmlListProperty;
class QSizeF;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class WebFrameTree;

struct InitData {
  InitData() : incognito(false) {}

  bool incognito;
  QUrl url;
};

class QQuickWebViewPrivate : public oxide::WebView {
  Q_DECLARE_PUBLIC(OxideQQuickWebView)

 public:
  static QQuickWebViewPrivate* Create(OxideQQuickWebView* view);
  ~QQuickWebViewPrivate();

  size_t GetMessageHandlerCount() const FINAL;
  oxide::MessageHandler* GetMessageHandlerAt(size_t index) const FINAL;

  void RootFrameCreated(oxide::WebFrame* root) FINAL;

  content::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) FINAL;

  gfx::Rect GetContainerBounds() FINAL;

  WebPopupMenu* CreatePopupMenu() FINAL;

  void UpdateVisibility();

  base::WeakPtr<QQuickWebViewPrivate> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  WebFrameTree* CreateWebFrameTree(content::RenderViewHost* rvh);

  void componentComplete();

  InitData* init_props() const { return init_props_.get(); }

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

  QList<OxideQQuickMessageHandler *>& message_handlers() {
    return message_handlers_;
  }

  static QQuickWebViewPrivate* get(OxideQQuickWebView* web_view);

  void updateSize(const QSizeF& size);

  QUrl url() const;
  void setUrl(const QUrl& url);

  void addAttachedPropertyTo(QObject* object);

  OxideQQuickWebContext* context;
  QQmlComponent* popup_menu;

 private:
  QQuickWebViewPrivate(OxideQQuickWebView* view);

  void OnURLChanged() FINAL;
  void OnTitleChanged() FINAL;
  void OnCommandsUpdated() FINAL;

  void OnRootFrameChanged() FINAL;

  void OnLoadStarted(const GURL& validated_url,
                     bool is_error_frame) FINAL;
  void OnLoadStopped(const GURL& validated_url) FINAL;
  void OnLoadFailed(const GURL& validated_url,
                    int error_code,
                    const std::string& error_description) FINAL;
  void OnLoadSucceeded(const GURL& validated_url) FINAL;

  OxideQQuickWebView* q_ptr;
  scoped_ptr<InitData> init_props_;
  QSharedPointer<OxideQQuickWebContext> default_context_;
  QList<OxideQQuickMessageHandler *> message_handlers_;
  base::WeakPtrFactory<QQuickWebViewPrivate> weak_factory_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_QUICK_API_WEB_VIEW_P_P_H_
