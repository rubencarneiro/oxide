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

#include "oxide_qt_web_view.h"

#include <vector>

#include <QCursor>
#include <QGuiApplication>
#include <QInputEvent>
#include <QInputMethod>
#include <QKeyEvent>
#include <QScreen>
#include <QString>
#include <QtDebug>
#include <QTextCharFormat>
#include <QUrl>
#include <QWindow>

#include "base/memory/scoped_vector.h"
#include "base/strings/utf_string_conversions.h"
#include "url/gurl.h"
#include "content/common/cursors/webcursor.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/navigation_controller.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "net/base/net_errors.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/WebKit/public/platform/WebColor.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/events/event.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/range/range.h"

#include "qt/core/api/oxideqdownloadrequest.h"
#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqnavigationrequest.h"
#include "qt/core/api/oxideqnewviewrequest.h"
#include "qt/core/api/oxideqnewviewrequest_p.h"
#include "qt/core/api/oxideqpermissionrequest.h"
#include "qt/core/api/oxideqpermissionrequest_p.h"
#include "qt/core/api/oxideqsecurityevents.h"
#include "qt/core/api/oxideqsecurityevents_p.h"
#include "qt/core/api/oxideqsecuritystatus.h"
#include "qt/core/api/oxideqsecuritystatus_p.h"
#include "qt/core/api/oxideqsslcertificate.h"
#include "qt/core/api/oxideqsslcertificate_p.h"
#include "qt/core/base/oxide_qt_event_utils.h"
#include "qt/core/base/oxide_qt_screen_utils.h"
#include "qt/core/base/oxide_qt_skutils.h"
#include "qt/core/glue/oxide_qt_script_message_handler_adapter_p.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter.h"
#include "qt/core/glue/oxide_qt_web_view_adapter.h"
#include "shared/base/oxide_enum_flags.h"
#include "shared/browser/oxide_render_widget_host_view.h"

#include "oxide_qt_file_picker.h"
#include "oxide_qt_javascript_dialog.h"
#include "oxide_qt_web_frame.h"
#include "oxide_qt_web_popup_menu.h"

