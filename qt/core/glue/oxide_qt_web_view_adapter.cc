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

#include "oxide_qt_web_view_adapter.h"

#include <QtDebug>

#include "base/logging.h"
#include "ui/gfx/size.h"
#include "url/gurl.h"

#include "qt/core/api/oxideqnewviewrequest_p.h"
#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/core/api/oxideqwebpreferences_p.h"
#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/browser/oxide_qt_web_preferences.h"
#include "qt/core/browser/oxide_qt_web_view.h"

#include "oxide_qt_web_context_adapter_p.h"
#include "oxide_qt_web_frame_adapter.h"

namespace oxide {
namespace qt {

void WebViewAdapter::Initialized() {
  DCHECK(isInitialized());

  OnInitialized(construct_props_->incognito,
                construct_props_->context);
  construct_props_.reset();
}

void WebViewAdapter::WebPreferencesDestroyed() {
  OnWebPreferencesChanged();
}

WebViewAdapter::WebViewAdapter(QObject* q) :
    AdapterBase(q),
    priv(WebView::Create(this)),
    construct_props_(new ConstructProperties()),
    created_with_new_view_request_(false) {}

WebViewAdapter::~WebViewAdapter() {}

void WebViewAdapter::init() {
  if (created_with_new_view_request_ || isInitialized()) {
    return;
  }

  oxide::WebView::Params params;
  params.context =
      WebContextAdapterPrivate::get(construct_props_->context)->GetContext();
  params.incognito = construct_props_->incognito;

  priv->Init(&params);
}

QUrl WebViewAdapter::url() const {
  return QUrl(QString::fromStdString(priv->GetURL().spec()));
}

void WebViewAdapter::setUrl(const QUrl& url) {
  priv->SetURL(GURL(url.toString().toStdString()));
}

QString WebViewAdapter::title() const {
  return QString::fromStdString(priv->GetTitle());
}

bool WebViewAdapter::canGoBack() const {
  return priv->CanGoBack();
}

bool WebViewAdapter::canGoForward() const {
  return priv->CanGoForward();
}

bool WebViewAdapter::incognito() const {
  if (construct_props_) {
    return construct_props_->incognito;
  }

  return priv->IsIncognito();  
}

void WebViewAdapter::setIncognito(bool incognito) {
  if (!construct_props_) {
    qWarning() << "Cannot change incognito mode after WebView is initialized";
    return;
  }

  construct_props_->incognito = incognito;
}

bool WebViewAdapter::loading() const {
  return priv->IsLoading();
}

bool WebViewAdapter::fullscreen() const {
  return priv->IsFullscreen();
}

void WebViewAdapter::setFullscreen(bool fullscreen) {
  priv->SetIsFullscreen(fullscreen);
}

WebFrameAdapter* WebViewAdapter::rootFrame() const {
  WebFrame* frame = static_cast<WebFrame *>(priv->GetRootFrame());
  if (!frame) {
    return NULL;
  }

  return frame->adapter();
}

WebContextAdapter* WebViewAdapter::context() const {
  if (construct_props_) {
    return construct_props_->context;
  }

  WebContextAdapterPrivate* context =
      WebContextAdapterPrivate::FromBrowserContext(priv->GetBrowserContext());
  if (!context) {
    return NULL;
  }

  return context->adapter();
}

void WebViewAdapter::setContext(WebContextAdapter* context) {
  DCHECK(construct_props_);
  construct_props_->context = context;
}

void WebViewAdapter::updateSize(const QSize& size) {
  priv->UpdateSize(gfx::Size(size.width(), size.height()));
}

void WebViewAdapter::updateVisibility(bool visible) {
  priv->UpdateVisibility(visible);
}

void WebViewAdapter::goBack() {
  priv->GoBack();
}

void WebViewAdapter::goForward() {
  priv->GoForward();
}

void WebViewAdapter::stop() {
  priv->Stop();
}

void WebViewAdapter::reload() {
  priv->Reload();
}

void WebViewAdapter::loadHtml(const QString& html, const QUrl& baseUrl) {
  QByteArray encodedData = html.toUtf8().toPercentEncoding();
  priv->LoadData(std::string(encodedData.constData(), encodedData.length()),
                 "text/html;charset=UTF-8",
                 GURL(baseUrl.toString().toStdString()));
}

bool WebViewAdapter::isInitialized() {
  return priv->GetWebContents() != NULL;
}

int WebViewAdapter::getNavigationEntryCount() const {
  return priv->GetNavigationEntryCount();
}

int WebViewAdapter::getNavigationCurrentEntryIndex() const {
  return priv->GetNavigationCurrentEntryIndex();
}

void WebViewAdapter::setNavigationCurrentEntryIndex(int index) {
  priv->SetNavigationCurrentEntryIndex(index);
}

int WebViewAdapter::getNavigationEntryUniqueID(int index) const {
  return priv->GetNavigationEntryUniqueID(index);
}

QUrl WebViewAdapter::getNavigationEntryUrl(int index) const {
  return QUrl(QString::fromStdString(priv->GetNavigationEntryUrl(index).spec()));
}

QString WebViewAdapter::getNavigationEntryTitle(int index) const {
  return QString::fromStdString(priv->GetNavigationEntryTitle(index));
}

QDateTime WebViewAdapter::getNavigationEntryTimestamp(int index) const {
  return QDateTime::fromMSecsSinceEpoch(priv->GetNavigationEntryTimestamp(index).ToJsTime());
}

OxideQWebPreferences* WebViewAdapter::preferences() {
  WebPreferences* prefs =
      static_cast<WebPreferences *>(priv->GetWebPreferences());
  if (!prefs) {
    OxideQWebPreferences* p = new OxideQWebPreferences(adapterToQObject(this));
    prefs = OxideQWebPreferencesPrivate::get(p)->preferences();
    priv->SetWebPreferences(prefs);
  } else if (!prefs->api_handle()) {
    OxideQWebPreferencesPrivate::Adopt(prefs, adapterToQObject(this));
  }

  return prefs->api_handle();
}

void WebViewAdapter::setPreferences(OxideQWebPreferences* prefs) {
  OxideQWebPreferences* old = NULL;
  if (WebPreferences* o =
      static_cast<WebPreferences *>(priv->GetWebPreferences())) {
    old = o->api_handle();
  }
 
  if (prefs && !prefs->parent()) { 
    prefs->setParent(adapterToQObject(this));
  }

  WebPreferences* p = NULL;
  if (prefs) {
    p = OxideQWebPreferencesPrivate::get(prefs)->preferences();
  }
  priv->SetWebPreferences(p);

  if (!old) {
    return;
  }

  if (old->parent() == adapterToQObject(this)) {
    delete old;
  }
}

void WebViewAdapter::setRequest(OxideQNewViewRequest* request) {
  if (isInitialized()) {
    qWarning() << "Cannot assign NewViewRequest to an already constructed WebView";
    return;
  }

  if (created_with_new_view_request_) {
    return;
  }

  OxideQNewViewRequestPrivate* rd = OxideQNewViewRequestPrivate::get(request);
  if (rd->view) {
    qWarning() << "Cannot assign NewViewRequest to more than one WebView";
    return;
  }

  rd->view = priv->AsWeakPtr();
  created_with_new_view_request_ = true;
}

void WebViewAdapter::updateWebPreferences() {
  priv->UpdateWebPreferences();
}

QPointF WebViewAdapter::scrollOffset() const {
  const gfx::Vector2dF& offset = priv->GetRootScrollOffset();
  return QPointF(offset.x(), offset.y());
}

QSizeF WebViewAdapter::layerSize() const {
  const gfx::SizeF& size = priv->GetRootLayerSize();
  return QSizeF(size.width(), size.height());
}

QSizeF WebViewAdapter::viewportSize() const {
  const gfx::SizeF& size = priv->GetViewportSize();
  return QSizeF(size.width(), size.height());
}

} // namespace qt
} // namespace oxide
