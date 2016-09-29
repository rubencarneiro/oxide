// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2016 Canonical Ltd.

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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_H_
#define _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_H_

#include <QByteArray>
#include <QDateTime>
#include <QImage>
#include <QList>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QtGlobal>
#include <QUrl>

#include "qt/core/glue/oxide_qt_proxy_base.h"

class OxideQFindController;
class OxideQNewViewRequest;
class OxideQSecurityStatus;
class OxideQWebPreferences;

namespace oxide {
namespace qt {

class ContentsViewProxyClient;
class ScriptMessageHandlerProxy;
class WebContextProxy;
class WebFrameProxy;
class WebViewProxyClient;
class WebView;

enum ContentTypeFlags {
  CONTENT_TYPE_NONE = 0,
  CONTENT_TYPE_MIXED_DISPLAY = 1 << 0,
  CONTENT_TYPE_MIXED_SCRIPT = 1 << 1
};

enum RestoreType {
  RESTORE_LAST_SESSION_EXITED_CLEANLY,
  RESTORE_LAST_SESSION_CRASHED,
  RESTORE_CURRENT_SESSION,
};

enum LocationBarMode {
  LOCATION_BAR_MODE_AUTO,
  LOCATION_BAR_MODE_SHOWN,
  LOCATION_BAR_MODE_HIDDEN
};

enum WebProcessStatus {
  WEB_PROCESS_RUNNING,
  WEB_PROCESS_KILLED,
  WEB_PROCESS_CRASHED
};

enum EditCapabilityFlags {
  NO_CAPABILITY = 0,
  UNDO_CAPABILITY = 1 << 0,
  REDO_CAPABILITY = 1 << 1,
  CUT_CAPABILITY = 1 << 2,
  COPY_CAPABILITY = 1 << 3,
  PASTE_CAPABILITY = 1 << 4,
  ERASE_CAPABILITY = 1 << 5,
  SELECT_ALL_CAPABILITY = 1 << 6
};

enum EditingCommands {
  EDITING_COMMAND_UNDO,
  EDITING_COMMAND_REDO,
  EDITING_COMMAND_CUT,
  EDITING_COMMAND_COPY,
  EDITING_COMMAND_PASTE,
  EDITING_COMMAND_ERASE,
  EDITING_COMMAND_SELECT_ALL
};

class Q_DECL_EXPORT WebViewProxy : public ProxyBase<WebView> {
 public:
  static WebViewProxy* create(
      WebViewProxyClient* client, // Must outlive returned proxy
      ContentsViewProxyClient* view_client, // Must outlive returned proxy
      QObject* handle,
      OxideQFindController* find_controller, // Returned proxy must outlive this
      OxideQSecurityStatus* security_status, // Returned proxy must outlive this
      QObject* context,
      bool incognito,
      const QByteArray& restore_state,
      RestoreType restore_type);
  static WebViewProxy* create(WebViewProxyClient* client,
                              ContentsViewProxyClient* view_client,
                              QObject* handle,
                              OxideQFindController* find_controller,
                              OxideQSecurityStatus* security_status,
                              OxideQNewViewRequest* new_view_request,
                              OxideQWebPreferences* initial_prefs);

  virtual ~WebViewProxy();

  virtual QUrl url() const = 0;
  virtual void setUrl(const QUrl& url) = 0;

  virtual QString title() const = 0;

  virtual QUrl favIconUrl() const = 0;

  virtual bool canGoBack() const = 0;
  virtual bool canGoForward() const = 0;

  virtual bool incognito() const = 0;

  virtual bool loading() const = 0;

  virtual bool fullscreen() const = 0;
  virtual void setFullscreen(bool fullscreen) = 0;

  virtual QObject* rootFrame() const = 0;

  virtual QObject* context() const = 0;

  virtual void goBack() = 0;
  virtual void goForward() = 0;
  virtual void stop() = 0;
  virtual void reload() = 0;

  virtual void loadHtml(const QString& html, const QUrl& base_url) = 0;

  virtual QList<QObject*>& messageHandlers() = 0;

  virtual int getNavigationEntryCount() const = 0;
  virtual int getNavigationCurrentEntryIndex() const = 0;
  virtual void setNavigationCurrentEntryIndex(int index) = 0;
  virtual int getNavigationEntryUniqueID(int index) const = 0;
  virtual QUrl getNavigationEntryUrl(int index) const = 0;
  virtual QString getNavigationEntryTitle(int index) const = 0;
  virtual QDateTime getNavigationEntryTimestamp(int index) const = 0;

  virtual QByteArray currentState() const = 0;

  virtual OxideQWebPreferences* preferences() = 0;
  virtual void setPreferences(OxideQWebPreferences* prefs) = 0;

  virtual void syncWebPreferences() = 0;

  virtual QPoint compositorFrameScrollOffset() = 0;
  virtual QSize compositorFrameContentSize() = 0;
  virtual QSize compositorFrameViewportSize() = 0;

  virtual void setCanTemporarilyDisplayInsecureContent(bool allow) = 0;
  virtual void setCanTemporarilyRunInsecureContent(bool allow) = 0;;

  virtual ContentTypeFlags blockedContent() const = 0;

  virtual void prepareToClose() = 0;

  virtual int locationBarHeight() const = 0;
  virtual void setLocationBarHeight(int height) = 0;
  virtual int locationBarOffset() const = 0;
  virtual int locationBarContentOffset() const = 0;
  virtual LocationBarMode locationBarMode() const = 0;
  virtual void setLocationBarMode(LocationBarMode mode) = 0;
  virtual bool locationBarAnimated() const = 0;
  virtual void setLocationBarAnimated(bool animated) = 0;
  virtual void locationBarShow(bool animate) = 0;
  virtual void locationBarHide(bool animate) = 0;

  virtual WebProcessStatus webProcessStatus() const = 0;

  virtual void executeEditingCommand(EditingCommands command) const = 0;

  virtual QUrl targetUrl() const = 0;

  virtual EditCapabilityFlags editFlags() const = 0;

  virtual qreal zoomFactor() const = 0;
  virtual void setZoomFactor(qreal factor) = 0;
  static qreal minimumZoomFactor();
  static qreal maximumZoomFactor();

  virtual void teardownFrameTree() = 0;

  virtual void killWebProcess(bool crash) = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_H_