namespace oxide {
namespace qt {

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

inline QCursor QCursorFromWebCursor(blink::WebCursorInfo::Type type) {
  Qt::CursorShape cs = Qt::ArrowCursor;
  switch (type) {
  case blink::WebCursorInfo::TypeCross:
    cs = Qt::CrossCursor;
    break;

  case blink::WebCursorInfo::TypeHand:
    cs = Qt::PointingHandCursor;
    break;

  case blink::WebCursorInfo::TypeCell:
  case blink::WebCursorInfo::TypeIBeam:
    cs = Qt::IBeamCursor;
    break;

  case blink::WebCursorInfo::TypeWait:
    cs = Qt::WaitCursor;
    break;

  case blink::WebCursorInfo::TypeHelp:
    cs = Qt::WhatsThisCursor;
    break;

  case blink::WebCursorInfo::TypeEastResize:
  case blink::WebCursorInfo::TypeWestResize:
  case blink::WebCursorInfo::TypeEastWestResize:
    cs = Qt::SizeHorCursor;
    break;

  case blink::WebCursorInfo::TypeNorthResize:
  case blink::WebCursorInfo::TypeSouthResize:
  case blink::WebCursorInfo::TypeNorthSouthResize:
    cs = Qt::SizeVerCursor;
    break;

  case blink::WebCursorInfo::TypeNorthEastResize:
  case blink::WebCursorInfo::TypeSouthWestResize:
    cs = Qt::SizeBDiagCursor;
    break;

  case blink::WebCursorInfo::TypeNorthWestResize:
  case blink::WebCursorInfo::TypeSouthEastResize:
    cs = Qt::SizeFDiagCursor;
    break;

  case blink::WebCursorInfo::TypeNorthEastSouthWestResize:
  case blink::WebCursorInfo::TypeNorthWestSouthEastResize:
  case blink::WebCursorInfo::TypeMove:
    cs = Qt::SizeAllCursor;
    break;

  case blink::WebCursorInfo::TypeColumnResize:
    cs = Qt::SplitHCursor;
    break;

  case blink::WebCursorInfo::TypeRowResize:
    cs = Qt::SplitVCursor;
    break;

  case blink::WebCursorInfo::TypeMiddlePanning:
  case blink::WebCursorInfo::TypeEastPanning:
  case blink::WebCursorInfo::TypeNorthPanning:
  case blink::WebCursorInfo::TypeNorthEastPanning:
  case blink::WebCursorInfo::TypeNorthWestPanning:
  case blink::WebCursorInfo::TypeSouthPanning:
  case blink::WebCursorInfo::TypeSouthEastPanning:
  case blink::WebCursorInfo::TypeSouthWestPanning:
  case blink::WebCursorInfo::TypeWestPanning:
  case blink::WebCursorInfo::TypeGrab:
  case blink::WebCursorInfo::TypeGrabbing:
    cs = Qt::ClosedHandCursor;
    break;

  case blink::WebCursorInfo::TypeProgress:
    cs = Qt::BusyCursor;
    break;

  case blink::WebCursorInfo::TypeNoDrop:
  case blink::WebCursorInfo::TypeNotAllowed:
    cs = Qt::ForbiddenCursor;
    break;

  case blink::WebCursorInfo::TypeCopy:
  case blink::WebCursorInfo::TypeContextMenu:
  case blink::WebCursorInfo::TypeVerticalText:
  case blink::WebCursorInfo::TypeAlias:
  case blink::WebCursorInfo::TypeZoomIn:
  case blink::WebCursorInfo::TypeZoomOut:
  case blink::WebCursorInfo::TypeCustom:
  case blink::WebCursorInfo::TypePointer:
  case blink::WebCursorInfo::TypeNone:
  default:
    break;
  }

  return QCursor(cs);
}

Qt::InputMethodHints QImHintsFromInputType(ui::TextInputType type) {
  switch (type) {
    case ui::TEXT_INPUT_TYPE_TEXT:
    case ui::TEXT_INPUT_TYPE_TEXT_AREA:
    case ui::TEXT_INPUT_TYPE_CONTENT_EDITABLE:
      return Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_PASSWORD:
      return Qt::ImhHiddenText | Qt::ImhSensitiveData |
          Qt::ImhNoAutoUppercase | Qt::ImhPreferLowercase |
          Qt::ImhNoPredictiveText;
    case ui::TEXT_INPUT_TYPE_SEARCH:
      return Qt::ImhNoAutoUppercase | Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_EMAIL:
      return Qt::ImhEmailCharactersOnly;
    case ui::TEXT_INPUT_TYPE_NUMBER:
      return Qt::ImhFormattedNumbersOnly;
    case ui::TEXT_INPUT_TYPE_TELEPHONE:
      return Qt::ImhDialableCharactersOnly;
    case ui::TEXT_INPUT_TYPE_URL:
      return Qt::ImhUrlCharactersOnly;
    case ui::TEXT_INPUT_TYPE_DATE:
    case ui::TEXT_INPUT_TYPE_MONTH:
    case ui::TEXT_INPUT_TYPE_WEEK:
      return Qt::ImhDate;
    case ui::TEXT_INPUT_TYPE_DATE_TIME:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_LOCAL:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_FIELD:
      return Qt::ImhDate | Qt::ImhTime;
    case ui::TEXT_INPUT_TYPE_TIME:
      return Qt::ImhTime;
    default:
      return Qt::ImhNone;
  }
}

}

WebView::WebView(WebViewAdapter* adapter) :
    adapter_(adapter),
    has_input_method_state_(false),
    qsecurity_status_(
        OxideQSecurityStatusPrivate::Create(this)) {}

float WebView::GetDeviceScaleFactor() const {
  QScreen* screen = adapter_->GetScreen();
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }

  return GetDeviceScaleFactorFromQScreen(screen);
}

bool WebView::ShouldShowInputPanel() const {
  if (text_input_type_ != ui::TEXT_INPUT_TYPE_NONE &&
      show_ime_if_needed_) {
    return true;
  }

  return false;
}

bool WebView::ShouldHideInputPanel() const {
  if (text_input_type_ == ui::TEXT_INPUT_TYPE_NONE &&
      !focused_node_is_editable_) {
    return true;
  }

  return false;
}

void WebView::SetInputPanelVisibility(bool visible) {
  adapter_->SetInputMethodEnabled(visible);

  if (!visible) {
    has_input_method_state_ = false;
  }

  if (QGuiApplication::inputMethod()->isVisible() == visible) {
    return;
  }

  QGuiApplication::inputMethod()->setVisible(visible);
}

