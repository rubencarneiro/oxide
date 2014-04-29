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

#include <QString>
#include <QUrl>
#include <QCoreApplication>

#include "base/strings/utf_string_conversions.h"
#include "url/gurl.h"
#include "content/public/browser/navigation_controller.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "net/base/net_errors.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "third_party/WebKit/Source/platform/WindowsKeyboardCodes.h"

#include "qt/core/api/oxideqloadevent.h"
#include "qt/core/api/oxideqnavigationrequest.h"
#include "qt/core/api/oxideqnewviewrequest.h"
#include "qt/core/api/oxideqnewviewrequest_p.h"
#include "qt/core/glue/oxide_qt_script_message_handler_adapter_p.h"
#include "qt/core/glue/oxide_qt_web_frame_adapter.h"
#include "qt/core/glue/oxide_qt_web_view_adapter.h"

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

}

WebView::WebView(WebViewAdapter* adapter) :
    adapter_(adapter) {}

bool WebView::Init(const oxide::WebView::Params& params) {
  if (!oxide::WebView::Init(params)) {
    return false;
  }

  adapter_->Initialized();
  return true;
}

size_t WebView::GetScriptMessageHandlerCount() const {
  return adapter_->message_handlers().size();
}

oxide::ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(
    size_t index) const {
  return &ScriptMessageHandlerAdapterPrivate::get(
      adapter_->message_handlers().at(index))->handler;
}

gfx::Rect WebView::GetContainerBounds() {
  QRect bounds = adapter_->GetContainerBounds();
  return gfx::Rect(bounds.x(),
                   bounds.y(),
                   bounds.width(),
                   bounds.height());
}

bool WebView::IsVisible() const {
  return adapter_->IsVisible();
}

