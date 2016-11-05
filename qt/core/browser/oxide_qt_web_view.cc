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

#include "oxide_qt_web_view.h"

#include <deque>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include <QGuiApplication>
#include <QInputEvent>
#include <QScreen>
#include <QString>
#include <QtDebug>
#include <QUrl>

#include "base/logging.h"
#include "base/macros.h"
#include "base/pickle.h"
#include "base/strings/utf_string_conversions.h"
#include "cc/output/compositor_frame_metadata.h"
#include "content/public/browser/host_zoom_map.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/restore_type.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/page_zoom.h"
#include "net/base/net_errors.h"
#include "third_party/WebKit/public/platform/WebDragOperation.h"
#include "ui/display/display.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"
#include "url/gurl.h"

#include "qt/core/api/oxideqdownloadrequest.h"
#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqhttpauthenticationrequest_p.h"
#include "qt/core/api/oxideqnavigationrequest.h"
#include "qt/core/api/oxideqnewviewrequest.h"
#include "qt/core/api/oxideqnewviewrequest_p.h"
#include "qt/core/api/oxideqpermissionrequest.h"
#include "qt/core/api/oxideqpermissionrequest_p.h"
#include "qt/core/api/oxideqcertificateerror.h"
#include "qt/core/api/oxideqcertificateerror_p.h"
#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/core/api/oxideqwebpreferences_p.h"
#include "qt/core/glue/macros.h"
#include "qt/core/glue/oxide_qt_contents_view_proxy_client.h"
#include "qt/core/glue/oxide_qt_web_frame_proxy_client.h"
#include "qt/core/glue/oxide_qt_web_view_proxy_client.h"
#include "qt/core/glue/web_context_menu.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_content_types.h"
#include "shared/browser/oxide_fullscreen_helper.h"
#include "shared/browser/oxide_render_widget_host_view.h"
#include "shared/browser/oxide_web_contents_view.h"
#include "shared/browser/oxide_web_frame.h"
#include "shared/browser/oxide_web_frame_tree.h"
#include "shared/browser/oxide_web_view.h"
#include "shared/browser/permissions/oxide_permission_request.h"
#include "shared/browser/permissions/oxide_permission_request_dispatcher.h"
#include "shared/browser/ssl/oxide_certificate_error.h"
#include "shared/browser/ssl/oxide_certificate_error_dispatcher.h"
#include "shared/browser/web_contents_helper.h"
#include "shared/common/oxide_enum_flags.h"

#include "oxide_qt_contents_view.h"
#include "oxide_qt_dpi_utils.h"
#include "oxide_qt_event_utils.h"
#include "oxide_qt_file_picker.h"
#include "oxide_qt_javascript_dialog.h"
#include "oxide_qt_screen_utils.h"
#include "oxide_qt_script_message_handler.h"
#include "oxide_qt_type_conversions.h"
#include "oxide_qt_web_context.h"
#include "oxide_qt_web_frame.h"
#include "qt_web_context_menu.h"
#include "web_contents_id_tracker.h"
#include "web_preferences.h"