void WebView::Init(oxide::WebView::Params* params) {
  oxide::WebView::Init(params);
  adapter_->Initialized();
}

void WebView::UpdateCursor(const content::WebCursor& cursor) {
  content::WebCursor::CursorInfo cursor_info;

  cursor.GetCursorInfo(&cursor_info);
  if (cursor.IsCustom()) {
    QImage::Format format =
        QImageFormatFromSkImageInfo(cursor_info.custom_image.info());
    if (format == QImage::Format_Invalid) {
      return;
    }
    QImage cursor_image((uchar*)cursor_info.custom_image.getPixels(),
                        cursor_info.custom_image.width(),
                        cursor_info.custom_image.height(),
                        cursor_info.custom_image.rowBytes(),
                        format);

    QPixmap cursor_pixmap;
    if (cursor_pixmap.convertFromImage(cursor_image)) {
      adapter_->UpdateCursor(QCursor(cursor_pixmap));
    }
  } else {
    adapter_->UpdateCursor(QCursorFromWebCursor(cursor_info.type));
  }
}

void WebView::ImeCancelComposition() {
  if (has_input_method_state_) {
    QGuiApplication::inputMethod()->reset();
  }
}

void WebView::SelectionChanged() {
  if (!HasFocus()) {
    return;
  }

  QGuiApplication::inputMethod()->update(
      static_cast<Qt::InputMethodQueries>(
        Qt::ImSurroundingText
        | Qt::ImCurrentSelection
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
        | Qt::ImTextBeforeCursor
        | Qt::ImTextAfterCursor
#endif
      ));
}

blink::WebScreenInfo WebView::GetScreenInfo() const {
  QScreen* screen = adapter_->GetScreen();
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }

  return GetWebScreenInfoFromQScreen(screen);
}

gfx::Rect WebView::GetContainerBoundsPix() const {
  QRect bounds = adapter_->GetContainerBoundsPix();
  return gfx::Rect(bounds.x(),
                   bounds.y(),
                   bounds.width(),
                   bounds.height());
}

bool WebView::IsVisible() const {
  return adapter_->IsVisible();
}

bool WebView::HasFocus() const {
  return adapter_->HasFocus();
}

oxide::JavaScriptDialog* WebView::CreateJavaScriptDialog(
    content::JavaScriptMessageType javascript_message_type,
    bool* did_suppress_message) {
  JavaScriptDialogDelegate::Type type;
  switch (javascript_message_type) {
  case content::JAVASCRIPT_MESSAGE_TYPE_ALERT:
    type = JavaScriptDialogDelegate::TypeAlert;
    break;
  case content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
    type = JavaScriptDialogDelegate::TypeConfirm;
    break;
  case content::JAVASCRIPT_MESSAGE_TYPE_PROMPT:
    type = JavaScriptDialogDelegate::TypePrompt;
    break;
  default:
    Q_UNREACHABLE();
  }
  JavaScriptDialogDelegate* delegate = adapter_->CreateJavaScriptDialogDelegate(type);
  return new JavaScriptDialog(delegate, did_suppress_message);
}

oxide::JavaScriptDialog* WebView::CreateBeforeUnloadDialog() {
  JavaScriptDialogDelegate* delegate = adapter_->CreateBeforeUnloadDialogDelegate();
  bool did_suppress_message = false;
  return new JavaScriptDialog(delegate, &did_suppress_message);
}

void WebView::FrameAdded(oxide::WebFrame* frame) {
  adapter_->FrameAdded(static_cast<WebFrame *>(frame)->adapter());
}

void WebView::FrameRemoved(oxide::WebFrame* frame) {
  adapter_->FrameRemoved(static_cast<WebFrame *>(frame)->adapter());
}

bool WebView::CanCreateWindows() const {
  return adapter_->CanCreateWindows();
}

size_t WebView::GetScriptMessageHandlerCount() const {
  return adapter_->message_handlers().size();
}

oxide::ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(
    size_t index) const {
  return &ScriptMessageHandlerAdapterPrivate::get(
      adapter_->message_handlers().at(index))->handler;
}

void WebView::OnURLChanged() {
  adapter_->URLChanged();
}

void WebView::OnTitleChanged() {
  adapter_->TitleChanged();
}

