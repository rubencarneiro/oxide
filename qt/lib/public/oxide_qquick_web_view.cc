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

#include "oxide_qquick_web_view_p.h"

#include <QPointF>
#include <QQuickWindow>
#include <QRectF>
#include <QSharedPointer>
#include <QSize>

#include "base/bind.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents.h"
#include "ui/gfx/size.h"

#include "shared/browser/oxide_web_contents_view.h"
#include "shared/browser/oxide_web_contents_view_delegate.h"
#include "shared/browser/oxide_web_view.h"
#include "shared/common/oxide_user_script.h"

#include "qt/lib/browser/oxide_qt_render_widget_host_view_qquick.h"
#include "qt/lib/browser/oxide_qt_web_frame.h"
#include "qt/lib/browser/oxide_qt_web_popup_menu_qquick.h"
#include "qt/lib/common/oxide_qt_content_main_delegate.h"

#include "oxide_qquick_web_view_context_p.h"
#include "oxide_qquick_web_view_context_p_p.h"

QT_USE_NAMESPACE

struct InitData {
  InitData() : incognito(false) {}

  bool incognito;
  QUrl url;
};

class OxideQQuickWebViewPrivate FINAL :
    public oxide::WebView,
    public oxide::WebContentsViewDelegate {
  Q_DECLARE_PUBLIC(OxideQQuickWebView)

 public:
  ~OxideQQuickWebViewPrivate();

  OxideQQuickWebViewPrivate(OxideQQuickWebView* view) :
      context_(NULL),
      popup_menu_(NULL),
      q_ptr(view),
      init_props_(new InitData()),
      weak_factory_(this) {}

  void UpdateVisibility();

  content::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) FINAL;

  gfx::Rect GetContainerBounds() FINAL;

  oxide::WebPopupMenu* CreatePopupMenu() FINAL;

  base::WeakPtr<OxideQQuickWebViewPrivate> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

  void componentComplete();

  InitData* init_props() const { return init_props_.get(); }

  OxideQQuickWebViewContext* context_;
  QQmlComponent* popup_menu_;

 private:
  void OnURLChanged() FINAL;
  void OnTitleChanged() FINAL;
  void OnLoadingChanged() FINAL;
  void OnCommandsUpdated() FINAL;
  void OnRootFrameChanged() FINAL;

  oxide::WebFrame* AllocWebFrame(int64 frame_id) FINAL;

  void OnExecuteScriptFinished(bool error, const std::string& error_string) {}

  OxideQQuickWebView* q_ptr;
  scoped_ptr<InitData> init_props_;
  QSharedPointer<OxideQQuickWebViewContext> default_context_;
  base::WeakPtrFactory<OxideQQuickWebViewPrivate> weak_factory_;
};

void OxideQQuickWebViewPrivate::OnURLChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->urlChanged();
}

void OxideQQuickWebViewPrivate::OnTitleChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->titleChanged();
}

void OxideQQuickWebViewPrivate::OnLoadingChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->loadingChanged();
}

void OxideQQuickWebViewPrivate::OnCommandsUpdated() {
  Q_Q(OxideQQuickWebView);

  emit q->commandsUpdated();
}

void OxideQQuickWebViewPrivate::OnRootFrameChanged() {
  Q_Q(OxideQQuickWebView);

  emit q->rootFrameChanged();
}

oxide::WebFrame* OxideQQuickWebViewPrivate::AllocWebFrame(
    int64 frame_id) {
  return new oxide::qt::WebFrame(frame_id);
}

OxideQQuickWebViewPrivate::~OxideQQuickWebViewPrivate() {
  // It's important that this goes away before our context
  DestroyWebContents();
}

void OxideQQuickWebViewPrivate::UpdateVisibility() {
  Q_Q(OxideQQuickWebView);

  if (init_props_) {
    return;
  }

  if (q->isVisible()) {
    Shown();
  } else {
    Hidden();
  }  
}

content::RenderWidgetHostView* OxideQQuickWebViewPrivate::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host) {
  Q_Q(OxideQQuickWebView);

  return new oxide::qt::RenderWidgetHostViewQQuick(render_widget_host, q);
}

gfx::Rect OxideQQuickWebViewPrivate::GetContainerBounds() {
  Q_Q(OxideQQuickWebView);

  QPointF pos(q->mapToScene(QPointF(0,0)));
  if (q->window()) {
    // We could be called before being added to a scene
    pos += q->window()->position();
  }

  return gfx::Rect(qRound(pos.x()),
                   qRound(pos.y()),
                   qRound(q->width()),
                   qRound(q->height()));
}

oxide::WebPopupMenu* OxideQQuickWebViewPrivate::CreatePopupMenu() {
  Q_Q(OxideQQuickWebView);

  return new oxide::qt::WebPopupMenuQQuick(q, web_contents());
}

