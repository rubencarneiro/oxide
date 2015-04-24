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

#include "oxide_qt_web_view.h"

#include <limits>
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

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_vector.h"
#include "base/pickle.h"
#include "base/strings/utf_string_conversions.h"
#include "cc/output/compositor_frame_metadata.h"
#include "content/common/cursors/webcursor.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/navigation_controller.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "net/base/net_errors.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/WebKit/public/platform/WebColor.h"
#include "third_party/WebKit/public/platform/WebCursorInfo.h"
#include "third_party/WebKit/public/platform/WebTopControlsState.h"
#include "ui/base/ime/text_input_type.h"
#include "ui/events/event.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/range/range.h"
#include "url/gurl.h"

#include "qt/core/api/oxideqdownloadrequest.h"
#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqnavigationrequest.h"
#include "qt/core/api/oxideqnewviewrequest.h"
#include "qt/core/api/oxideqnewviewrequest_p.h"
#include "qt/core/api/oxideqpermissionrequest.h"
#include "qt/core/api/oxideqpermissionrequest_p.h"
#include "qt/core/api/oxideqcertificateerror.h"
#include "qt/core/api/oxideqcertificateerror_p.h"
#include "qt/core/api/oxideqsecuritystatus.h"
#include "qt/core/api/oxideqsecuritystatus_p.h"
#include "qt/core/api/oxideqwebpreferences.h"
#include "qt/core/api/oxideqwebpreferences_p.h"
#include "qt/core/common/oxide_qt_screen_utils.h"
#include "qt/core/common/oxide_qt_skutils.h"
#include "qt/core/glue/oxide_qt_web_view_proxy_client.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/browser/oxide_browser_process_main.h"
#include "shared/browser/oxide_content_types.h"
#include "shared/browser/oxide_render_widget_host_view.h"
#include "shared/common/oxide_enum_flags.h"

#include "oxide_qt_file_picker.h"
#include "oxide_qt_javascript_dialog.h"
#include "oxide_qt_script_message_handler.h"
#include "oxide_qt_web_context.h"
#include "oxide_qt_web_context_menu.h"
#include "oxide_qt_web_frame.h"
#include "oxide_qt_web_popup_menu.h"
#include "oxide_qt_web_preferences.h"

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

static const char* STATE_SERIALIZER_MAGIC_NUMBER = "oxide";
static uint16_t STATE_SERIALIZER_VERSION = 1;

blink::WebTopControlsState LocationBarModeToBlinkTopControlsState(
    LocationBarMode mode) {
  switch (mode) {
    case LOCATION_BAR_MODE_AUTO:
      return blink::WebTopControlsBoth;
    case LOCATION_BAR_MODE_SHOWN:
      return blink::WebTopControlsShown;
    case LOCATION_BAR_MODE_HIDDEN:
      return blink::WebTopControlsHidden;
    default:
      NOTREACHED();
      return blink::WebTopControlsBoth;
  }
}

}

class CompositorFrameHandleImpl : public CompositorFrameHandle {
 public:
  CompositorFrameHandleImpl(oxide::CompositorFrameHandle* frame,
                            int location_bar_content_offset)
      : frame_(frame) {
    if (frame_.get()) {
      rect_ = QRect(0, location_bar_content_offset,
                    frame_->size_in_pixels().width(),
                    frame_->size_in_pixels().height());
    }
  }

  virtual ~CompositorFrameHandleImpl() {}

  CompositorFrameHandle::Type GetType() final {
    if (!frame_.get()) {
      return CompositorFrameHandle::TYPE_INVALID;
    }
    if (frame_->gl_frame_data()) {
      return CompositorFrameHandle::TYPE_ACCELERATED;
    }
    if (frame_->image_frame_data()) {
      return CompositorFrameHandle::TYPE_IMAGE;
    }
    if (frame_->software_frame_data()) {
      return CompositorFrameHandle::TYPE_SOFTWARE;
    }

    NOTREACHED();
    return CompositorFrameHandle::TYPE_INVALID;
  }

  const QRect& GetRect() const final {
    return rect_;
  }

  QImage GetSoftwareFrame() final {
    DCHECK_EQ(GetType(), CompositorFrameHandle::TYPE_SOFTWARE);
    return QImage(
        static_cast<uchar *>(frame_->software_frame_data()->pixels()),
        frame_->size_in_pixels().width(),
        frame_->size_in_pixels().height(),
        QImage::Format_ARGB32);
  }

