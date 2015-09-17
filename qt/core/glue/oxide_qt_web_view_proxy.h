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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_H_
#define _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_H_

#include <QByteArray>
#include <QDateTime>
#include <QImage>
#include <QList>
#include <QPoint>
#include <QRect>
#include <QSharedPointer>
#include <QSize>
#include <QString>
#include <QtGlobal>
#include <Qt>
#include <QUrl>
#include <QVariant>

#include "qt/core/glue/oxide_qt_proxy_handle.h"

typedef void* EGLImageKHR;

QT_BEGIN_NAMESPACE
class QFocusEvent;
class QHoverEvent;
class QInputMethodEvent;
class QKeyEvent;
class QMouseEvent;
class QTouchEvent;
class QWheelEvent;
QT_END_NAMESPACE

class OxideQFindController;
class OxideQNewViewRequest;
class OxideQSecurityStatus;
class OxideQWebPreferences;

namespace oxide {
namespace qt {

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
  RESTORE_CURRENT_SESSION,
  RESTORE_LAST_SESSION_EXITED_CLEANLY,
  RESTORE_LAST_SESSION_CRASHED,
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

enum EditingCommands {
  EDITING_COMMAND_UNDO,
  EDITING_COMMAND_REDO,
  EDITING_COMMAND_CUT,
  EDITING_COMMAND_COPY,
  EDITING_COMMAND_PASTE,
  EDITING_COMMAND_ERASE,
  EDITING_COMMAND_SELECT_ALL
};

class CompositorFrameHandle {
 public:
  virtual ~CompositorFrameHandle() {}

  enum Type {
    TYPE_INVALID,
    TYPE_SOFTWARE,
    TYPE_ACCELERATED,
    TYPE_IMAGE
  };

  virtual Type GetType() = 0;
  virtual const QRect& GetRect() const = 0;

  virtual QImage GetSoftwareFrame() = 0;
  virtual unsigned int GetAcceleratedFrameTexture() = 0;
  virtual EGLImageKHR GetImageFrame() = 0;
};

struct FindInPageState {
  int request_id;
  QString text;
  bool case_sensitive;
  int current;
  int count;
};

OXIDE_Q_DECL_PROXY_HANDLE(ScriptMessageHandlerProxy);
OXIDE_Q_DECL_PROXY_HANDLE(WebContextProxy);
OXIDE_Q_DECL_PROXY_HANDLE(WebFrameProxy);

class Q_DECL_EXPORT WebViewProxy {
  OXIDE_Q_DECL_PROXY_FOR(WebView);
 public:
  static WebViewProxy* create(WebViewProxyClient* client,
                              OxideQFindController* find_controller,
                              OxideQSecurityStatus* security_status,
                              WebContextProxyHandle* context,
                              bool incognito,
                              const QByteArray& restore_state,
                              RestoreType restore_type);
  static WebViewProxy* create(WebViewProxyClient* client,
                              OxideQFindController* find_controller,
                              OxideQSecurityStatus* security_status,
                              OxideQNewViewRequest* new_view_request);

  virtual ~WebViewProxy();

  static void createHelpers(
      QScopedPointer<OxideQFindController>* find_controller,
      QScopedPointer<OxideQSecurityStatus>* security_status);

  virtual QUrl url() const = 0;
  virtual void setUrl(const QUrl& url) = 0;

  virtual QString title() const = 0;

  virtual bool canGoBack() const = 0;
  virtual bool canGoForward() const = 0;

  virtual bool incognito() const = 0;

  virtual bool loading() const = 0;

  virtual bool fullscreen() const = 0;
  virtual void setFullscreen(bool fullscreen) = 0;

  virtual WebFrameProxyHandle* rootFrame() const = 0;

  virtual WebContextProxyHandle* context() const = 0;

  virtual void wasResized() = 0;
  virtual void screenUpdated() = 0;
  virtual void visibilityChanged() = 0;

  virtual void handleFocusEvent(QFocusEvent* event) = 0;
  virtual void handleHoverEvent(QHoverEvent* event,
                                const QPoint& window_pos,
                                const QPoint& global_pos) = 0;
  virtual void handleInputMethodEvent(QInputMethodEvent* event) = 0;
  virtual void handleKeyEvent(QKeyEvent* event) = 0;
  virtual void handleMouseEvent(QMouseEvent* event) = 0;
  virtual void handleTouchEvent(QTouchEvent* event) = 0;
  virtual void handleWheelEvent(QWheelEvent* event,
                                const QPoint& window_pos) = 0;

  virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const = 0;

  virtual void goBack() = 0;
  virtual void goForward() = 0;
  virtual void stop() = 0;
  virtual void reload() = 0;

  virtual void loadHtml(const QString& html, const QUrl& base_url) = 0;

  virtual QList<ScriptMessageHandlerProxyHandle*>& messageHandlers() = 0;

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

  virtual void updateWebPreferences() = 0;

  virtual QPoint compositorFrameScrollOffsetPix() = 0;
  virtual QSize compositorFrameContentSizePix() = 0;
  virtual QSize compositorFrameViewportSizePix() = 0;

  virtual QSharedPointer<CompositorFrameHandle> compositorFrameHandle() = 0;
  virtual void didCommitCompositorFrame() = 0;

  virtual void setCanTemporarilyDisplayInsecureContent(bool allow) = 0;
  virtual void setCanTemporarilyRunInsecureContent(bool allow) = 0;;

  virtual ContentTypeFlags blockedContent() const = 0;

  virtual void prepareToClose() = 0;

  virtual int locationBarHeight() = 0;
  virtual void setLocationBarHeight(int height) = 0;
  virtual int locationBarOffsetPix() = 0;
  virtual int locationBarContentOffsetPix() = 0;
  virtual LocationBarMode locationBarMode() const = 0;
  virtual void setLocationBarMode(LocationBarMode mode) = 0;
  virtual bool locationBarAnimated() const = 0;
  virtual void setLocationBarAnimated(bool animated) = 0;
  virtual void locationBarShow(bool animate) = 0;
  virtual void locationBarHide(bool animate) = 0;

  virtual WebProcessStatus webProcessStatus() const = 0;

  virtual void executeEditingCommand(EditingCommands command) const = 0;
};

OXIDE_Q_DECL_PROXY_HANDLE(WebViewProxy);

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_H_