oxide::WebPopupMenu* WebView::CreatePopupMenu(content::RenderViewHost* rvh) {
  return new WebPopupMenu(adapter_->CreateWebPopupMenuDelegate(), rvh);
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

oxide::FilePicker* WebView::CreateFilePicker(content::RenderViewHost* rvh) {
  return new FilePicker(adapter_->CreateFilePickerDelegate(), rvh);
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

void WebView::OnURLChanged() {
  adapter_->URLChanged();
}

void WebView::OnTitleChanged() {
  adapter_->TitleChanged();
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

void WebView::OnWebPreferencesChanged() {
  adapter_->WebPreferencesChanged();
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

  return OxideQNewViewRequestPrivate::get(&request)->view.get();
}

// static
WebView* WebView::Create(WebViewAdapter* adapter) {
  return new WebView(adapter);
}

int WebView::WebEventKeyCodeToQKeyEventKeyCode(
    const content::NativeWebKeyboardEvent& event) {

  int keycode = event.windowsKeyCode;

  if (event.modifiers & blink::WebInputEvent::IsKeyPad) {
    if (keycode >= VK_NUMPAD0 && keycode >= VK_NUMPAD9) {
      return (keycode - VK_NUMPAD0) + Qt::Key_0;
    }

    switch (keycode) {
    case VK_MULTIPLY:
      return Qt::Key_Asterisk;
    case VK_ADD:
      return Qt::Key_Plus;
    case VK_SUBTRACT:
      return Qt::Key_Minus;
    case VK_DECIMAL:
      return Qt::Key_Period;
    case VK_DIVIDE:
      return Qt::Key_Slash;
    case VK_PRIOR:
      return Qt::Key_PageUp;
    case VK_NEXT:
      return Qt::Key_PageDown;
    case VK_HOME:
      return Qt::Key_Home;
    case VK_END:
      return Qt::Key_End;
    case VK_INSERT:
      return Qt::Key_Insert;
    case VK_DELETE:
      return Qt::Key_Delete;
    case VK_RETURN:
      // Same WebEventKeyCode for Qt::Key_Enter
      return Qt::Key_Return;
    case VK_UP:
      return Qt::Key_Up;
    case VK_DOWN:
      return Qt::Key_Down;
    case VK_LEFT:
      return Qt::Key_Left;
    case VK_RIGHT:
      return Qt::Key_Right;
    default:
      return 0;
    }
  }

  if (keycode >= VK_A && keycode <= VK_Z) {
    return (keycode - VK_A) + Qt::Key_A;
  }

  if (keycode >= VK_0 && keycode <= VK_9) {
    return (keycode - VK_0) + Qt::Key_0;
  }

  if (keycode >= VK_F1 && keycode <= VK_F24) {
    // We miss Qt::Key_F25 - Qt::Key_F35
    return (keycode - VK_F1) + Qt::Key_F1;
  }

  switch (keycode) {
  case VK_ESCAPE:
    return Qt::Key_Escape;
  case VK_TAB:
    // Same WebEventKeyCode for Qt::Key_Backtab
    return Qt::Key_Tab;
  case VK_BACK:
    return Qt::Key_Backspace;
  case VK_RETURN:
    // Same WebEventKeyCode for Qt::Key_Enter
    return Qt::Key_Return;
  case VK_INSERT:
    return Qt::Key_Insert;
  case VK_DELETE:
    return Qt::Key_Delete;
  case VK_PAUSE:
    return Qt::Key_Pause;
  case VK_SNAPSHOT:
    // Same WebEventKeyCode for Qt::Key_SysReq
    return Qt::Key_Print;
  case VK_CLEAR:
    return Qt::Key_Clear;
  case VK_HOME:
    return Qt::Key_Home;
  case VK_END:
    return Qt::Key_End;
  case VK_LEFT:
    return Qt::Key_Left;
  case VK_RIGHT:
    return Qt::Key_Right;
  case VK_DOWN:
    return Qt::Key_Down;
  case VK_PRIOR:
    return Qt::Key_PageUp;
  case VK_NEXT:
    return Qt::Key_PageDown;
  case VK_SHIFT:
    return Qt::Key_Shift;
  case VK_CONTROL:
    return Qt::Key_Control;
  case VK_MENU:
    // Same WebEventKeyCode for Qt::Key_Meta
    return Qt::Key_Menu;
  case VK_LWIN:
    return Qt::Key_Super_L;
  case VK_LMENU:
    return Qt::Key_Alt;
  case VK_CAPITAL:
    return Qt::Key_CapsLock;
  case VK_NUMLOCK:
    return Qt::Key_NumLock;
  case VK_SCROLL:
    return Qt::Key_ScrollLock;
  case VK_RWIN:
    return Qt::Key_Super_R;
  case VK_HELP:
    return Qt::Key_Help;
  case VK_SPACE:
    return Qt::Key_Space;
  case VK_1:
    return Qt::Key_Exclam;
  case VK_OEM_7:
    // Same WebEventKeyCode for Qt::Key_Apostrophe
    return Qt::Key_QuoteDbl;
  case VK_3:
    return Qt::Key_NumberSign;
  case VK_4:
    return Qt::Key_Dollar;
  case VK_5:
    return Qt::Key_Percent;
  case VK_7:
    return Qt::Key_Ampersand;
  case VK_9:
    return Qt::Key_ParenLeft;
  case VK_0:
    return Qt::Key_ParenRight;
  case VK_8:
    return Qt::Key_Asterisk;
  case VK_OEM_PLUS:
    // Same WebEventKeyCode for Qt::Key_Equal
    return Qt::Key_Plus;
  case VK_OEM_COMMA:
    // Same WebEventKeyCode for Qt::Key_Less
    return Qt::Key_Comma;
  case VK_OEM_MINUS:
    // Same WebEventKeyCode for Qt::Key_Underscore
    return Qt::Key_Minus;
  case VK_OEM_PERIOD:
    // Same WebEventKeyCode for Qt::Key_Greater
    return Qt::Key_Period;
  case VK_OEM_2:
    // Same WebEventKeyCode for Qt::Key_Question
    return Qt::Key_Slash;
  case VK_OEM_1:
    // Same WebEventKeyCode for Qt::Key_Semicolon
    return Qt::Key_Colon;
  case VK_2:
    return Qt::Key_At;
  case VK_OEM_4:
    // Same WebEventKeyCode for Qt::Key_BraceLeft
    return Qt::Key_BracketLeft;
  case VK_OEM_5:
    // Same WebEventKeyCode for Qt::Key_Bar
    return Qt::Key_Backslash;
  case VK_OEM_6:
    // Same WebEventKeyCode for Qt::Key_BraceRight
    return Qt::Key_BracketRight;
  case VK_6:
    return Qt::Key_AsciiCircum;
  case VK_OEM_3:
    // Same WebEventKeyCode for Qt::Key_AsciiTilde
    return Qt::Key_QuoteLeft;
  case VK_BROWSER_BACK:
    return Qt::Key_Back;
  case VK_BROWSER_FORWARD:
    return Qt::Key_Forward;
  case VK_BROWSER_STOP:
    return Qt::Key_Stop;
  case VK_BROWSER_REFRESH:
    return Qt::Key_Refresh;
  case VK_VOLUME_DOWN:
    return Qt::Key_VolumeDown;
  case VK_VOLUME_MUTE:
    return Qt::Key_VolumeMute;
  case VK_VOLUME_UP:
    return Qt::Key_VolumeUp;
  case VK_MEDIA_PLAY_PAUSE:
    // Same WebEventKeyCode for Qt::Key_MediaPause
    // Same WebEventKeyCode for Qt::Key_MediaTogglePlayPause
    return Qt::Key_MediaPlay;
  case VK_MEDIA_STOP:
    return Qt::Key_MediaStop;
  case VK_MEDIA_PREV_TRACK:
    return Qt::Key_MediaPrevious;
  case VK_MEDIA_NEXT_TRACK:
    return Qt::Key_MediaNext;
  case VK_BROWSER_HOME:
    return Qt::Key_HomePage;
  case VK_BROWSER_FAVORITES:
    return Qt::Key_Favorites;
  case VK_BROWSER_SEARCH:
    return Qt::Key_Search;
  case VK_MEDIA_LAUNCH_MAIL:
    return Qt::Key_LaunchMail;
  case VK_MEDIA_LAUNCH_MEDIA_SELECT:
    return Qt::Key_LaunchMedia;
  case VK_MEDIA_LAUNCH_APP1:
    return Qt::Key_Launch0;
  case VK_MEDIA_LAUNCH_APP2:
    return Qt::Key_Launch1;
  default:
    break;
  }

  return 0;
}

void WebView::HandleKeyboardEvent(content::WebContents* source,
                                  const content::NativeWebKeyboardEvent& event) {
  QEvent::Type tp = QEvent::None;

  switch (event.type) {
  case blink::WebInputEvent::KeyDown:
    tp = QEvent::KeyPress;
    break;
  case blink::WebInputEvent::KeyUp:
    tp = QEvent::KeyRelease;
    break;
  default:
    NOTREACHED();
  }

  int key = WebEventKeyCodeToQKeyEventKeyCode(event);

  Qt::KeyboardModifiers qmodifiers;
  if (event.modifiers & blink::WebInputEvent::ShiftKey) {
    qmodifiers |= Qt::ShiftModifier;
  }
  if (event.modifiers & blink::WebInputEvent::ControlKey) {
    qmodifiers |= Qt::ControlModifier;
  }
  if (event.modifiers & blink::WebInputEvent::AltKey) {
    qmodifiers |= Qt::AltModifier;
  }
  if (event.modifiers & blink::WebInputEvent::MetaKey) {
    qmodifiers |= Qt::MetaModifier;
  }
  if (event.modifiers & blink::WebInputEvent::IsKeyPad) {
    qmodifiers |= Qt::KeypadModifier;
  }

  QString text = QString::fromUtf16(event.text, sizeof(event.text));

  bool autorep = false;
  if (event.modifiers & blink::WebInputEvent::IsAutoRepeat) {
    autorep = true;
  }

  QKeyEvent* qevent = new QKeyEvent(tp, key, qmodifiers, text, autorep);

  QCoreApplication::postEvent(adapterToQObject(adapter_), qevent);
}

} // namespace qt
} // namespace oxide
