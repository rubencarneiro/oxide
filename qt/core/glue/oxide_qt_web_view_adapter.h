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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_VIEW_ADAPTER_H_
#define _OXIDE_QT_CORE_GLUE_WEB_VIEW_ADAPTER_H_

#include <QDateTime>
#include <QList>
#include <QRect>
#include <QScopedPointer>
#include <QString>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_adapter_base.h"

QT_BEGIN_NAMESPACE
class QSize;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class MessageHandlerAdapter;
class RenderWidgetHostViewDelegate;
class WebContextAdapter;
class WebFrameAdapter;
class WebPopupMenuDelegate;
class WebViewAdapterPrivate;

class Q_DECL_EXPORT WebViewAdapter : public AdapterBase {
 public:
  virtual ~WebViewAdapter();

  void init(WebContextAdapter* context,
            const QSize& initial_size,
            bool incognito,
            const QUrl& initial_url,
            bool visible);

  QUrl url() const;
  void setUrl(const QUrl& url);

  QString title() const;

  bool canGoBack() const;
  bool canGoForward() const;

  bool incognito() const;

  bool loading() const;

  WebFrameAdapter* rootFrame() const;

  void updateSize(const QSize& size);
  void updateVisibility(bool visible);

  void goBack();
  void goForward();
  void stop();
  void reload();

  QList<MessageHandlerAdapter *>& message_handlers() {
    return message_handlers_;
  }

  virtual RenderWidgetHostViewDelegate* CreateRenderWidgetHostViewDelegate() = 0;
  virtual WebPopupMenuDelegate* CreateWebPopupMenuDelegate() = 0;

  virtual void URLChanged() = 0;
  virtual void TitleChanged() = 0;
  virtual void CommandsUpdated() = 0;

  virtual void RootFrameChanged() = 0;

  virtual void LoadStarted(const QUrl& url) = 0;
  virtual void LoadStopped(const QUrl& url) = 0;
  virtual void LoadFailed(const QUrl& url,
                          int error_code,
                          const QString& error_description) = 0;
  virtual void LoadSucceeded(const QUrl& url) = 0;

  virtual void NavigationEntryCommitted() = 0;
  virtual void NavigationListPruned(bool from_front, int count) = 0;
  virtual void NavigationEntryChanged(int index) = 0;

  virtual WebFrameAdapter* CreateWebFrame() = 0;

  virtual QRect GetContainerBounds() = 0;

  void shutdown();

  bool isInitialized();

  int getNavigationEntryCount() const;
  int getNavigationCurrentEntryIndex() const;
  void setNavigationCurrentEntryIndex(int index);
  int getNavigationEntryUniqueID(int index) const;
  QUrl getNavigationEntryUrl(int index) const;
  QString getNavigationEntryTitle(int index) const;
  QDateTime getNavigationEntryTimestamp(int index) const;

 protected:
  WebViewAdapter();

 private:
  QList<MessageHandlerAdapter *> message_handlers_;
  QScopedPointer<WebViewAdapterPrivate> priv;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_VIEW_ADAPTER_H_