namespace oxide {
namespace qt {

using oxide::CertificateErrorDispatcher;
using oxide::FullscreenHelper;
using oxide::PermissionRequestDispatcher;
using oxide::WebContentsUniquePtr;
using oxide::WebFrameTreeObserver;
using oxide::WebFrameTree;
using oxide::WebProcessStatusMonitor;

namespace {

OxideQLoadEvent::ErrorDomain ErrorDomainFromErrorCode(int error_code) {
  if (error_code == net::OK) {
    return OxideQLoadEvent::ErrorDomainNone;
  }
  if (net::IsCertificateError(error_code)) {
    return OxideQLoadEvent::ErrorDomainCertificate;
  }
  if (-1 >= error_code && error_code > -100) {
    return OxideQLoadEvent::ErrorDomainInternal;
  }
  if (-100 >= error_code && error_code > -200) {
    return OxideQLoadEvent::ErrorDomainConnection;
  }
  if (-200 >= error_code && error_code > -300) {
    return OxideQLoadEvent::ErrorDomainCertificate;
  }
  if (-300 >= error_code && error_code > -400) {
    return OxideQLoadEvent::ErrorDomainHTTP;
  }
  if (-400 >= error_code && error_code > -500) {
    return OxideQLoadEvent::ErrorDomainCache;
  }
  if (-600 >= error_code && error_code > -700) {
    return OxideQLoadEvent::ErrorDomainFTP;
  }
  if (-800 >= error_code && error_code > -900) {
    return OxideQLoadEvent::ErrorDomainDNS;
  }

  return OxideQLoadEvent::ErrorDomainInternal;
}

static const char* STATE_SERIALIZER_MAGIC_NUMBER = "oxide";
static uint16_t STATE_SERIALIZER_VERSION = 1;

void CreateRestoreEntriesFromRestoreState(
    const QByteArray& state,
    std::vector<sessions::SerializedNavigationEntry>* entries_out,
    int* index_out) {
#define WARN_INVALID_DATA \
    qWarning() << "Failed to read initial state: invalid data"
  std::vector<sessions::SerializedNavigationEntry> entries;
  base::Pickle pickle(state.data(), state.size());
  base::PickleIterator i(pickle);
  std::string magic_number;
  if (!i.ReadString(&magic_number)) {
    WARN_INVALID_DATA;
    return;
  }
  if (magic_number != STATE_SERIALIZER_MAGIC_NUMBER) {
    WARN_INVALID_DATA;
    return;
  }
  uint16_t version;
  if (!i.ReadUInt16(&version)) {
    WARN_INVALID_DATA;
    return;
  }
  if (version != STATE_SERIALIZER_VERSION) {
    WARN_INVALID_DATA;
    return;
  }
  int count;
  if (!i.ReadLength(&count)) {
    WARN_INVALID_DATA;
    return;
  }
  entries.resize(count);
  for (int j = 0; j < count; ++j) {
    sessions::SerializedNavigationEntry entry;
    if (!entry.ReadFromPickle(&i)) {
      WARN_INVALID_DATA;
      return;
    }
    entries[j] = entry;
  }
  int index;
  if (!i.ReadInt(&index)) {
    WARN_INVALID_DATA; 
    return;
  }
#undef WARN_INVALID_DATA

  entries_out->swap(entries);
  *index_out = index;
}

content::RestoreType ToContentRestoreType(RestoreType type) {
  static_assert(
      RESTORE_CURRENT_SESSION == static_cast<RestoreType>(
          content::RestoreType::CURRENT_SESSION),
      "RestoreType and content::RestoreType don't "
      "match: RESTORE_CURRENT_SESSION");
  static_assert(
      RESTORE_LAST_SESSION_EXITED_CLEANLY == static_cast<RestoreType>(
          content::RestoreType::LAST_SESSION_EXITED_CLEANLY),
      "RestoreType and content::RestoreType don't "
      "match: RESTORE_LAST_SESSION_EXITED_CLEANLY");
  static_assert(
      RESTORE_LAST_SESSION_CRASHED == static_cast<RestoreType>(
          content::RestoreType::LAST_SESSION_CRASHED),
      "RestoreType and content::RestoreType don't "
      "match: RESTORE_LAST_SESSION_CRASHED");

  return static_cast<content::RestoreType>(type);
}

bool TeardownFrameTreeForEachHelper(std::deque<oxide::WebFrame*>* d,
                                    oxide::WebFrame* frame) {
  d->push_back(frame);
  return true;
}

}

WebView::WebView(WebViewProxyClient* client,
                 ContentsViewProxyClient* view_client,
                 QObject* handle)
    : contents_view_(new ContentsView(view_client, handle)),
      client_(client),
      frame_tree_torn_down_(false) {
  DCHECK(client);
  DCHECK(handle);

  setHandle(handle);
}

void WebView::CommonInit() {
  content::WebContents* contents = web_view_->GetWebContents();

  // base::Unretained is safe here because we disconnect in the destructor
  CertificateErrorDispatcher::FromWebContents(contents)->SetCallback(
      base::Bind(&WebView::OnCertificateError,
                 base::Unretained(this)));

  FullscreenHelper::FromWebContents(contents)->set_client(this);
  PermissionRequestDispatcher::FromWebContents(contents)->set_client(this);
  WebFrameTreeObserver::Observe(WebFrameTree::FromWebContents(contents));

  track_zoom_subscription_ =
      content::HostZoomMap::GetForWebContents(contents)
          ->AddZoomLevelChangedCallback(base::Bind(&WebView::OnZoomLevelChanged,
                                                   base::Unretained(this)));
  web_process_status_subscription_ =
      WebProcessStatusMonitor::FromWebContents(contents)->AddChangeCallback(
          base::Bind(&WebView::OnWebProcessStatusChanged,
                     base::Unretained(this)));

  CHECK_EQ(web_view_->GetRootFrame()->GetChildFrames().size(), 0U);
  WebFrame* root_frame = new WebFrame(web_view_->GetRootFrame());
  web_view_->GetRootFrame()->set_script_message_target_delegate(root_frame);
  client_->CreateWebFrame(root_frame);
}

void WebView::OnZoomLevelChanged(
    const content::HostZoomMap::ZoomLevelChange& change) {
  client_->ZoomLevelChanged();
}

void WebView::OnWebProcessStatusChanged() {
  client_->WebProcessStatusChanged();
}

oxide::JavaScriptDialog* WebView::CreateJavaScriptDialog(
    content::JavaScriptMessageType javascript_message_type) {
  JavaScriptDialogProxyClient::Type type;
  switch (javascript_message_type) {
  case content::JAVASCRIPT_MESSAGE_TYPE_ALERT:
    type = JavaScriptDialogProxyClient::TypeAlert;
    break;
  case content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
    type = JavaScriptDialogProxyClient::TypeConfirm;
    break;
  case content::JAVASCRIPT_MESSAGE_TYPE_PROMPT:
    type = JavaScriptDialogProxyClient::TypePrompt;
    break;
  default:
    Q_UNREACHABLE();
  }

  JavaScriptDialog* dialog = new JavaScriptDialog();
  dialog->SetProxy(client_->CreateJavaScriptDialog(type, dialog));
  return dialog;
}

oxide::JavaScriptDialog* WebView::CreateBeforeUnloadDialog() {
  JavaScriptDialog* dialog = new JavaScriptDialog();
  dialog->SetProxy(client_->CreateBeforeUnloadDialog(dialog));
  return dialog;
}

void WebView::URLChanged() {
  client_->URLChanged();
}

void WebView::TitleChanged() {
  client_->TitleChanged();
}

void WebView::FaviconChanged() {
  client_->FaviconChanged();
}

void WebView::LoadingChanged() {
  client_->LoadingChanged();
}

void WebView::LoadProgressChanged(double progress) {
  client_->LoadProgressChanged(progress);
}

void WebView::LoadStarted(const GURL& validated_url) {
  OxideQLoadEvent event =
      OxideQLoadEvent::createStarted(
        QUrl(QString::fromStdString(validated_url.spec())));
  client_->LoadEvent(event);
}

void WebView::LoadRedirected(const GURL& url,
                             const GURL& original_url,
                             int http_status_code) {
  OxideQLoadEvent event =
      OxideQLoadEvent::createRedirected(
        QUrl(QString::fromStdString(url.spec())),
        QUrl(QString::fromStdString(original_url.spec())),
        http_status_code);
  client_->LoadEvent(event);
}

void WebView::LoadCommitted(const GURL& url,
                            bool is_error_page,
                            int http_status_code) {
  OxideQLoadEvent event =
      OxideQLoadEvent::createCommitted(
        QUrl(QString::fromStdString(url.spec())),
        is_error_page,
        http_status_code);
  client_->LoadEvent(event);
}

void WebView::LoadStopped(const GURL& validated_url) {
  OxideQLoadEvent event =
      OxideQLoadEvent::createStopped(
        QUrl(QString::fromStdString(validated_url.spec())));
  client_->LoadEvent(event);
}

void WebView::LoadFailed(const GURL& validated_url,
                         int error_code,
                         const std::string& error_description,
                         int http_status_code) {
  OxideQLoadEvent event =
      OxideQLoadEvent::createFailed(
        QUrl(QString::fromStdString(validated_url.spec())),
        ErrorDomainFromErrorCode(error_code),
        QString::fromStdString(error_description),
        error_code,
        http_status_code);
  client_->LoadEvent(event);
}

void WebView::LoadSucceeded(const GURL& validated_url, int http_status_code) {
  OxideQLoadEvent event =
      OxideQLoadEvent::createSucceeded(
        QUrl(QString::fromStdString(validated_url.spec())),
        http_status_code);
  client_->LoadEvent(event);
}

bool WebView::AddMessageToConsole(
    int level,
    const base::string16& message,
    int line_no,
    const base::string16& source_id) {
  client_->AddMessageToConsole(
      level,
      QString::fromStdString(base::UTF16ToUTF8(message)),
      line_no,
      QString::fromStdString(base::UTF16ToUTF8(source_id)));
  return true;
}

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(FrameMetadataChangeFlags)

void WebView::FrameMetadataUpdated(const cc::CompositorFrameMetadata& old) {
  FrameMetadataChangeFlags flags = FRAME_METADATA_CHANGE_NONE;

  if (old.root_scroll_offset.x() !=
          web_view_->compositor_frame_metadata().root_scroll_offset.x() ||
      old.root_scroll_offset.y() !=
          web_view_->compositor_frame_metadata().root_scroll_offset.y()) {
    flags |= FRAME_METADATA_CHANGE_SCROLL_OFFSET;
  }
  if (old.root_layer_size.width() !=
          web_view_->compositor_frame_metadata().root_layer_size.width() ||
      old.root_layer_size.height() !=
          web_view_->compositor_frame_metadata().root_layer_size.height()) {
    flags |= FRAME_METADATA_CHANGE_CONTENT;
  }
  if (old.scrollable_viewport_size.width() !=
          web_view_->compositor_frame_metadata().scrollable_viewport_size.width() ||
      old.scrollable_viewport_size.height() !=
          web_view_->compositor_frame_metadata().scrollable_viewport_size.height()) {
    flags |= FRAME_METADATA_CHANGE_VIEWPORT;
  }
  if (old.device_scale_factor !=
          web_view_->compositor_frame_metadata().device_scale_factor ||
      old.page_scale_factor !=
          web_view_->compositor_frame_metadata().page_scale_factor) {
    flags |= FRAME_METADATA_CHANGE_SCROLL_OFFSET;
    flags |= FRAME_METADATA_CHANGE_CONTENT;
    flags |= FRAME_METADATA_CHANGE_VIEWPORT;
  }

  client_->FrameMetadataUpdated(flags);
}

oxide::FilePicker* WebView::CreateFilePicker(content::RenderFrameHost* rfh) {
  FilePicker* picker = new FilePicker(rfh);
  picker->SetProxy(client_->CreateFilePicker(picker));
  return picker;
}

void WebView::ContentBlocked() {
  client_->ContentBlocked();
}

void WebView::PrepareToCloseResponseReceived(bool proceed) {
  client_->PrepareToCloseResponse(proceed);
}

void WebView::CloseRequested() {
  client_->CloseRequested();
}

void WebView::TargetURLChanged() {
  client_->TargetURLChanged();
}

void WebView::OnEditingCapabilitiesChanged() {
  client_->OnEditingCapabilitiesChanged();
}

bool WebView::ShouldHandleNavigation(const GURL& url, bool user_gesture) {
  OxideQNavigationRequest request(QUrl(QString::fromStdString(url.spec())),
                                  OxideQNavigationRequest::DispositionCurrentTab,
                                  user_gesture);

  client_->NavigationRequested(&request);

  return request.action() == OxideQNavigationRequest::ActionAccept;
}

bool WebView::CanCreateWindows() {
  return client_->CanCreateWindows();
}

bool WebView::ShouldCreateNewWebContents(const GURL& url,
                                         WindowOpenDisposition disposition,
                                         bool user_gesture) {
  OxideQNavigationRequest::Disposition d = OxideQNavigationRequest::DispositionNewWindow;

  switch (disposition) {
    case WindowOpenDisposition::NEW_FOREGROUND_TAB:
      d = OxideQNavigationRequest::DispositionNewForegroundTab;
      break;
    case WindowOpenDisposition::NEW_BACKGROUND_TAB:
      d = OxideQNavigationRequest::DispositionNewBackgroundTab;
      break;
    case WindowOpenDisposition::NEW_POPUP:
      d = OxideQNavigationRequest::DispositionNewPopup;
      break;
    case WindowOpenDisposition::NEW_WINDOW:
      d = OxideQNavigationRequest::DispositionNewWindow;
      break;
    default:
      NOTREACHED();
  }

  OxideQNavigationRequest request(QUrl(QString::fromStdString(url.spec())),
                                  d, user_gesture);

  client_->NavigationRequested(&request);

  return request.action() == OxideQNavigationRequest::ActionAccept;
}

bool WebView::AdoptNewWebContents(const gfx::Rect& initial_pos,
                                  WindowOpenDisposition disposition,
                                  oxide::WebContentsUniquePtr contents) {
  OxideQNewViewRequest::Disposition d = OxideQNewViewRequest::DispositionNewWindow;

  switch (disposition) {
    case WindowOpenDisposition::NEW_FOREGROUND_TAB:
      d = OxideQNewViewRequest::DispositionNewForegroundTab;
      break;
    case WindowOpenDisposition::NEW_BACKGROUND_TAB:
      d = OxideQNewViewRequest::DispositionNewBackgroundTab;
      break;
    case WindowOpenDisposition::NEW_POPUP:
      d = OxideQNewViewRequest::DispositionNewPopup;
      break;
    case WindowOpenDisposition::NEW_WINDOW:
      d = OxideQNewViewRequest::DispositionNewWindow;
      break;
    default:
      NOTREACHED();
  }

  OxideQNewViewRequest request(QRect(initial_pos.x(),
                                     initial_pos.y(),
                                     initial_pos.width(),
                                     initial_pos.height()),
                               d);
  OxideQNewViewRequestPrivate::get(&request)->contents = std::move(contents);

  client_->NewViewRequested(&request);

  return OxideQNewViewRequestPrivate::get(&request)->view.get() != nullptr;
}

void WebView::DownloadRequested(const GURL& url,
                                const std::string& mime_type,
                                const bool should_prompt,
                                const base::string16& suggested_filename,
                                const std::string& cookies,
                                const std::string& referrer,
                                const std::string& user_agent) {
  OxideQDownloadRequest download_request(
      QUrl(QString::fromStdString(url.spec())),
      QString::fromStdString(mime_type),
      should_prompt,
      QString::fromStdString(base::UTF16ToUTF8(suggested_filename)),
      QString::fromStdString(cookies),
      QString::fromStdString(referrer),
      QString::fromStdString(user_agent));

  client_->DownloadRequested(download_request);
}

void WebView::HttpAuthenticationRequested(
        oxide::ResourceDispatcherHostLoginDelegate* login_delegate) {
  // The client takes ownership of the request
  client_->HttpAuthenticationRequested(
      OxideQHttpAuthenticationRequestPrivate::Create(login_delegate));
}

std::unique_ptr<oxide::WebContextMenu> WebView::CreateContextMenu(
    const content::ContextMenuParams& params,
    oxide::WebContextMenuClient* client) {
  std::unique_ptr<WebContextMenuImpl> menu =
      base::MakeUnique<WebContextMenuImpl>(params, client);
  menu->Init(client_->CreateWebContextMenu(menu->GetParams(), menu.get()));
  return std::move(menu);
}

size_t WebView::GetScriptMessageHandlerCount() const {
  return message_handlers_.size();
}

const oxide::ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(
    size_t index) const {
  return ScriptMessageHandler::FromProxyHandle(
      message_handlers_.at(index))->handler();
}

void WebView::RequestGeolocationPermission(
    std::unique_ptr<oxide::PermissionRequest> request) {
  std::unique_ptr<OxideQGeolocationPermissionRequest> req(
      OxideQGeolocationPermissionRequestPrivate::Create(std::move(request)));

  // The embedder takes ownership of this
  client_->RequestGeolocationPermission(req.release());
}

void WebView::RequestNotificationPermission(
    std::unique_ptr<oxide::PermissionRequest> request) {
  std::unique_ptr<OxideQPermissionRequest> req(
      OxideQPermissionRequestPrivate::Create(std::move(request)));

  // The embedder takes ownership of this
  client_->RequestNotificationPermission(req.release());
}

void WebView::RequestMediaAccessPermission(
    std::unique_ptr<oxide::MediaAccessPermissionRequest> request) {
  std::unique_ptr<OxideQMediaAccessPermissionRequest> req(
      OxideQMediaAccessPermissionRequestPrivate::Create(std::move(request)));

  // The embedder takes ownership of this
  client_->RequestMediaAccessPermission(req.release());
}

void WebView::FrameCreated(oxide::WebFrame* frame) {
  DCHECK(!frame_tree_torn_down_);
  DCHECK(!WebFrame::FromSharedWebFrame(frame));
  DCHECK(frame->parent());

  WebFrame* f = new WebFrame(frame);
  frame->set_script_message_target_delegate(f);
  client_->CreateWebFrame(f);

  DCHECK(f->handle());

  WebFrame* parent = WebFrame::FromSharedWebFrame(frame->parent());
  parent->client()->ChildFramesChanged();
}

void WebView::FrameDeleted(oxide::WebFrame* frame) {
  WebFrame* f = WebFrame::FromSharedWebFrame(frame);
  DCHECK(f);

  client_->FrameRemoved(f->handle());
  frame->set_script_message_target_delegate(nullptr);

  f->client()->DestroyFrame();
  // |f| has been deleted

  if (!frame->parent()) {
    return;
  }

  WebFrame* parent = WebFrame::FromSharedWebFrame(frame->parent());
  DCHECK(parent);

  parent->client()->ChildFramesChanged();
}

void WebView::LoadCommittedInFrame(oxide::WebFrame* frame) {
  WebFrame* f = WebFrame::FromSharedWebFrame(frame);
  f->client()->LoadCommitted();
}

void WebView::OnCertificateError(
    std::unique_ptr<oxide::CertificateError> error) {
  std::unique_ptr<OxideQCertificateError> qerror =
      OxideQCertificateErrorPrivate::Create(std::move(error));

  client_->CertificateError(std::move(qerror));
}

void WebView::EnterFullscreenMode(const GURL& origin) {
  client_->ToggleFullscreenMode(true);
}

void WebView::ExitFullscreenMode() {
  client_->ToggleFullscreenMode(false);
}

WebContentsID WebView::webContentsID() const {
  return WebContentsIDTracker::GetInstance()->GetIDForWebContents(
      web_view_->GetWebContents());
}

QUrl WebView::url() const {
  return QUrl(QString::fromStdString(web_view_->GetURL().spec()));
}

void WebView::setUrl(const QUrl& url) {
  web_view_->SetURL(GURL(url.toString().toStdString()));
}

QString WebView::title() const {
  return QString::fromStdString(web_view_->GetTitle());
}

QUrl WebView::favIconUrl() const {
  return QUrl(QString::fromStdString(web_view_->GetFaviconURL().spec()));
}

bool WebView::canGoBack() const {
  return web_view_->CanGoBack();
}

bool WebView::canGoForward() const {
  return web_view_->CanGoForward();
}

bool WebView::incognito() const {
  return web_view_->IsIncognito();
}

bool WebView::loading() const {
  return web_view_->IsLoading();
}

bool WebView::fullscreen() const {
  return FullscreenHelper::FromWebContents(
      web_view_->GetWebContents())->fullscreen_granted();
}

void WebView::setFullscreen(bool fullscreen) {
  FullscreenHelper::FromWebContents(web_view_->GetWebContents())
      ->SetFullscreenGranted(fullscreen);
}

QObject* WebView::rootFrame() const {
  WebFrame* f = WebFrame::FromSharedWebFrame(web_view_->GetRootFrame());
  if (!f) {
    return nullptr;
  }

  return f->handle();
}

QObject* WebView::context() const {
  WebContext* c = GetContext();
  if (!c) {
    return nullptr;
  }

  return c->handle();
}

void WebView::goBack() {
  web_view_->GoBack();
}

void WebView::goForward() {
  web_view_->GoForward();
}

void WebView::stop() {
  web_view_->Stop();
}

void WebView::reload() {
  web_view_->Reload();
}

void WebView::loadHtml(const QString& html, const QUrl& base_url) {
  QByteArray encoded_data = html.toUtf8().toPercentEncoding();
  web_view_->LoadData(std::string(encoded_data.constData(), encoded_data.length()),
                  "text/html;charset=UTF-8",
                  GURL(base_url.toString().toStdString()));
}

QList<QObject*>& WebView::messageHandlers() {
  return message_handlers_;
}

QByteArray WebView::currentState() const {
  // XXX(chrisccoulson): Move the pickling in to oxide::WebView
  std::vector<sessions::SerializedNavigationEntry> entries = web_view_->GetState();
  if (entries.size() == 0) {
    return QByteArray();
  }
  base::Pickle pickle;
  pickle.WriteString(STATE_SERIALIZER_MAGIC_NUMBER);
  pickle.WriteUInt16(STATE_SERIALIZER_VERSION);
  pickle.WriteInt(entries.size());
  std::vector<sessions::SerializedNavigationEntry>::const_iterator i;
  static const size_t max_state_size =
      std::numeric_limits<uint16_t>::max() - 1024;
  for (i = entries.begin(); i != entries.end(); ++i) {
    i->WriteToPickle(max_state_size, &pickle);
  }
  pickle.WriteInt(
      web_view_->GetWebContents()->GetController().GetCurrentEntryIndex());
  return QByteArray(static_cast<const char*>(pickle.data()), pickle.size());
}

OxideQWebPreferences* WebView::preferences() {
  WebPreferences* prefs =
      WebPreferences::FromPrefs(
          web_view_->GetWebContentsHelper()->GetPreferences());
  return prefs ? prefs->api_handle() : nullptr;
}

void WebView::setPreferences(OxideQWebPreferences* prefs) {
  oxide::WebPreferences* p = nullptr;
  if (prefs) {
    p = OxideQWebPreferencesPrivate::get(prefs)->GetPrefs();
  }
  web_view_->GetWebContentsHelper()->SetPreferences(p);
}

void WebView::syncWebPreferences() {
  web_view_->GetWebContentsHelper()->SyncWebPreferences();
}

QPoint WebView::compositorFrameScrollOffset() {
  return ToQt(DpiUtils::ConvertChromiumPixelsToQt(
      web_view_->GetCompositorFrameScrollOffset(),
      contents_view_->GetScreen()));
}

QSize WebView::compositorFrameContentSize() {
  return ToQt(DpiUtils::ConvertChromiumPixelsToQt(
      web_view_->GetCompositorFrameContentSize(),
      contents_view_->GetScreen()));
}

QSize WebView::compositorFrameViewportSize() {
  return ToQt(DpiUtils::ConvertChromiumPixelsToQt(
      web_view_->GetCompositorFrameViewportSize(),
      contents_view_->GetScreen()));
}

void WebView::setCanTemporarilyDisplayInsecureContent(bool allow) {
  web_view_->SetCanTemporarilyDisplayInsecureContent(allow);
}

void WebView::setCanTemporarilyRunInsecureContent(bool allow) {
  web_view_->SetCanTemporarilyRunInsecureContent(allow);
}

ContentTypeFlags WebView::blockedContent() const {
  static_assert(
      CONTENT_TYPE_NONE ==
        static_cast<ContentTypeFlags>(oxide::CONTENT_TYPE_NONE),
      "ContentTypeFlags and oxide::ContentType enums don't match: "
      "CONTENT_TYPE_NONE");
  static_assert(
      CONTENT_TYPE_MIXED_DISPLAY ==
        static_cast<ContentTypeFlags>(oxide::CONTENT_TYPE_MIXED_DISPLAY),
      "ContentTypeFlags and oxide::ContentType enums don't match: "
      "CONTENT_TYPE_MIXED_DISPLAY");
  static_assert(
      CONTENT_TYPE_MIXED_SCRIPT ==
        static_cast<ContentTypeFlags>(oxide::CONTENT_TYPE_MIXED_SCRIPT),
      "ContentTypeFlags and oxide::ContentType enums don't match: "
      "CONTENT_TYPE_MIXED_SCRIPT");

  return static_cast<ContentTypeFlags>(web_view_->blocked_content());
}

void WebView::prepareToClose() {
  web_view_->PrepareToClose();
}

void WebView::terminateWebProcess() {
  web_view_->TerminateWebProcess();
}

WebProcessStatus WebView::webProcessStatus() const {
  STATIC_ASSERT_MATCHING_ENUM(WEB_PROCESS_RUNNING,
                              WebProcessStatusMonitor::Status::Running);
  STATIC_ASSERT_MATCHING_ENUM(WEB_PROCESS_KILLED,
                              WebProcessStatusMonitor::Status::Killed);
  STATIC_ASSERT_MATCHING_ENUM(WEB_PROCESS_CRASHED,
                              WebProcessStatusMonitor::Status::Crashed);
  STATIC_ASSERT_MATCHING_ENUM(WEB_PROCESS_UNRESPONSIVE,
                              WebProcessStatusMonitor::Status::Unresponsive);

  return static_cast<WebProcessStatus>(
      WebProcessStatusMonitor::FromWebContents(web_view_->GetWebContents())
          ->GetStatus());
}

void WebView::executeEditingCommand(EditingCommands command) const {
  content::WebContents* contents = web_view_->GetWebContents();

  switch (command) {
    case EDITING_COMMAND_UNDO:
      return contents->Undo();
    case EDITING_COMMAND_REDO:
      return contents->Redo();
    case EDITING_COMMAND_CUT:
      return contents->Cut();
    case EDITING_COMMAND_COPY:
      return contents->Copy();
    case EDITING_COMMAND_PASTE:
      return contents->Paste();
    case EDITING_COMMAND_ERASE:
      return contents->Delete();
    case EDITING_COMMAND_SELECT_ALL:
      return contents->SelectAll();
    default:
      NOTREACHED();
  }
}

QUrl WebView::targetUrl() const {
  return QUrl(QString::fromStdString(web_view_->target_url().spec()));
}

EditCapabilityFlags WebView::editFlags() const {
  EditCapabilityFlags caps;
  blink::WebContextMenuData::EditFlags flags = web_view_->GetEditFlags();
  if (flags & blink::WebContextMenuData::CanUndo) {
    caps |= EDIT_CAPABILITY_UNDO;
  }
  if (flags & blink::WebContextMenuData::CanRedo) {
    caps |= EDIT_CAPABILITY_REDO;
  }
  if (flags & blink::WebContextMenuData::CanCut) {
    caps |= EDIT_CAPABILITY_CUT;
  }
  if (flags & blink::WebContextMenuData::CanCopy) {
    caps |= EDIT_CAPABILITY_COPY;
  }
  if (flags & blink::WebContextMenuData::CanPaste) {
    caps |= EDIT_CAPABILITY_PASTE;
  }
  if (flags & blink::WebContextMenuData::CanDelete) {
    caps |= EDIT_CAPABILITY_ERASE;
  }
  if (flags & blink::WebContextMenuData::CanSelectAll) {
    caps |= EDIT_CAPABILITY_SELECT_ALL;
  }
  return caps;
}

qreal WebView::zoomFactor() const {
  return content::ZoomLevelToZoomFactor(
      content::HostZoomMap::GetZoomLevel(web_view_->GetWebContents()));
}

void WebView::setZoomFactor(qreal factor) {
  if (factor < content::kMinimumZoomFactor ||
      factor > content::kMaximumZoomFactor) {
    return;
  }

  content::HostZoomMap::SetZoomLevel(
      web_view_->GetWebContents(),
      content::ZoomFactorToZoomLevel(static_cast<double>(factor)));
}

void WebView::teardownFrameTree() {
  DCHECK(!frame_tree_torn_down_);

  WebFrameTreeObserver::Observe(nullptr);

  std::deque<oxide::WebFrame*> frames;
  WebFrameTree::FromWebContents(web_view_->GetWebContents())->ForEachFrame(
      base::Bind(&TeardownFrameTreeForEachHelper, &frames));
  while (frames.size() > 0) {
    oxide::WebFrame* frame = frames.back();
    frames.pop_back();

    FrameDeleted(frame);
  }

  frame_tree_torn_down_ = true;
}

WebView::WebView(WebViewProxyClient* client,
                 ContentsViewProxyClient* view_client,
                 QObject* handle,
                 WebContext* context,
                 bool incognito,
                 const QByteArray& restore_state,
                 RestoreType restore_type)
    : WebView(client, view_client, handle) {
  oxide::WebView::CommonParams common_params;
  common_params.client = this;
  common_params.view_client = contents_view_.get();
  common_params.contents_client = this;

  oxide::WebView::CreateParams create_params;
  create_params.context = context->GetContext();
  create_params.incognito = incognito;

  if (!restore_state.isEmpty()) {
    CreateRestoreEntriesFromRestoreState(restore_state,
                                         &create_params.restore_entries,
                                         &create_params.restore_index);
    create_params.restore_type = ToContentRestoreType(restore_type);
  }

  if (oxide::BrowserProcessMain::GetInstance()->GetProcessModel() ==
          oxide::PROCESS_MODEL_SINGLE_PROCESS) {
    DCHECK(!incognito);
  }

  web_view_.reset(new oxide::WebView(common_params, create_params));

  CommonInit();
}

// static
WebView* WebView::CreateFromNewViewRequest(
    WebViewProxyClient* client,
    ContentsViewProxyClient* view_client,
    QObject* handle,
    OxideQNewViewRequest* new_view_request,
    OxideQWebPreferences* initial_prefs) {
  OxideQNewViewRequestPrivate* rd =
      OxideQNewViewRequestPrivate::get(new_view_request);
  if (rd->view) {
    return nullptr;
  }

  WebView* new_view = new WebView(client, view_client, handle);

  oxide::WebView::CommonParams params;
  params.client = new_view;
  params.view_client = new_view->contents_view_.get();
  params.contents_client = new_view;
  new_view->web_view_.reset(new oxide::WebView(params, std::move(rd->contents)));

  rd->view = new_view->web_view_->AsWeakPtr();

  new_view->CommonInit();

  if (initial_prefs) {
    oxide::WebPreferences* p =
        new_view->web_view_->GetWebContentsHelper()->GetPreferences();
    DCHECK(!WebPreferences::FromPrefs(p));

    OxideQWebPreferencesPrivate::get(initial_prefs)->AdoptPrefs(p);
  }

  return new_view;
}

WebView::~WebView() {
  content::WebContents* contents = web_view_->GetWebContents();
  CertificateErrorDispatcher::FromWebContents(contents)->SetCallback(
      CertificateErrorDispatcher::Callback());
  FullscreenHelper::FromWebContents(contents)->set_client(nullptr);
  DCHECK(frame_tree_torn_down_);

  oxide::PermissionRequestDispatcher::FromWebContents(
      contents)->set_client(nullptr);
}

// static
WebView* WebView::FromView(oxide::WebView* view) {
  return static_cast<WebView*>(view->client());
}

WebContext* WebView::GetContext() const {
  return WebContext::FromBrowserContext(web_view_->GetBrowserContext());
}

} // namespace qt
} // namespace oxide