void OxideQQuickWebViewPrivate::componentComplete() {
  Q_Q(OxideQQuickWebView);

  Q_ASSERT(init_props_);

  if (!context_) {
    // Ok, we handle the default context a bit differently. If our context
    // comes from setContext(), then we don't hold a strong reference to it
    // because it will be owned by someone else in the QML object
    // hierarchy. However, the default context is not in this hierarchy and
    // has no QObject parent, so we use reference counting for it instead to
    // ensure that it is freed once all webviews are closed
    default_context_.reset(OxideQQuickWebViewContext::defaultContext());
    context_ = default_context_.data();
  }

  Init(OxideQQuickWebViewContextPrivate::get(context_)->GetContext(),
       this, init_props_->incognito,
       gfx::Size(qRound(q->width()), qRound(q->height())));

  if (!init_props_->url.isEmpty()) {
    SetURL(GURL(init_props_->url.toString().toStdString()));
  }

  init_props_.reset();

  UpdateVisibility();
}

void OxideQQuickWebView::visibilityChangedListener() {
  Q_D(OxideQQuickWebView);

  d->UpdateVisibility();
}

void OxideQQuickWebView::geometryChanged(const QRectF& newGeometry,
                                         const QRectF& oldGeometry) {
  Q_D(OxideQQuickWebView);

  QQuickItem::geometryChanged(newGeometry, oldGeometry);

  for (int i = 0; i < childItems().count(); ++i) {
    QQuickItem* item = childItems().at(i);
    item->setSize(newGeometry.size());
  }

  if (d->web_contents()) {
    d->UpdateSize(gfx::Size(qRound(width()), qRound(height())));
  }
}

OxideQQuickWebView::OxideQQuickWebView(QQuickItem* parent) :
    QQuickItem(parent),
    d_ptr(new OxideQQuickWebViewPrivate(this)) {
  QObject::connect(this, SIGNAL(visibleChanged()),
                   this, SLOT(visibilityChangedListener()));
}

OxideQQuickWebView::~OxideQQuickWebView() {
  QObject::disconnect(this, SIGNAL(visibleChanged()),
                      this, SLOT(visibilityChangedListener()));
}

void OxideQQuickWebView::componentComplete() {
  Q_D(OxideQQuickWebView);

  QQuickItem::componentComplete();

  d->componentComplete();
}

QUrl OxideQQuickWebView::url() const {
  Q_D(const OxideQQuickWebView);

  return QUrl(QString::fromStdString(d->GetURL().spec()));
}

void OxideQQuickWebView::setUrl(const QUrl& url) {
  Q_D(OxideQQuickWebView);

  if (d->init_props()) {
    d->init_props()->url = url;
  } else {
    d->SetURL(GURL(url.toString().toStdString()));
  }
}

QString OxideQQuickWebView::title() const {
  Q_D(const OxideQQuickWebView);

  return QString::fromStdString(d->GetTitle());
}

bool OxideQQuickWebView::canGoBack() const {
  Q_D(const OxideQQuickWebView);

  return d->CanGoBack();
}

bool OxideQQuickWebView::canGoForward() const {
  Q_D(const OxideQQuickWebView);

  return d->CanGoForward();
}

bool OxideQQuickWebView::incognito() const {
  Q_D(const OxideQQuickWebView);

  return d->IsIncognito();
}

void OxideQQuickWebView::setIncognito(bool incognito) {
  Q_D(OxideQQuickWebView);

  if (d->init_props()) {
    d->init_props()->incognito = incognito;
  }
}

bool OxideQQuickWebView::loading() const {
  Q_D(const OxideQQuickWebView);

  return d->IsLoading();
}

OxideQWebFrame* OxideQQuickWebView::rootFrame() const {
  Q_D(const OxideQQuickWebView);

  return static_cast<oxide::qt::WebFrame *>(d->GetRootFrame())->q_web_frame();
}

QQmlComponent* OxideQQuickWebView::popupMenu() const {
  Q_D(const OxideQQuickWebView);

  return d->popup_menu_;
}

void OxideQQuickWebView::setPopupMenu(QQmlComponent* popup_menu) {
  Q_D(OxideQQuickWebView);

  d->popup_menu_ = popup_menu;
  emit popupMenuChanged();
}

OxideQQuickWebViewContext* OxideQQuickWebView::context() const {
  Q_D(const OxideQQuickWebView);

  return d->context_;
}

void OxideQQuickWebView::setContext(OxideQQuickWebViewContext* context) {
  Q_D(OxideQQuickWebView);

  if (!d->web_contents()) {
    Q_ASSERT(!d->context_);
    d->context_ = context;
  }
}

void OxideQQuickWebView::goBack() {
  Q_D(OxideQQuickWebView);

  d->GoBack();
}

void OxideQQuickWebView::goForward() {
  Q_D(OxideQQuickWebView);

  d->GoForward();
}

void OxideQQuickWebView::stop() {
  Q_D(OxideQQuickWebView);

  d->Stop();
}

void OxideQQuickWebView::reload() {
  Q_D(OxideQQuickWebView);

  d->Reload();
}