  unsigned int GetAcceleratedFrameTexture() final {
    DCHECK_EQ(GetType(), CompositorFrameHandle::TYPE_ACCELERATED);
    return frame_->gl_frame_data()->texture_id();
  }

  EGLImageKHR GetImageFrame() final {
    return frame_->image_frame_data()->image();
  }

 private:
  scoped_refptr<oxide::CompositorFrameHandle> frame_;
  QRect rect_;
};

void WebView::OnInputPanelVisibilityChanged() {
  InputPanelVisibilityChanged();
}

float WebView::GetDeviceScaleFactor() const {
  QScreen* screen = client_->GetScreen();
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }

  return GetDeviceScaleFactorFromQScreen(screen);
}

bool WebView::ShouldShowInputPanel() const {
  if (text_input_type_ != ui::TEXT_INPUT_TYPE_NONE &&
      show_ime_if_needed_ && focused_node_is_editable_) {
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
  client_->SetInputMethodEnabled(visible);

  if (!visible) {
    has_input_method_state_ = false;
  }

  // Do not check whether the input method is currently visible here, to avoid
  // a possible race condition: if hide() and show() are called very quickly
  // in a row, when show() is called the hide() request might not have
  // completed yet, and isVisible() could return true.
  QGuiApplication::inputMethod()->setVisible(visible);
}

void WebView::RestoreState(qt::RestoreType type, const QByteArray& state) {
  COMPILE_ASSERT(
      RESTORE_CURRENT_SESSION == static_cast<RestoreType>(
          content::NavigationController::RESTORE_CURRENT_SESSION),
      restore_type_enums_current_doesnt_match);
  COMPILE_ASSERT(
      RESTORE_LAST_SESSION_EXITED_CLEANLY == static_cast<RestoreType>(
          content::NavigationController::RESTORE_LAST_SESSION_EXITED_CLEANLY),
      restore_type_enums_exited_cleanly_doesnt_match);
  COMPILE_ASSERT(
      RESTORE_LAST_SESSION_CRASHED == static_cast<RestoreType>(
          content::NavigationController::RESTORE_LAST_SESSION_CRASHED),
      restore_type_enums_crashed_doesnt_match);

  content::NavigationController::RestoreType restore_type =
      static_cast<content::NavigationController::RestoreType>(type);

#define WARN_INVALID_DATA \
    qWarning() << "Failed to read initial state: invalid data"
  std::vector<sessions::SerializedNavigationEntry> entries;
  Pickle pickle(state.data(), state.size());
  PickleIterator i(pickle);
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

  SetState(restore_type, entries, index);
}

void WebView::EnsurePreferences() {
  if (GetWebPreferences()) {
    return;
  }

  OxideQWebPreferences* p = new OxideQWebPreferences(client_->GetApiHandle());
  SetWebPreferences(
      OxideQWebPreferencesPrivate::get(p)->preferences());
}

void WebView::Init(oxide::WebView::Params* params) {
  oxide::WebView::Init(params);

  OxideQWebPreferences* p =
      static_cast<WebPreferences*>(GetWebPreferences())->api_handle();
  if (!p->parent()) {
    // This will happen for a WebView created by newViewRequested, as
    // we clone the openers preferences before the WebView is created
    p->setParent(client_->GetApiHandle());
  }

  client_->Initialized();
}

blink::WebScreenInfo WebView::GetScreenInfo() const {
  QScreen* screen = client_->GetScreen();
  if (!screen) {
    screen = QGuiApplication::primaryScreen();
  }

  return GetWebScreenInfoFromQScreen(screen);
}

gfx::Rect WebView::GetViewBoundsPix() const {
  QRect bounds = client_->GetViewBoundsPix();
  return gfx::Rect(bounds.x(),
                   bounds.y(),
                   bounds.width(),
                   bounds.height());
}

bool WebView::IsVisible() const {
  return client_->IsVisible();
}

bool WebView::HasFocus() const {
  return client_->HasFocus();
}

bool WebView::IsInputPanelVisible() const {
  QInputMethod* im = QGuiApplication::inputMethod();
  if (!im) {
    return false;
  }

  return im->isVisible();
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

bool WebView::CanCreateWindows() const {
  return client_->CanCreateWindows();
}

size_t WebView::GetScriptMessageHandlerCount() const {
  return message_handlers_.size();
}

const oxide::ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(
    size_t index) const {
  return ScriptMessageHandler::FromProxyHandle(
      message_handlers_.at(index))->handler();
}

void WebView::OnURLChanged() {
  client_->URLChanged();
}

void WebView::OnTitleChanged() {
  client_->TitleChanged();
}

void WebView::OnIconChanged(const GURL& icon) {
  client_->IconChanged(QUrl(QString::fromStdString(icon.spec())));
}

void WebView::OnCommandsUpdated() {
  client_->CommandsUpdated();
}

void WebView::OnLoadingChanged() {
  client_->LoadingChanged();
}

void WebView::OnLoadProgressChanged(double progress) {
  client_->LoadProgressChanged(progress);
}

void WebView::OnLoadStarted(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStarted);
  client_->LoadEvent(&event);
}

void WebView::OnLoadRedirected(const GURL& url,
                               const GURL& original_url) {
  OxideQLoadEvent event(
     QUrl(QString::fromStdString(url.spec())),
     QUrl(QString::fromStdString(original_url.spec())));
  client_->LoadEvent(&event);
}

void WebView::OnLoadCommitted(const GURL& url,
                              bool is_error_page) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(url.spec())),
      OxideQLoadEvent::TypeCommitted,
      is_error_page);
  client_->LoadEvent(&event);
}

