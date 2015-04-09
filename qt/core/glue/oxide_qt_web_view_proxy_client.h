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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_CLIENT_H_
#define _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_CLIENT_H_

#include <QRect>
#include <QUrl>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_javascript_dialog_proxy_client.h"
#include "qt/core/glue/oxide_qt_proxy_handle.h"

class OxideQCertificateError;
class OxideQDownloadRequest;
class OxideQGeolocationPermissionRequest;
class OxideQLoadEvent;
class OxideQNavigationRequest;
class OxideQNewViewRequest;

QT_BEGIN_NAMESPACE
class QCursor;
class QKeyEvent;
class QObject;
class QScreen;
class QString;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class FilePickerProxy;
class FilePickerProxyClient;
class JavaScriptDialogProxy;
class WebFrameProxy;
class WebPopupMenuProxy;
class WebPopupMenuProxyClient;

enum FrameMetadataChangeFlags {
  FRAME_METADATA_CHANGE_NONE = 0,

  FRAME_METADATA_CHANGE_SCROLL_OFFSET = 1 << 0,
  FRAME_METADATA_CHANGE_CONTENT = 1 << 1,
  FRAME_METADATA_CHANGE_VIEWPORT = 1 << 2,
  FRAME_METADATA_CHANGE_CONTROLS_OFFSET = 1 << 3,
  FRAME_METADATA_CHANGE_CONTENT_OFFSET = 1 << 4
};

OXIDE_Q_DECL_PROXY_HANDLE(WebFrameProxy);

class WebViewProxyClient {
 public:
  virtual ~WebViewProxyClient() {}

  virtual void Initialized() = 0;

  virtual QObject* GetApiHandle() = 0;

  virtual WebPopupMenuProxy* CreateWebPopupMenu(
      WebPopupMenuProxyClient* client) = 0;
  virtual JavaScriptDialogProxy* CreateJavaScriptDialog(
      JavaScriptDialogProxyClient::Type type,
      JavaScriptDialogProxyClient* client) = 0;
  virtual JavaScriptDialogProxy* CreateBeforeUnloadDialog(
      JavaScriptDialogProxyClient* client) = 0;
  virtual FilePickerProxy* CreateFilePicker(FilePickerProxyClient* client) = 0;

  virtual void URLChanged() = 0;
  virtual void TitleChanged() = 0;
  virtual void IconChanged(QUrl icon) = 0; // XXX(chrisccoulson): Move paramter to a member on WebView
  virtual void CommandsUpdated() = 0;
  virtual void LoadingChanged() = 0;
  virtual void LoadProgressChanged(double progress) = 0;
  virtual void LoadEvent(OxideQLoadEvent* event) = 0;

  virtual void NavigationEntryCommitted() = 0;
  virtual void NavigationListPruned(bool from_front, int count) = 0;
  virtual void NavigationEntryChanged(int index) = 0;

  virtual WebFrameProxyHandle* CreateWebFrame(WebFrameProxy* proxy) = 0;

  virtual QScreen* GetScreen() const = 0;
  virtual QRect GetViewBoundsPix() const = 0;
  virtual bool IsVisible() const = 0;
  virtual bool HasFocus() const = 0;

  virtual void AddMessageToConsole(int level,
                                   const QString& message,
                                   int line_no,
                                   const QString& source_id) = 0;

  virtual void ToggleFullscreenMode(bool enter) = 0;

  virtual void WebPreferencesReplaced() = 0;

  virtual void FrameAdded(WebFrameProxyHandle* frame) = 0;
  virtual void FrameRemoved(WebFrameProxyHandle* frame) = 0;

  virtual bool CanCreateWindows() const = 0;

  virtual void UpdateCursor(const QCursor& cursor) = 0;

  virtual void NavigationRequested(OxideQNavigationRequest* request) = 0;
  virtual void NewViewRequested(OxideQNewViewRequest* request) = 0;

  virtual void RequestGeolocationPermission(
      OxideQGeolocationPermissionRequest* request) = 0;

  virtual void HandleUnhandledKeyboardEvent(QKeyEvent* event) = 0;

  virtual void FrameMetadataUpdated(FrameMetadataChangeFlags flags) = 0;
  virtual void ScheduleUpdate() = 0;
  virtual void EvictCurrentFrame() = 0;

  virtual void SetInputMethodEnabled(bool enabled) = 0;

  virtual void DownloadRequested(OxideQDownloadRequest* download_request) = 0;

  virtual void CertificateError(OxideQCertificateError* cert_error) = 0;

  virtual void ContentBlocked() = 0; // XXX(chrisccoulson): Rename to BlockedContentChanged throughout Oxide

  virtual void PrepareToCloseResponse(bool proceed) = 0;
  virtual void CloseRequested() = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_CLIENT_H_
