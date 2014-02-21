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

#include "base/logging.h"
#include "ui/gfx/size.h"
#include "url/gurl.h"

#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/core/api/oxideqwebpreferences_p.h"
#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/glue/private/oxide_qt_web_view_adapter_p.h"

#include "oxide_qt_web_context_adapter_p.h"
#include "oxide_qt_web_frame_adapter.h"

namespace oxide {
namespace qt {

WebViewAdapter::WebViewAdapter(QObject* q) :
    AdapterBase(q),
    priv(WebViewAdapterPrivate::Create(this)) {
  setPreferences(new OxideQWebPreferences(adapterToQObject(this)));
}

WebViewAdapter::~WebViewAdapter() {}

void WebViewAdapter::init(WebContextAdapter* context,
                          const QSize& initial_size,
                          bool incognito,
                          const QUrl& initial_url,
                          bool visible) {
  if (!priv->Init(
          WebContextAdapterPrivate::get(context)->context(),
          incognito,
          gfx::Size(initial_size.width(), initial_size.height()))) {
    return;
  }

  if (!initial_url.isEmpty()) {
    priv->SetURL(GURL(initial_url.toString().toStdString()));
  }

  updateVisibility(visible);
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
  return priv->IsIncognito();
}

bool WebViewAdapter::loading() const {
  return priv->IsLoading();
}

WebFrameAdapter* WebViewAdapter::rootFrame() const {
  return static_cast<WebFrame *>(priv->GetRootFrame())->adapter();
}

void WebViewAdapter::updateSize(const QSize& size) {
  priv->UpdateSize(gfx::Size(size.width(), size.height()));
}

void WebViewAdapter::updateVisibility(bool visible) {
  if (visible) {
    priv->Shown();
  } else {
    priv->Hidden();
  }
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
  if (!priv->GetWebPreferences()) {
    setPreferences(new OxideQWebPreferences(adapterToQObject(this)));
  }
  return static_cast<WebPreferences *>(priv->GetWebPreferences())->api_handle();
}

void WebViewAdapter::setPreferences(OxideQWebPreferences* prefs) {
  OxideQWebPreferences* old = NULL;
  if (WebPreferences* o =
      static_cast<WebPreferences *>(priv->GetWebPreferences())) {
    old = o->api_handle();
  }

  if (!prefs) {
    prefs = new OxideQWebPreferences(adapterToQObject(this));
  }
  if (!prefs->parent()) {
    prefs->setParent(adapterToQObject(this));
  }
  priv->SetWebPreferences(
      &OxideQWebPreferencesPrivate::get(prefs)->preferences);

  if (!old) {
    return;
  }

  if (!OxideQWebPreferencesPrivate::get(old)->in_destructor() &&
      old->parent() == adapterToQObject(this)) {
    delete old;
  }
}

void WebViewAdapter::WebPreferencesChanged() {
  if (!priv->GetWebPreferences()) {
    setPreferences(new OxideQWebPreferences(adapterToQObject(this)));
  } else if (isInitialized()) {
    OnWebPreferencesChanged();
  }
}

} // namespace qt
} // namespace oxide