void WebView::OnLoadStopped(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeStopped);
  client_->LoadEvent(&event);
}

void WebView::OnLoadFailed(const GURL& validated_url,
                           int error_code,
                           const std::string& error_description) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      ErrorDomainFromErrorCode(error_code),
      QString::fromStdString(error_description),
      error_code);
  client_->LoadEvent(&event);
}

void WebView::OnLoadSucceeded(const GURL& validated_url) {
  OxideQLoadEvent event(
      QUrl(QString::fromStdString(validated_url.spec())),
      OxideQLoadEvent::TypeSucceeded);
  client_->LoadEvent(&event);
}

void WebView::OnNavigationEntryCommitted() {
  client_->NavigationEntryCommitted();
}

void WebView::OnNavigationListPruned(bool from_front, int count) {
  client_->NavigationListPruned(from_front, count);
}

void WebView::OnNavigationEntryChanged(int index) {
  client_->NavigationEntryChanged(index);
}

bool WebView::OnAddMessageToConsole(
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

void WebView::OnToggleFullscreenMode(bool enter) {
  client_->ToggleFullscreenMode(enter);
}

void WebView::OnWebPreferencesDestroyed() {
  OxideQWebPreferences* p = new OxideQWebPreferences(client_->GetApiHandle());
  SetWebPreferences(
      OxideQWebPreferencesPrivate::get(p)->preferences());
  client_->WebPreferencesReplaced();
}

void WebView::OnRequestGeolocationPermission(
    scoped_ptr<oxide::SimplePermissionRequest> request) {
  scoped_ptr<OxideQGeolocationPermissionRequest> req(
      OxideQGeolocationPermissionRequestPrivate::Create(
        request.Pass()));

  // The embedder takes ownership of this
  client_->RequestGeolocationPermission(req.release());
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
  DCHECK(!event.os_event->isAccepted());

  client_->HandleUnhandledKeyboardEvent(event.os_event);
}

OXIDE_MAKE_ENUM_BITWISE_OPERATORS(FrameMetadataChangeFlags)

void WebView::OnFrameMetadataUpdated(const cc::CompositorFrameMetadata& old) {
  FrameMetadataChangeFlags flags = FRAME_METADATA_CHANGE_NONE;

  if (old.root_scroll_offset.x() !=
          compositor_frame_metadata().root_scroll_offset.x() ||
      old.root_scroll_offset.y() !=
          compositor_frame_metadata().root_scroll_offset.y()) {
    flags |= FRAME_METADATA_CHANGE_SCROLL_OFFSET;
  }
  if (old.root_layer_size.width() !=
          compositor_frame_metadata().root_layer_size.width() ||
      old.root_layer_size.height() !=
          compositor_frame_metadata().root_layer_size.height()) {
    flags |= FRAME_METADATA_CHANGE_CONTENT;
  }
  if (old.scrollable_viewport_size.width() !=
          compositor_frame_metadata().scrollable_viewport_size.width() ||
      old.scrollable_viewport_size.height() !=
          compositor_frame_metadata().scrollable_viewport_size.height()) {
    flags |= FRAME_METADATA_CHANGE_VIEWPORT;
  }
  if (old.location_bar_offset.y() !=
      compositor_frame_metadata().location_bar_offset.y()) {
    flags |= FRAME_METADATA_CHANGE_CONTROLS_OFFSET;
  }
  if (old.location_bar_content_translation.y() !=
      compositor_frame_metadata().location_bar_content_translation.y()) {
    flags |= FRAME_METADATA_CHANGE_CONTENT_OFFSET;
  }
  if (old.device_scale_factor !=
          compositor_frame_metadata().device_scale_factor ||
      old.page_scale_factor !=
          compositor_frame_metadata().page_scale_factor) {
    flags |= FRAME_METADATA_CHANGE_SCROLL_OFFSET;
    flags |= FRAME_METADATA_CHANGE_CONTENT;
    flags |= FRAME_METADATA_CHANGE_VIEWPORT;
    flags |= FRAME_METADATA_CHANGE_CONTROLS_OFFSET;
    flags |= FRAME_METADATA_CHANGE_CONTENT_OFFSET;
  }

  client_->FrameMetadataUpdated(flags);
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

  client_->DownloadRequested(&downloadRequest);
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

  client_->NavigationRequested(&request);

  return request.action() == OxideQNavigationRequest::ActionAccept;
}

oxide::WebFrame* WebView::CreateWebFrame(
    content::RenderFrameHost* render_frame_host) {
  WebFrame* frame = new WebFrame(render_frame_host, this);
  WebFrameProxyHandle* handle = client_->CreateWebFrame(frame);
  return WebFrame::FromProxyHandle(handle);
}

oxide::WebContextMenu* WebView::CreateContextMenu(
    content::RenderFrameHost* rfh,
    const content::ContextMenuParams& params) {
  WebContextMenu* menu = new WebContextMenu(rfh, params);
  menu->SetProxy(client_->CreateWebContextMenu(menu));
  return menu;
}

oxide::WebPopupMenu* WebView::CreatePopupMenu(content::RenderFrameHost* rfh) {
  WebPopupMenu* menu = new WebPopupMenu(rfh);
  menu->SetProxy(client_->CreateWebPopupMenu(menu));
  return menu;
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

  client_->NewViewRequested(&request);

  oxide::WebView* view = OxideQNewViewRequestPrivate::get(&request)->view.get();
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
  FilePicker* picker = new FilePicker(rvh);
  picker->SetProxy(client_->CreateFilePicker(picker));
  return picker;
}

void WebView::OnSwapCompositorFrame() {
  compositor_frame_.reset();
  client_->ScheduleUpdate();
}

void WebView::OnEvictCurrentFrame() {
  compositor_frame_.reset();
  client_->EvictCurrentFrame();
}

void WebView::OnTextInputStateChanged() {
  if (!HasFocus()) {
    return;
  }

  if (text_input_type_ != ui::TEXT_INPUT_TYPE_NONE) {
    QGuiApplication::inputMethod()->update(
        static_cast<Qt::InputMethodQueries>(Qt::ImQueryInput | Qt::ImHints));
  }

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
  } else if (!has_input_method_state_ && ShouldShowInputPanel()) {
    SetInputPanelVisibility(true);
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

void WebView::OnImeCancelComposition() {
  if (has_input_method_state_) {
    QGuiApplication::inputMethod()->reset();
  }
}

void WebView::OnSelectionChanged() {
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

void WebView::OnUpdateCursor(const content::WebCursor& cursor) {
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
      client_->UpdateCursor(QCursor(cursor_pixmap));
    }
  } else {
    client_->UpdateCursor(QCursorFromWebCursor(cursor_info.type));
  }
}

void WebView::OnSecurityStatusChanged(const oxide::SecurityStatus& old) {
  OxideQSecurityStatusPrivate::get(qsecurity_status_.get())->Update(old);
}

void WebView::OnCertificateError(scoped_ptr<oxide::CertificateError> error) {
  scoped_ptr<OxideQCertificateError> qerror(
      OxideQCertificateErrorPrivate::Create(error.Pass()));

  // Embedder takes ownership of qerror
  client_->CertificateError(qerror.release());
}

void WebView::OnContentBlocked() {
  client_->ContentBlocked();
}

void WebView::OnPrepareToCloseResponse(bool proceed) {
  client_->PrepareToCloseResponse(proceed);
}

void WebView::OnCloseRequested() {
  client_->CloseRequested();
}

void WebView::init(bool incognito,
                   WebContextProxyHandle* context,
                   OxideQNewViewRequest* new_view_request,
                   const QByteArray& restore_state,
                   qt::RestoreType restore_type) {
  DCHECK(!GetWebContents());

  bool script_opened = false;

  if (new_view_request) {
    OxideQNewViewRequestPrivate* rd =
        OxideQNewViewRequestPrivate::get(new_view_request);
    if (rd->view) {
      qWarning() << "OxideQNewViewRequest: Cannot assign to more than one WebView";
    } else {
      rd->view = AsWeakPtr();
      script_opened = true;
    }
  }

  if (script_opened) {
    // Script opened webviews get initialized via another path
    return;
  }

  if (!restore_state.isEmpty()) {
    RestoreState(restore_type, restore_state);
  }

  CHECK(context) <<
      "No context available for WebView. If you see this when running in "
      "single-process mode, it is possible that the default WebContext has "
      "been deleted by the application. In single-process mode, there is only "
      "one WebContext, and this has to live for the life of the application";

  WebContext* c = WebContext::FromProxyHandle(context);

  if (oxide::BrowserProcessMain::GetInstance()->GetProcessModel() ==
          oxide::PROCESS_MODEL_SINGLE_PROCESS) {
    DCHECK(!incognito);
    DCHECK_EQ(c, WebContext::GetDefault());
  }

  EnsurePreferences();

  oxide::WebView::Params params;
  params.context = c->GetContext();
  params.incognito = incognito;

  Init(&params);
}

QUrl WebView::url() const {
  return QUrl(QString::fromStdString(GetURL().spec()));
}

void WebView::setUrl(const QUrl& url) {
  SetURL(GURL(url.toString().toStdString()));
}

QString WebView::title() const {
  return QString::fromStdString(GetTitle());
}

bool WebView::canGoBack() const {
  return CanGoBack();
}

bool WebView::canGoForward() const {
  return CanGoForward();
}

bool WebView::incognito() const {
  return IsIncognito();  
}

bool WebView::loading() const {
  return IsLoading();
}

bool WebView::fullscreen() const {
  return IsFullscreen();
}

void WebView::setFullscreen(bool fullscreen) {
  SetIsFullscreen(fullscreen);
}

WebFrameProxyHandle* WebView::rootFrame() const {
  WebFrame* f = static_cast<WebFrame*>(GetRootFrame());
  if (!f) {
    return nullptr;
  }

  return f->handle();
}

WebContextProxyHandle* WebView::context() const {
  WebContext* c = GetContext();
  if (!c) {
    return nullptr;
  }

  return c->handle();
}

void WebView::wasResized() {
  WasResized();
}

void WebView::screenUpdated() {
  ScreenUpdated();
}

void WebView::visibilityChanged() {
  VisibilityChanged();
}

void WebView::handleFocusEvent(QFocusEvent* event) {
  if (event->gotFocus() && ShouldShowInputPanel()) {
    SetInputPanelVisibility(true);
  }

  FocusChanged();
}

void WebView::handleInputMethodEvent(QInputMethodEvent* event) {
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

void WebView::handleKeyEvent(QKeyEvent* event) {
  content::NativeWebKeyboardEvent e(MakeNativeWebKeyboardEvent(event, false));
  HandleKeyEvent(e);

  // If the event is a printable character, send a corresponding Char event
  if (event->type() == QEvent::KeyPress && e.text[0] != 0) {
    HandleKeyEvent(MakeNativeWebKeyboardEvent(event, true));
  }
}

void WebView::handleMouseEvent(QMouseEvent* event) {
  if (!(event->button() == Qt::LeftButton ||
        event->button() == Qt::MidButton ||
        event->button() == Qt::RightButton ||
        event->button() == Qt::NoButton)) {
    event->ignore();
    return;
  }

  HandleMouseEvent(
      MakeWebMouseEvent(event,
                        GetDeviceScaleFactor(),
                        GetLocationBarContentOffsetDip()));
}

void WebView::handleTouchEvent(QTouchEvent* event) {
  ScopedVector<ui::TouchEvent> events;
  touch_event_factory_.MakeEvents(event,
                                  GetDeviceScaleFactor(),
                                  GetLocationBarContentOffsetDip(),
                                  &events);

  for (size_t i = 0; i < events.size(); ++i) {
    HandleTouchEvent(*events[i]);
  }
}

void WebView::handleWheelEvent(QWheelEvent* event) {
  HandleWheelEvent(
      MakeWebMouseWheelEvent(event,
                             GetDeviceScaleFactor(),
                             GetLocationBarContentOffsetDip()));
}

QVariant WebView::inputMethodQuery(Qt::InputMethodQuery query) const {
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

void WebView::goBack() {
  GoBack();
}

void WebView::goForward() {
  GoForward();
}

void WebView::stop() {
  Stop();
}

void WebView::reload() {
  Reload();
}

void WebView::loadHtml(const QString& html, const QUrl& base_url) {
  QByteArray encoded_data = html.toUtf8().toPercentEncoding();
  LoadData(std::string(encoded_data.constData(), encoded_data.length()),
           "text/html;charset=UTF-8",
           GURL(base_url.toString().toStdString()));
}

QList<ScriptMessageHandlerProxyHandle*>& WebView::messageHandlers() {
  return message_handlers_;
}

bool WebView::isInitialized() const {
  return GetWebContents() != nullptr;
}

int WebView::getNavigationEntryCount() const {
  return GetNavigationEntryCount();
}

int WebView::getNavigationCurrentEntryIndex() const {
  return GetNavigationCurrentEntryIndex();
}

void WebView::setNavigationCurrentEntryIndex(int index) {
  SetNavigationCurrentEntryIndex(index);
}

int WebView::getNavigationEntryUniqueID(int index) const {
  return GetNavigationEntryUniqueID(index);
}

QUrl WebView::getNavigationEntryUrl(int index) const {
  return QUrl(QString::fromStdString(GetNavigationEntryUrl(index).spec()));
}

QString WebView::getNavigationEntryTitle(int index) const {
  return QString::fromStdString(GetNavigationEntryTitle(index));
}

QDateTime WebView::getNavigationEntryTimestamp(int index) const {
  return QDateTime::fromMSecsSinceEpoch(
      GetNavigationEntryTimestamp(index).ToJsTime());
}

QByteArray WebView::currentState() const {
  // XXX(chrisccoulson): Move the pickling in to oxide::WebView
  std::vector<sessions::SerializedNavigationEntry> entries = GetState();
  if (entries.size() == 0) {
    return QByteArray();
  }
  Pickle pickle;
  pickle.WriteString(STATE_SERIALIZER_MAGIC_NUMBER);
  pickle.WriteUInt16(STATE_SERIALIZER_VERSION);
  pickle.WriteInt(entries.size());
  std::vector<sessions::SerializedNavigationEntry>::const_iterator i;
  static const size_t max_state_size = std::numeric_limits<uint16>::max() - 1024;
  for (i = entries.begin(); i != entries.end(); ++i) {
    i->WriteToPickle(max_state_size, &pickle);
  }
  pickle.WriteInt(GetNavigationCurrentEntryIndex());
  return QByteArray(static_cast<const char*>(pickle.data()), pickle.size());
}

OxideQWebPreferences* WebView::preferences() {
  EnsurePreferences();
  return static_cast<WebPreferences*>(GetWebPreferences())->api_handle();
}

void WebView::setPreferences(OxideQWebPreferences* prefs) {
  OxideQWebPreferences* old = nullptr;
  if (WebPreferences* o = static_cast<WebPreferences *>(GetWebPreferences())) {
    old = o->api_handle();
  }

  if (!prefs) {
    prefs = new OxideQWebPreferences(client_->GetApiHandle());
  } else if (!prefs->parent()) {
    prefs->setParent(client_->GetApiHandle());
  }

  SetWebPreferences(
      OxideQWebPreferencesPrivate::get(prefs)->preferences());

  if (!old) {
    return;
  }

  if (old->parent() == client_->GetApiHandle()) {
    delete old;
  }
}

void WebView::updateWebPreferences() {
  UpdateWebPreferences();
}

QPoint WebView::compositorFrameScrollOffsetPix() {
  gfx::Point offset = GetCompositorFrameScrollOffsetPix();
  return QPoint(offset.x(), offset.y());
}

QSize WebView::compositorFrameContentSizePix() {
  gfx::Size size = GetCompositorFrameContentSizePix();
  return QSize(size.width(), size.height());
}

QSize WebView::compositorFrameViewportSizePix() {
  gfx::Size size = GetCompositorFrameViewportSizePix();
  return QSize(size.width(), size.height());
}

QSharedPointer<CompositorFrameHandle> WebView::compositorFrameHandle() {
  if (!compositor_frame_) {
    compositor_frame_ =
        QSharedPointer<CompositorFrameHandle>(new CompositorFrameHandleImpl(
          GetCompositorFrameHandle(),
          compositor_frame_metadata().device_scale_factor *
            compositor_frame_metadata().location_bar_content_translation.y()));
  }

  return compositor_frame_;
}

void WebView::didCommitCompositorFrame() {
  DidCommitCompositorFrame();
}

void WebView::setCanTemporarilyDisplayInsecureContent(bool allow) {
  SetCanTemporarilyDisplayInsecureContent(allow);
}

void WebView::setCanTemporarilyRunInsecureContent(bool allow) {
  SetCanTemporarilyRunInsecureContent(allow);
}

OxideQSecurityStatus* WebView::securityStatus() {
  return qsecurity_status_.get();
}

ContentTypeFlags WebView::blockedContent() const {
  COMPILE_ASSERT(
      CONTENT_TYPE_NONE ==
        static_cast<ContentTypeFlags>(oxide::CONTENT_TYPE_NONE),
      content_type_flags_none_doesnt_match);
  COMPILE_ASSERT(
      CONTENT_TYPE_MIXED_DISPLAY ==
        static_cast<ContentTypeFlags>(oxide::CONTENT_TYPE_MIXED_DISPLAY),
      content_type_flags_mixed_display_doesnt_match);
  COMPILE_ASSERT(
      CONTENT_TYPE_MIXED_SCRIPT ==
        static_cast<ContentTypeFlags>(oxide::CONTENT_TYPE_MIXED_SCRIPT),
      content_type_flags_mixed_script_doesnt_match);

  return static_cast<ContentTypeFlags>(blocked_content());
}

void WebView::prepareToClose() {
  PrepareToClose();
}

int WebView::locationBarHeight() {
  return GetLocationBarHeightPix();
}

void WebView::setLocationBarHeight(int height) {
  SetLocationBarHeightPix(height);
}

int WebView::locationBarOffsetPix() {
  return GetLocationBarOffsetPix();
}

int WebView::locationBarContentOffsetPix() {
  return GetLocationBarContentOffsetPix();
}

LocationBarMode WebView::locationBarMode() const {
  switch (location_bar_constraints()) {
    case blink::WebTopControlsShown:
      return LOCATION_BAR_MODE_SHOWN;
    case blink::WebTopControlsHidden:
      return LOCATION_BAR_MODE_HIDDEN;
    case blink::WebTopControlsBoth:
      return LOCATION_BAR_MODE_AUTO;
    default:
      NOTREACHED();
      return LOCATION_BAR_MODE_AUTO;
  }
}

void WebView::setLocationBarMode(LocationBarMode mode) {
  SetLocationBarConstraints(LocationBarModeToBlinkTopControlsState(mode));
}

bool WebView::locationBarAnimated() const {
  return location_bar_animated();
}

void WebView::setLocationBarAnimated(bool animated) {
  set_location_bar_animated(animated);
}

void WebView::locationBarShow(bool animate) {
  ShowLocationBar(animate);
}

void WebView::locationBarHide(bool animate) {
  HideLocationBar(animate);
}

WebView::WebView(WebViewProxyClient* client) :
    client_(client),
    has_input_method_state_(false),
    qsecurity_status_(
        OxideQSecurityStatusPrivate::Create(this)) {
  QInputMethod* im = QGuiApplication::inputMethod();
  if (im) {
    connect(im, SIGNAL(visibleChanged()),
            SLOT(OnInputPanelVisibilityChanged()));
  }
}

WebView::~WebView() {
  QInputMethod* im = QGuiApplication::inputMethod();
  if (im) {
    im->disconnect(this);
  }
}

// static
WebView* WebView::FromProxyHandle(WebViewProxyHandle* handle) {
  return static_cast<WebView*>(handle->proxy_.data());
}

WebContext* WebView::GetContext() const {
  return WebContext::FromBrowserContext(GetBrowserContext());
}

void WebView::FrameAdded(oxide::WebFrame* frame) {
  client_->FrameAdded(static_cast<WebFrame*>(frame)->handle());
}

void WebView::FrameRemoved(oxide::WebFrame* frame) {
  client_->FrameRemoved(static_cast<WebFrame*>(frame)->handle());
}

} // namespace qt
} // namespace oxide