void WebView::OnIconChanged(const GURL& icon) {
  adapter_->IconChanged(QUrl(QString::fromStdString(icon.spec())));
}

void WebView::OnCommandsUpdated() {
  adapter_->CommandsUpdated();
}

void WebView::OnLoadProgressChanged(double progress) {
  adapter_->LoadProgressChanged(progress);
}

void WebView::OnLoadStarted(const GURL& validated_url,
                            bool is_error_frame) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStarted);
  adapter_->LoadEvent(&event);
}

void WebView::OnLoadStopped(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStopped);
  adapter_->LoadEvent(&event);
}

void WebView::OnLoadFailed(const GURL& validated_url,
                           int error_code,
                           const std::string& error_description) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeFailed,
      ErrorDomainFromErrorCode(error_code),
      QString::fromStdString(error_description),
      error_code);
  adapter_->LoadEvent(&event);
}

void WebView::OnLoadSucceeded(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeSucceeded);
  adapter_->LoadEvent(&event);
}

void WebView::OnNavigationEntryCommitted() {
  adapter_->NavigationEntryCommitted();
}

void WebView::OnNavigationListPruned(bool from_front, int count) {
  adapter_->NavigationListPruned(from_front, count);
}

void WebView::OnNavigationEntryChanged(int index) {
  adapter_->NavigationEntryChanged(index);
}

bool WebView::OnAddMessageToConsole(
    int level,
    const base::string16& message,
    int line_no,
    const base::string16& source_id) {
  adapter_->AddMessageToConsole(
      level,
      QString::fromStdString(base::UTF16ToUTF8(message)),
      line_no,
      QString::fromStdString(base::UTF16ToUTF8(source_id)));
  return true;
}

void WebView::OnToggleFullscreenMode(bool enter) {
  adapter_->ToggleFullscreenMode(enter);
}

void WebView::OnWebPreferencesDestroyed() {
  adapter_->WebPreferencesDestroyed();
}

void WebView::OnRequestGeolocationPermission(
    scoped_ptr<oxide::GeolocationPermissionRequest> request) {
  OxideQGeolocationPermissionRequest* qreq =
      new OxideQGeolocationPermissionRequest();
  OxideQPermissionRequestPrivate::get(qreq)->Init(
      request.PassAs<oxide::PermissionRequest>());

  // The embedder takes ownership of this
  adapter_->RequestGeolocationPermission(qreq);
}

void WebView::OnUnhandledKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {
  if (event.skip_in_browser) {
    return;
  }

  if (event.type != blink::WebInputEvent::RawKeyDown &&
      event.type != blink::WebInputEvent::KeyUp) {
    return;
  }

  DCHECK(event.os_event);

  QKeyEvent* qevent = reinterpret_cast<QKeyEvent *>(event.os_event);
  DCHECK(!qevent->isAccepted());

  adapter_->HandleUnhandledKeyboardEvent(qevent);
}

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(FrameMetadataChangeFlags)

void WebView::OnFrameMetadataUpdated(const cc::CompositorFrameMetadata& old) {
  FrameMetadataChangeFlags flags = FRAME_METADATA_CHANGE_NONE;

  if (old.device_scale_factor !=
      compositor_frame_metadata().device_scale_factor) {
    flags |= FRAME_METADATA_CHANGE_DEVICE_SCALE;
  }
  if (old.root_scroll_offset.x() !=
      compositor_frame_metadata().root_scroll_offset.x()) {
    flags |= FRAME_METADATA_CHANGE_SCROLL_OFFSET_X;
  }
  if (old.root_scroll_offset.y() !=
      compositor_frame_metadata().root_scroll_offset.y()) {
    flags |= FRAME_METADATA_CHANGE_SCROLL_OFFSET_Y;
  }
  if (old.root_layer_size.width() !=
      compositor_frame_metadata().root_layer_size.width()) {
    flags |= FRAME_METADATA_CHANGE_CONTENT_WIDTH;
  }
  if (old.root_layer_size.height() !=
      compositor_frame_metadata().root_layer_size.height()) {
    flags |= FRAME_METADATA_CHANGE_CONTENT_HEIGHT;
  }
  if (old.scrollable_viewport_size.width() !=
      compositor_frame_metadata().scrollable_viewport_size.width()) {
    flags |= FRAME_METADATA_CHANGE_VIEWPORT_WIDTH;
  }
  if (old.scrollable_viewport_size.height() !=
      compositor_frame_metadata().scrollable_viewport_size.height()) {
    flags |= FRAME_METADATA_CHANGE_VIEWPORT_HEIGHT;
  }
  if (old.page_scale_factor !=
      compositor_frame_metadata().page_scale_factor) {
    flags |= FRAME_METADATA_CHANGE_PAGE_SCALE;
  }

  adapter_->FrameMetadataUpdated(flags);
}

