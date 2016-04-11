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

#ifndef _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_CLIENT_H_
#define _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_CLIENT_H_

#include <memory>
#include <QRect>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_javascript_dialog_proxy_client.h"

class OxideQCertificateError;
class OxideQDownloadRequest;
class OxideQGeolocationPermissionRequest;
class OxideQHttpAuthenticationRequest;
class OxideQLoadEvent;
class OxideQMediaAccessPermissionRequest;
class OxideQNavigationRequest;
class OxideQNewViewRequest;
class OxideQPermissionRequest;

QT_BEGIN_NAMESPACE
class QKeyEvent;
class QObject;
class QString;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class FilePickerProxy;
class FilePickerProxyClient;
class JavaScriptDialogProxy;
class WebFrameProxy;

enum FrameMetadataChangeFlags {
  FRAME_METADATA_CHANGE_NONE = 0,

  FRAME_METADATA_CHANGE_SCROLL_OFFSET = 1 << 0,
  FRAME_METADATA_CHANGE_CONTENT = 1 << 1,
  FRAME_METADATA_CHANGE_VIEWPORT = 1 << 2,
  FRAME_METADATA_CHANGE_CONTROLS_OFFSET = 1 << 3,
  FRAME_METADATA_CHANGE_CONTENT_OFFSET = 1 << 4
};

class WebViewProxyClient {
 public:
  virtual ~WebViewProxyClient() {}

  virtual JavaScriptDialogProxy* CreateJavaScriptDialog(
      JavaScriptDialogProxyClient::Type type,
      JavaScriptDialogProxyClient* client) = 0;
  virtual JavaScriptDialogProxy* CreateBeforeUnloadDialog(
      JavaScriptDialogProxyClient* client) = 0;
  virtual FilePickerProxy* CreateFilePicker(FilePickerProxyClient* client) = 0;

  virtual void WebProcessStatusChanged() = 0;

  virtual void URLChanged() = 0;
  virtual void TitleChanged() = 0;
  virtual void FaviconChanged() = 0;
  virtual void CommandsUpdated() = 0;
  virtual void LoadingChanged() = 0;
  virtual void LoadProgressChanged(double progress) = 0;
  virtual void LoadEvent(const OxideQLoadEvent& event) = 0;

  virtual void NavigationEntryCommitted() = 0;
  virtual void NavigationListPruned(bool from_front, int count) = 0;
  virtual void NavigationEntryChanged(int index) = 0;

  virtual void CreateWebFrame(WebFrameProxy* proxy) = 0;

  virtual void AddMessageToConsole(int level,
                                   const QString& message,
                                   int line_no,
                                   const QString& source_id) = 0;

  virtual void ToggleFullscreenMode(bool enter) = 0;

  virtual void WebPreferencesReplaced() = 0;

  virtual void FrameRemoved(QObject* frame) = 0;

  virtual bool CanCreateWindows() const = 0;

  virtual void NavigationRequested(OxideQNavigationRequest* request) = 0;
  virtual void NewViewRequested(OxideQNewViewRequest* request) = 0;

  virtual void RequestGeolocationPermission(
      OxideQGeolocationPermissionRequest* request) = 0;
  virtual void RequestMediaAccessPermission(
      OxideQMediaAccessPermissionRequest* request) = 0;
  virtual void RequestNotificationPermission(
      OxideQPermissionRequest* request) = 0;

  virtual void FrameMetadataUpdated(FrameMetadataChangeFlags flags) = 0;

  virtual void DownloadRequested(
      const OxideQDownloadRequest& download_request) = 0;

  virtual void HttpAuthenticationRequested(
      OxideQHttpAuthenticationRequest* authentication_request) = 0;

  virtual void CertificateError(
      std::unique_ptr<OxideQCertificateError> cert_error) = 0;

  virtual void ContentBlocked() = 0; // XXX(chrisccoulson): Rename to BlockedContentChanged throughout Oxide

  virtual void PrepareToCloseResponse(bool proceed) = 0;
  virtual void CloseRequested() = 0;

  virtual void TargetURLChanged() = 0;

  virtual void OnEditingCapabilitiesChanged() = 0;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_WEB_VIEW_PROXY_CLIENT_H_
