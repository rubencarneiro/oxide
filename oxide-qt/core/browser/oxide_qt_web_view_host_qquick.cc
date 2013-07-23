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

#include "oxide_qt_web_view_host_qquick.h"

#include <QPointF>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSize>
#include <QSizeF>

#include "base/logging.h"
#include "content/public/browser/web_contents.h"
#include "ui/gfx/rect.h"
#include "ui/gfx/size.h"

#include "oxide/browser/oxide_web_contents_view.h"

#include "oxide_qt_render_widget_host_view_qquick.h"
#include "oxide_qt_web_view_host_delegate.h"

QT_USE_NAMESPACE

namespace oxide {
namespace qt {

WebViewHostQQuick::WebViewHostQQuick(QQuickItem* container,
                                     WebViewHostDelegate* delegate) :
    container_(container),
    delegate_(delegate) {}

void WebViewHostQQuick::OnURLChanged() {
  delegate_->OnURLChanged();
}

void WebViewHostQQuick::OnTitleChanged() {
  delegate_->OnTitleChanged();
}

void WebViewHostQQuick::OnLoadingChanged() {
  delegate_->OnLoadingChanged();
}

void WebViewHostQQuick::OnCommandsUpdated() {
  delegate_->OnCommandsUpdated();
}

WebViewHostQQuick::~WebViewHostQQuick() {}

WebViewHostQQuick* WebViewHostQQuick::Create(QQuickItem* container,
                                             WebViewHostDelegate* delegate,
                                             bool incognito,
                                             const QSizeF& initial_size,
                                             bool visible) {
  CHECK(container && delegate) <<
      "Invalid parameters to WebViewHostQQuick::Create";

  WebViewHostQQuick* wvh = new WebViewHostQQuick(container, delegate);
  if (!wvh->Init(incognito,
                 gfx::Size(initial_size.toSize().width(),
                           initial_size.toSize().height()))) {
    delete wvh;
    return NULL;
  }

  static_cast<oxide::WebContentsView *>(
      wvh->web_contents()->GetView())->SetDelegate(wvh);

  visible ? wvh->WasShown() : wvh->WasHidden();

  return wvh;
}

void WebViewHostQQuick::UpdateSize(const QSizeF& size) {
  oxide::WebViewHost::UpdateSize(gfx::Size(size.toSize().width(),
                                           size.toSize().height()));
}

content::RenderWidgetHostView* WebViewHostQQuick::CreateViewForWidget(
    content::RenderWidgetHost* render_widget_host) {
  return new RenderWidgetHostViewQQuick(render_widget_host, container_);
}

gfx::Rect WebViewHostQQuick::GetContainerBounds() {
  QPointF pos(container_->mapToScene(QPointF(0,0)));
  pos += QPointF(container_->window()->x(), container_->window()->y());

  return gfx::Rect(qRound(pos.x()),
                   qRound(pos.y()),
                   qRound(container_->width()),
                   qRound(container_->height()));
}

} // namespace qt
} // namespace oxide
