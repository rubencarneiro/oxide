// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxide_qt_web_view_proxy.h"

#include "base/logging.h"
#include "content/public/common/page_zoom.h"
#include "qt/core/browser/oxide_qt_web_context.h"
#include "qt/core/browser/oxide_qt_web_view.h"

namespace oxide {
namespace qt {

// static
WebViewProxy* WebViewProxy::create(WebViewProxyClient* client,
                                   ContentsViewClient* view_client,
                                   QObject* handle,
                                   QObject* context,
                                   bool incognito,
                                   const QByteArray& restore_state,
                                   RestoreType restore_type) {
  CHECK(context);

  return new WebView(client,
                     view_client,
                     handle,
                     WebContext::FromProxyHandle(context),
                     incognito,
                     restore_state,
                     restore_type);
}

// static
WebViewProxy* WebViewProxy::create(WebViewProxyClient* client,
                                   ContentsViewClient* view_client,
                                   QObject* handle,
                                   OxideQNewViewRequest* new_view_request,
                                   OxideQWebPreferences* initial_prefs) {
  return WebView::CreateFromNewViewRequest(client,
                                           view_client,
                                           handle,
                                           new_view_request,
                                           initial_prefs);
}

WebViewProxy::~WebViewProxy() {}

// static
qreal WebViewProxy::minimumZoomFactor() {
  return content::kMinimumZoomFactor;
}

// static
qreal WebViewProxy::maximumZoomFactor() {
  return content::kMaximumZoomFactor;
}

} // namespace qt
} // namespace oxide
