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

#include "javascript_dialog_contents_helper.h"

#include <sstream>
#include <string>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/threading/thread_task_runner_handle.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/console_message_level.h"
#include "content/public/common/javascript_message_type.h"

#include "javascript_dialog_host.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(JavaScriptDialogContentsHelper);

namespace {

content::WebContents* g_last_focused_web_contents;

const char* ToDialogTypeString(content::JavaScriptMessageType type) {
  switch (type) {
    case content::JAVASCRIPT_MESSAGE_TYPE_ALERT:
      return "alert";
    case content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
      return "confirm";
    case content::JAVASCRIPT_MESSAGE_TYPE_PROMPT:
      return "prompt";
  }

  NOTREACHED();
  return "";
}

}

JavaScriptDialogContentsHelper::JavaScriptDialogContentsHelper(
    content::WebContents* contents)
    : content::WebContentsObserver(contents),
      factory_(nullptr),
      is_displaying_before_unload_dialog_(false),
      weak_ptr_factory_(this) {}

void JavaScriptDialogContentsHelper::ReportDialogUnsupported(
    content::JavaScriptMessageType type) const {
  std::ostringstream msg;
  msg << "A window." << ToDialogTypeString(type) << "() "
      << "dialog requested by this page was suppressed because the "
      << "embedding view does not support displaying dialogs";
  web_contents()->GetMainFrame()->AddMessageToConsole(
      content::CONSOLE_MESSAGE_LEVEL_WARNING,
      msg.str());
}

bool JavaScriptDialogContentsHelper::IsWebContentsForeground() const {
  content::RenderWidgetHostView* view =
      web_contents()->GetFullscreenRenderWidgetHostView();
  if (!view) {
    view = web_contents()->GetRenderWidgetHostView();
  }

  if (!view) {
    return false;
  }

  if (!view->HasFocus() && web_contents() != g_last_focused_web_contents) {
    return false;
  }

  return view->IsShowing();
}

void JavaScriptDialogContentsHelper::DismissActiveDialog() {
  if (!active_dialog_) {
    DCHECK(!is_displaying_before_unload_dialog_);
    return;
  }

  DCHECK(!pending_dialog_request_data_);

  active_dialog_->Dismiss();
  DCHECK(!active_dialog_);

  is_displaying_before_unload_dialog_ = false;
}

void JavaScriptDialogContentsHelper::DismissPendingOrActiveDialog() {
  if (pending_dialog_request_data_) {
    DCHECK(!active_dialog_);
    DCHECK(!is_displaying_before_unload_dialog_);
    pending_dialog_request_data_.reset();
  } else {
    DismissActiveDialog();
  }
}

void JavaScriptDialogContentsHelper::RunPendingDialog(
    const DialogClosedCallback& callback) {
  DCHECK(pending_dialog_request_data_);

  std::unique_ptr<DialogRequestData> data =
      std::move(pending_dialog_request_data_);

  active_dialog_ =
      base::MakeUnique<JavaScriptDialogHost>(
          GetWeakPtr(),
          data->origin_url,
          false,
          data->type,
          data->message_text,
          data->default_prompt_text,
          base::Bind(&JavaScriptDialogContentsHelper::OnDialogClosed,
                     base::Unretained(this), callback));

  if (!active_dialog_->Show()) {
    DCHECK(!active_dialog_);
    ReportDialogUnsupported(data->type);
  }
}

void JavaScriptDialogContentsHelper::OnDialogClosed(
    const DialogClosedCallback& callback,
    bool success,
    const base::string16& user_input) {
  DCHECK(active_dialog_);
  DCHECK(!pending_dialog_request_data_);

  if (!callback.is_null()) {
    callback.Run(success, user_input);
  }

  base::ThreadTaskRunnerHandle::Get()->DeleteSoon(FROM_HERE,
                                                  active_dialog_.release());
  is_displaying_before_unload_dialog_ = false;
}

void JavaScriptDialogContentsHelper::HandleFocusOrVisibilityChange() {
  if (IsWebContentsForeground()) {
    if (pending_dialog_request_data_) {
      DCHECK(!active_dialog_);
      DCHECK(!is_displaying_before_unload_dialog_);

      RunPendingDialog();
    }
  } else if (active_dialog_ && !is_displaying_before_unload_dialog_) {
    DismissActiveDialog();
  }
}

