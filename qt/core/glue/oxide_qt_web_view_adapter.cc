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

#include "ui/gfx/size.h"
#include "url/gurl.h"

#include "qt/core/browser/oxide_qt_web_frame.h"
#include "qt/core/glue/private/oxide_qt_web_context_adapter_p.h"
#include "qt/core/glue/private/oxide_qt_web_view_adapter_p.h"

#include "oxide_qt_web_frame_adapter.h"

namespace oxide {
namespace qt {

WebViewAdapter::WebViewAdapter() :
    priv(WebViewAdapterPrivate::Create(this)) {}

WebViewAdapter::~WebViewAdapter() {}

void WebViewAdapter::init(WebContextAdapter* context,
                          const QSize& initial_size,
                          bool incognito,
                          const QUrl& initial_url,
                          bool visible) {
  if (!priv->Init(
          WebContextAdapterPrivate::get(context)->GetContext(),
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
  WebFrame* frame = static_cast<WebFrame *>(priv->GetRootFrame());
  if (!frame) {
    return NULL;
  }

  return frame->adapter;
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

void WebViewAdapter::shutdown() {
  priv->Shutdown();
}

bool WebViewAdapter::isInitialized() {
  return priv->web_contents() != NULL;
}

} // namespace qt
} // namespace oxide