void WebView::OnDownloadRequested(const GURL& url,
                                  const std::string& mimeType,
                                  const bool shouldPrompt,
                                  const base::string16& suggestedFilename,
                                  const std::string& cookies,
                                  const std::string& referrer) {
  OxideQDownloadRequest downloadRequest(
      QUrl(QString::fromStdString(url.spec())),
      QString::fromStdString(mimeType),
      shouldPrompt,
      QString::fromStdString(base::UTF16ToUTF8(suggestedFilename)),
      QString::fromStdString(cookies),
      QString::fromStdString(referrer));

  adapter_->DownloadRequested(&downloadRequest);
}

bool WebView::ShouldHandleNavigation(const GURL& url,
                                     WindowOpenDisposition disposition,
                                     bool user_gesture) {
  OxideQNavigationRequest::Disposition d = OxideQNavigationRequest::DispositionNewWindow;

  switch (disposition) {
    case CURRENT_TAB:
      d = OxideQNavigationRequest::DispositionCurrentTab;
      break;
    case NEW_FOREGROUND_TAB:
      d = OxideQNavigationRequest::DispositionNewForegroundTab;
      break;
    case NEW_BACKGROUND_TAB:
      d = OxideQNavigationRequest::DispositionNewBackgroundTab;
      break;
    case NEW_POPUP:
      d = OxideQNavigationRequest::DispositionNewPopup;
      break;
    case NEW_WINDOW:
      d = OxideQNavigationRequest::DispositionNewWindow;
      break;
    default:
      NOTREACHED();
  }

  OxideQNavigationRequest request(QUrl(QString::fromStdString(url.spec())),
                                  d, user_gesture);

  adapter_->NavigationRequested(&request);

  return request.action() == OxideQNavigationRequest::ActionAccept;
}

oxide::WebFrame* WebView::CreateWebFrame(content::FrameTreeNode* node) {
  return new WebFrame(adapter_->CreateWebFrame(), node, this);
}

oxide::WebPopupMenu* WebView::CreatePopupMenu(content::RenderViewHost* rvh) {
  return new WebPopupMenu(adapter_->CreateWebPopupMenuDelegate(), rvh);
}

oxide::WebView* WebView::CreateNewWebView(const gfx::Rect& initial_pos,
                                          WindowOpenDisposition disposition) {
  OxideQNewViewRequest::Disposition d = OxideQNewViewRequest::DispositionNewWindow;

  switch (disposition) {
    case CURRENT_TAB:
      d = OxideQNewViewRequest::DispositionCurrentTab;
      break;
    case NEW_FOREGROUND_TAB:
      d = OxideQNewViewRequest::DispositionNewForegroundTab;
      break;
    case NEW_BACKGROUND_TAB:
      d = OxideQNewViewRequest::DispositionNewBackgroundTab;
      break;
    case NEW_POPUP:
      d = OxideQNewViewRequest::DispositionNewPopup;
      break;
    case NEW_WINDOW:
      d = OxideQNewViewRequest::DispositionNewWindow;
      break;
    default:
      NOTREACHED();
  }

  OxideQNewViewRequest request(QRect(initial_pos.x(),
                                     initial_pos.y(),
                                     initial_pos.width(),
                                     initial_pos.height()), d);

  adapter_->NewViewRequested(&request);

  WebView* view = OxideQNewViewRequestPrivate::get(&request)->view.get();
  if (!view) {
    qCritical() <<
        "Either a webview wasn't created in WebView.newViewRequested, or the "
        "request object was not passed to the new webview. *THIS IS AN "
        "APPLICATION BUG*. Embedders must create a new webview in "
        "WebView.newViewRequested and must pass the request object to the new "
        "WebView. Failure to do this may result in render process crashes or "
        "undefined behaviour. If you want to block a new webview from opening, "
        "this must be done in WebView.navigationRequested. Alternatively, if "
        "your application doesn't support multiple webviews, just don't "
        "implement WebView.newViewRequested.";
  }
  return view;
}

