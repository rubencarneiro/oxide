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

#ifndef _OXIDE_QT_CORE_BROWSER_WEB_VIEW_HOST_QQUICK_H_
#define _OXIDE_QT_CORE_BROWSER_WEB_VIEW_HOST_QQUICK_H_

#include <QtGlobal>
#include <QUrl>

#include "base/basictypes.h"
#include "base/compiler_specific.h"

#include "oxide/browser/oxide_web_contents_view_delegate.h"
#include "oxide/browser/oxide_web_view_host.h"
#include "oxide/browser/oxide_browser_process_handle.h"

#include "oxide-qt/core/common/oxide_qt_content_main_delegate.h"

QT_BEGIN_NAMESPACE
class QQuickItem;
class QSizeF;
QT_END_NAMESPACE

QT_USE_NAMESPACE

namespace oxide {
namespace qt {

class WebViewHostDelegate;

class OXIDE_EXPORT WebViewHostQQuick FINAL :
    public oxide::WebViewHost,
    public oxide::WebContentsViewDelegate {
 public:
  ~WebViewHostQQuick();

  static WebViewHostQQuick* Create(QQuickItem* container,
                                   WebViewHostDelegate* delegate,
                                   bool incognito,
                                   const QSizeF& initial_size,
                                   bool visible);

  QUrl GetURL() const;
  void SetURL(const QUrl& url);

  void UpdateSize(const QSizeF& size);

  content::RenderWidgetHostView* CreateViewForWidget(
      content::RenderWidgetHost* render_widget_host) FINAL;

  gfx::Rect GetContainerBounds() FINAL;

 private:
  WebViewHostQQuick(QQuickItem* container,
                    WebViewHostDelegate* delegate);


  void OnURLChanged() FINAL;
  void OnTitleChanged() FINAL;
  void OnLoadingChanged() FINAL;
  void OnCommandsUpdated() FINAL;

  BrowserProcessHandle<ContentMainDelegate> browser_handle_;
  QQuickItem* container_;
  WebViewHostDelegate* delegate_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(WebViewHostQQuick);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_WEB_VIEW_HOST_QQUICK_H_
