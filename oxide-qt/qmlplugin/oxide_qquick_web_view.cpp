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

#include "oxide_qquick_web_view.h"

#include <QRectF>
#include <QSizeF>

#include "url/gurl.h"

#include "core/browser/oxide_qt_web_view_host.h"
#include "core/browser/oxide_web_view_host_delegate.h"

QT_USE_NAMESPACE

struct InitData {
  InitData() : incognito(false) {}

  bool incognito;
  QUrl url;
};

class OxideQQuickWebViewPrivate : public oxide::qt::WebViewHostDelegate {
  Q_DECLARE_PUBLIC(OxideQQuickWebView)

 public:
  OxideQQuickWebViewPrivate(OxideQQuickWebView* view) :
      oxide::qt::WebViewHostDelegate(),
      q_ptr(view) {}

  void OnURLChanged();
  void OnTitleChanged();
  void OnLoadingChanged();
  void OnCommandsUpdated();

  OxideQQuickWebView* q_ptr;
  QScopedPointer<oxide:qt::WebViewHost> web_view_host_;
  QScopedPointer<InitData> init_props_;
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

void OxideQQuickWebView::visibilityChangedListener() {
  if (isVisible()) {
    d->web_view_host_->WasShown();
  } else {
    d->web_view_host_->WasHidden();
  }
}

void OxideQQuickWebView::geometryChanged(const QRectF& newGeometry,
                                         const QRectF& oldGeometry) {
  QQuickItem::geometryChanged(newGeometry, oldGeometry);

  for (QList<QQuickItem *>::iterator it = childItems().begin();
       it != childItems().end();
       ++it) {
    *it->setSize(newGeometry.size());
  }

  if (d->web_view_host_) {
    d->web_view_host_->UpdateSize(newGeometry.size());
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

void OxideQQuickWebView::classBegin() {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(!d->web_view_host_);
  Q_ASSERT(!d->init_props_);

  d->init_props_.reset(new InitData());
}

void OxideQQuickWebView::componentComplete() {
  Q_D(OxideQQuickWebView);

  Q_ASSERT(!d->web_view_host_);
  Q_ASSERT(d->init_props_);

  d->web_view_host_.reset(
      oxide::qt::WebViewHost::Create(
        this, d, d->init_props_.incognito,
        QSizeF(width(), height()), isVisible()));

  if (!d->init_props_.url.isEmpty()) {
    d->web_view_host_->SetURL(
        GURL(d->init_props_.url.toString().toStdString()));
  }

  d->init_props_.reset();
}

QUrl OxideQQuickWebView::url() const {
  Q_D(OxideQQuickWebView);

  if (d->init_props_) {
    return d->init_props_.url;
  }

  if (d->web_view_host_) {
    return QUrl(QString::fromStdString(d->web_view_host_->GetURL().spec()));
  }

  return QUrl();
}

void OxideQQuickWebView::setUrl(const QUrl& url) {
  Q_D(OxideQQuickWebView);

  if (d->init_props_) {
    d->init_props_.url = url;
  } else if (d->web_view_host_) {
    d->web_view_host_->SetURL(GURL(url.toString().toStdString()));
  }
}

QString OxideQQuickWebView::title() const {
  Q_D(OxideQQuickWebView);

  if (d->web_view_host_) {
    return QString::fromStdString(d->web_view_host_->GetTitle());
  }

  return QString();
}

bool OxideQQuickWebView::canGoBack() const {
  Q_D(OxideQQuickWebView);

  if (!d->web_view_host_) {
    return false;
  }

  return d->web_view_host_->CanGoBack();
}

bool OxideQQuickWebView::canGoForward() const {
  Q_D(OxideQQuickWebView);

  if (!d->web_view_host_) {
    return false;
  }

  return d->web_view_host_->CanGoForward();
}

bool OxideQQuickWebView::incognito() const {
  Q_D(OxideQQuickWebView);

  if (d->init_props_) {
    return d->init_props_.incognito;
  }

  if (d->web_view_host_) {
    return d->web_view_host_->IsIncognito();
  }

  return false;
}

void OxideQQuickWebView::setIncognito(bool incognito) {
  Q_D(OxideQQuickWebView);

  if (d->init_props_) {
    d->init_props_.incognito = incognito
  }
}

bool OxideQQuickWebView::loading() {
  Q_D(OxideQQuickWebView);

  if (d->web_view_host_) {
    return d->web_view_host_->IsLoading();
  }

  return false;
}

void OxideQQuickWebView::goBack() {
  Q_D(OxideQQuickWebView);

  if (d->web_view_host_) {
    d->web_view_host_->GoBack();
  }
}

void OxideQQuickWebView::goForward() {
  Q_D(OxideQQuickWebView);

  if (d->web_view_host_) {
    d->web_view_host_->GoForward();
  }
}

void OxideQQuickWebView::stop() {
  Q_D(OxideQQuickWebView);

  if (d->web_view_host_) {
    d->web_view_host_->Stop();
  }
}

void OxideQQuickWebView::reload() {
  Q_D(OxideQQuickWebView);

  if (d->web_view_host_) {
    d->web_view_host_->Reload();
  }
}