oxide::FilePicker* WebView::CreateFilePicker(content::RenderViewHost* rvh) {
  return new FilePicker(adapter_->CreateFilePickerDelegate(), rvh);
}

void WebView::OnSwapCompositorFrame() {
  adapter_->ScheduleUpdate();
}

void WebView::OnEvictCurrentFrame() {
  adapter_->EvictCurrentFrame();
}

void WebView::OnTextInputStateChanged() {
  if (!HasFocus()) {
    return;
  }

  QGuiApplication::inputMethod()->update(
      static_cast<Qt::InputMethodQueries>(Qt::ImQueryInput | Qt::ImHints));

  if (ShouldShowInputPanel()) {
    SetInputPanelVisibility(true);
  } else if (ShouldHideInputPanel()) {
    SetInputPanelVisibility(false);
  }
}

void WebView::OnFocusedNodeChanged() {
  // Work around for https://launchpad.net/bugs/1323743
  if (QGuiApplication::focusWindow() &&
      QGuiApplication::focusWindow()->focusObject()) {
    QGuiApplication::focusWindow()->focusObjectChanged(
        QGuiApplication::focusWindow()->focusObject());
  }

  if (ShouldHideInputPanel() && HasFocus()) {
    SetInputPanelVisibility(false);
  } else if (has_input_method_state_ && focused_node_is_editable_) {
    QGuiApplication::inputMethod()->reset();
  }
}

void WebView::OnSelectionBoundsChanged() {
  if (!HasFocus()) {
    return;
  }

  QGuiApplication::inputMethod()->update(
      static_cast<Qt::InputMethodQueries>(
        Qt::ImCursorRectangle
        | Qt::ImCursorPosition
        | Qt::ImAnchorPosition
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
        | Qt::ImTextBeforeCursor
        | Qt::ImTextAfterCursor
#endif
      ));
}

void WebView::OnSecurityStatusChanged(const oxide::SecurityStatus& old) {
  OxideQSecurityStatusPrivate::get(qsecurity_status_.get())->Update(old);
}

bool WebView::OnCertificateError(bool is_main_frame,
                                 oxide::CertError cert_error,
                                 const scoped_refptr<net::X509Certificate>& cert,
                                 const GURL& request_url,
                                 content::ResourceType resource_type,
                                 bool overridable,
                                 bool strict_enforcement,
                                 const base::Callback<void(bool)>& callback) {
  scoped_ptr<OxideQSslCertificate> q_cert;
  if (cert) {
    q_cert.reset(OxideQSslCertificatePrivate::Create(cert));
  }

  bool is_subresource =
      !((is_main_frame && resource_type == content::RESOURCE_TYPE_MAIN_FRAME) ||
        (!is_main_frame && resource_type == content::RESOURCE_TYPE_SUB_FRAME));

  scoped_ptr<OxideQCertificateError> error(
      OxideQCertificateErrorPrivate::Create(
        QUrl(QString::fromStdString(request_url.spec())),
        is_main_frame,
        is_subresource,
        overridable,
        strict_enforcement,
        q_cert.Pass(),
        static_cast<OxideQCertificateError::Error>(cert_error),
        callback));

  // Embedder takes ownership of error
  adapter_->CertificateError(error.release());

  return true;
}

// static
WebView* WebView::Create(WebViewAdapter* adapter) {
  return new WebView(adapter);
}

WebView::~WebView() {}

void WebView::HandleFocusEvent(QFocusEvent* event) {
  if (event->gotFocus() && ShouldShowInputPanel()) {
    SetInputPanelVisibility(true);
  }

  FocusChanged();
}