void JavaScriptDialogContentsHelper::RunJavaScriptDialog(
    content::WebContents* source_web_contents,
    const GURL& origin_url,
    content::JavaScriptMessageType javascript_message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const DialogClosedCallback& callback,
    bool* did_suppress_message) {
  DCHECK_EQ(source_web_contents, web_contents());

  *did_suppress_message = false;

  if (is_displaying_before_unload_dialog_) {
    *did_suppress_message = true;
    return;
  }

  if (!factory_) {
    *did_suppress_message = true;
    ReportDialogUnsupported(javascript_message_type);
    return;
  }

  bool is_web_contents_foreground = IsWebContentsForeground();

  if (is_web_contents_foreground ||
      javascript_message_type == content::JAVASCRIPT_MESSAGE_TYPE_ALERT) {
    DismissPendingOrActiveDialog();

    pending_dialog_request_data_ = base::MakeUnique<DialogRequestData>();
    pending_dialog_request_data_->origin_url = origin_url;
    pending_dialog_request_data_->type = javascript_message_type;
    pending_dialog_request_data_->message_text = message_text;
    pending_dialog_request_data_->default_prompt_text = default_prompt_text;
  } else {
    // Suppress window.prompt and window.confirm dialogs from background views
    *did_suppress_message = true;
    std::ostringstream msg;
    msg << "A window." << ToDialogTypeString(javascript_message_type) << "() "
        << "dialog requested by this page was suppressed because the "
        << "embedding view does not have active focus. Please ensure that "
        << "dialogs are requested based on user interactions";
    web_contents()->GetMainFrame()->AddMessageToConsole(
        content::CONSOLE_MESSAGE_LEVEL_WARNING, msg.str());
    return;
  }

  if (!is_web_contents_foreground) {
    DCHECK_EQ(javascript_message_type,
              content::JAVASCRIPT_MESSAGE_TYPE_ALERT);
    // Queue window.alert dialogs from background views, but don't block
    // script execution, as this might block the foreground view
    callback.Run(true, base::string16());
    return;
  }

  RunPendingDialog(callback);
}

void JavaScriptDialogContentsHelper::RunBeforeUnloadDialog(
    content::WebContents* source_web_contents,
    bool is_reload,
    bool is_renderer_initiated,
    bool has_user_gesture,
    const DialogClosedCallback& callback,
    bool* did_suppress_dialog) {
  DCHECK_EQ(source_web_contents, web_contents());

  *did_suppress_dialog = false;

  if (is_renderer_initiated) {
    if (!has_user_gesture) {
      *did_suppress_dialog = true;
      return;
    } else if (!IsWebContentsForeground()) {
      callback.Run(false, base::string16());
      return;
    }
  }

  std::ostringstream unsupported_msg;
  unsupported_msg << "A beforeunload dialog requested by this page was "
                  << "suppressed because the embedding view does not support "
                  << "displaying dialogs";

  if (!factory_) {
    *did_suppress_dialog = true;
    web_contents()->GetMainFrame()->AddMessageToConsole(
        content::CONSOLE_MESSAGE_LEVEL_WARNING,
        unsupported_msg.str());
    return;
  }

  DismissPendingOrActiveDialog();

  active_dialog_ =
      base::MakeUnique<JavaScriptDialogHost>(
          GetWeakPtr(),
          web_contents()->GetLastCommittedURL(),
          true,
          content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM,
          base::string16(),
          base::string16(),
          base::Bind(&JavaScriptDialogContentsHelper::OnDialogClosed,
                     base::Unretained(this), callback));

  if (!active_dialog_->Show()) {
    DCHECK(!active_dialog_);
    web_contents()->GetMainFrame()->AddMessageToConsole(
        content::CONSOLE_MESSAGE_LEVEL_WARNING,
        unsupported_msg.str());
    return;
  }

  is_displaying_before_unload_dialog_ = true;
}

bool JavaScriptDialogContentsHelper::HandleJavaScriptDialog(
    content::WebContents* source_web_contents,
    bool accept,
    const base::string16* prompt_override) {
  DCHECK_EQ(source_web_contents, web_contents());

  if (!active_dialog_) {
    return false;
  }

  active_dialog_->Handle(accept, prompt_override);
  DCHECK(!active_dialog_);

  return true;
}

void JavaScriptDialogContentsHelper::CancelDialogs(
    content::WebContents* web_contents,
    bool suppress_callbacks,
    bool reset_state) {
  DismissPendingOrActiveDialog();
}

void JavaScriptDialogContentsHelper::WasShown() {
  HandleFocusOrVisibilityChange();
}

void JavaScriptDialogContentsHelper::WasHidden() {
  HandleFocusOrVisibilityChange();
}

void JavaScriptDialogContentsHelper::WebContentsDestroyed() {
  if (web_contents() == g_last_focused_web_contents) {
    g_last_focused_web_contents = nullptr;
  }
}

void JavaScriptDialogContentsHelper::OnWebContentsFocused() {
  content::WebContents* last_focused_contents = g_last_focused_web_contents;
  if (last_focused_contents == web_contents()) {
    return;
  }

  g_last_focused_web_contents = web_contents();

  if (last_focused_contents) {
    JavaScriptDialogContentsHelper* last_focused_helper =
        JavaScriptDialogContentsHelper::FromWebContents(last_focused_contents);
    if (last_focused_helper) {
      last_focused_helper->HandleFocusOrVisibilityChange();
    }
  }

  HandleFocusOrVisibilityChange();
}

// static
void JavaScriptDialogContentsHelper::CreateForWebContents(
    content::WebContents* contents) {
  return content::WebContentsUserData<JavaScriptDialogContentsHelper>
      ::CreateForWebContents(contents);
}

// static
JavaScriptDialogContentsHelper* JavaScriptDialogContentsHelper::FromWebContents(
    content::WebContents* contents) {
  return content::WebContentsUserData<JavaScriptDialogContentsHelper>
      ::FromWebContents(contents);
}

JavaScriptDialogContentsHelper::~JavaScriptDialogContentsHelper() = default;

base::WeakPtr<JavaScriptDialogContentsHelper>
JavaScriptDialogContentsHelper::GetWeakPtr() {
  return weak_ptr_factory_.GetWeakPtr();
}

} // namespace oxide