void WebView::HandleInputMethodEvent(QInputMethodEvent* event) {
  QString commit_string = event->commitString();

  if (!commit_string.isEmpty()) {
    gfx::Range replacement_range = gfx::Range::InvalidRange();
    if (event->replacementLength() > 0) {
      replacement_range.set_start(event->replacementStart());
      replacement_range.set_end(event->replacementStart() +
                                event->replacementLength());
    }
    ImeCommitText(base::UTF8ToUTF16(commit_string.toStdString()),
                  replacement_range);
  }

  QString preedit_string = event->preeditString();

  std::vector<blink::WebCompositionUnderline> underlines;
  int cursor_position = -1;
  gfx::Range selection_range = gfx::Range::InvalidRange();

  Q_FOREACH (const QInputMethodEvent::Attribute& attribute, event->attributes()) {
    switch (attribute.type) {
    case QInputMethodEvent::Cursor:
      if (attribute.length > 0) {
        cursor_position = attribute.start;
      }
      break;
    case QInputMethodEvent::Selection:
      selection_range.set_start(
          qMin(attribute.start, (attribute.start + attribute.length)));
      selection_range.set_end(
          qMax(attribute.start, (attribute.start + attribute.length)));
      break;
    case QInputMethodEvent::TextFormat: {
      QTextCharFormat format =
          attribute.value.value<QTextFormat>().toCharFormat();
      blink::WebColor color = format.underlineColor().rgba();
      int start = qMin(attribute.start, (attribute.start + attribute.length));
      int end = qMax(attribute.start, (attribute.start + attribute.length));
      blink::WebCompositionUnderline underline(
          start, end, color, false, SK_ColorTRANSPARENT);
      underlines.push_back(underline);
      break;
    }
    default:
      break;
    }
  }

  if (!selection_range.IsValid()) {
    selection_range = gfx::Range(
        cursor_position > 0 ? cursor_position : preedit_string.length());
  }
  ImeSetComposingText(base::UTF8ToUTF16(preedit_string.toStdString()),
                      underlines, selection_range);

  has_input_method_state_ = !preedit_string.isEmpty();
}

void WebView::HandleKeyEvent(QKeyEvent* event) {
  content::NativeWebKeyboardEvent e(MakeNativeWebKeyboardEvent(event, false));
  oxide::WebView::HandleKeyEvent(e);

  // If the event is a printable character, send a corresponding Char event
  if (event->type() == QEvent::KeyPress && e.text[0] != 0) {
    oxide::WebView::HandleKeyEvent(MakeNativeWebKeyboardEvent(event, true));
  }
}

void WebView::HandleMouseEvent(QMouseEvent* event) {
  if (!(event->button() == Qt::LeftButton ||
        event->button() == Qt::MidButton ||
        event->button() == Qt::RightButton ||
        event->button() == Qt::NoButton)) {
    event->ignore();
    return;
  }

  oxide::WebView::HandleMouseEvent(
      MakeWebMouseEvent(event, GetDeviceScaleFactor()));
}

void WebView::HandleTouchEvent(QTouchEvent* event) {
  ScopedVector<ui::TouchEvent> events;
  MakeUITouchEvents(event, GetDeviceScaleFactor(), &events);

  for (size_t i = 0; i < events.size(); ++i) {
    oxide::WebView::HandleTouchEvent(*events[i]);
  }
}

void WebView::HandleWheelEvent(QWheelEvent* event) {
  oxide::WebView::HandleWheelEvent(
      MakeWebMouseWheelEvent(event, GetDeviceScaleFactor()));
}

QVariant WebView::InputMethodQuery(Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImHints:
      return QVariant(QImHintsFromInputType(text_input_type_));
    case Qt::ImCursorRectangle: {
      // XXX: Is this in the right coordinate space?
      return QRect(caret_rect_.x(), caret_rect_.y(),
                   caret_rect_.width(), caret_rect_.height());
    }
    case Qt::ImCursorPosition:
      return static_cast<int>(selection_cursor_position_ & INT_MAX);
    case Qt::ImSurroundingText:
      return QString::fromStdString(base::UTF16ToUTF8(GetSelectionText()));
    case Qt::ImCurrentSelection:
      return QString::fromStdString(base::UTF16ToUTF8(GetSelectedText()));
    case Qt::ImAnchorPosition:
      return static_cast<int>(selection_anchor_position_ & INT_MAX);
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    case Qt::ImTextBeforeCursor: {
      std::string text = base::UTF16ToUTF8(GetSelectionText());
      return QString::fromStdString(
          text.substr(0, selection_cursor_position_));
    }
    case Qt::ImTextAfterCursor: {
      std::string text = base::UTF16ToUTF8(GetSelectionText());
      if (selection_cursor_position_ > text.length()) {
        return QString();
      }
      return QString::fromStdString(
          text.substr(selection_cursor_position_, std::string::npos));
    }
#endif
    default:
      break;
  }

  return QVariant();
}

} // namespace qt
} // namespace oxide
