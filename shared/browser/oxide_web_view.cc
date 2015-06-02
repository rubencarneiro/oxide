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

#include "oxide_web_view.h"

#include <queue>
#include <utility>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/supports_user_data.h"
#include "components/sessions/content/content_serialized_navigation_builder.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/renderer_host/event_with_latency_info.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/renderer_host/ui_events_helper.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/browser/web_contents/web_contents_view.h"
#include "content/public/browser/invalidate_type.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/media_capture_devices.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/page_navigator.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/resource_request_details.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/favicon_url.h"
#include "content/public/common/file_chooser_file_info.h"
#include "content/public/common/file_chooser_params.h"
#include "content/public/common/media_stream_request.h"
#include "content/public/common/menu_item.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/web_preferences.h"
#include "ipc/ipc_message_macros.h"
#include "net/base/net_errors.h"
#include "net/ssl/ssl_info.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "ui/base/window_open_disposition.h"
#include "ui/events/event.h"
#include "ui/gfx/range/range.h"
#include "ui/gl/gl_implementation.h"
#include "ui/shell_dialogs/selected_file_info.h"
#include "url/gurl.h"
#include "url/url_constants.h"

#include "shared/browser/compositor/oxide_compositor.h"
#include "shared/browser/compositor/oxide_compositor_frame_handle.h"
#include "shared/common/oxide_content_client.h"
#include "shared/common/oxide_enum_flags.h"
#include "shared/common/oxide_messages.h"

#include "third_party/WebKit/public/web/WebFindOptions.h"

#include "oxide_browser_context.h"
#include "oxide_browser_process_main.h"
#include "oxide_content_browser_client.h"
#include "oxide_event_utils.h"
#include "oxide_file_picker.h"
#include "oxide_javascript_dialog_manager.h"
#include "oxide_render_widget_host_view.h"
#include "oxide_web_contents_unloader.h"
#include "oxide_web_contents_view.h"
#include "oxide_web_context_menu.h"
#include "oxide_web_frame.h"
#include "oxide_web_popup_menu.h"
#include "oxide_web_preferences.h"
#include "oxide_web_view_client.h"
#include "oxide_web_view_contents_helper.h"

#if defined(ENABLE_MEDIAHUB)
#include "shared/browser/media/oxide_media_web_contents_observer.h"
#endif

#define DCHECK_VALID_SOURCE_CONTENTS DCHECK_EQ(source, web_contents());

namespace oxide {

namespace {

const char kWebViewKey[] = "oxide_web_view_data";

// SupportsUserData implementations own their data. This class exists
// because we don't want WebContents to own WebView (it's the other way
// around)
class WebViewUserData : public base::SupportsUserData::Data {
 public:
  WebViewUserData(WebView* view) : view_(view) {}

  WebView* get() const { return view_; }

 private:
  WebView* view_;
};

void FillLoadURLParamsFromOpenURLParams(
    content::NavigationController::LoadURLParams* load_params,
    const content::OpenURLParams& params) {
  load_params->transition_type = params.transition;
  load_params->frame_tree_node_id = params.frame_tree_node_id;
  load_params->referrer = params.referrer;
  load_params->redirect_chain = params.redirect_chain;
  load_params->extra_headers = params.extra_headers;
  load_params->is_renderer_initiated = params.is_renderer_initiated;
  load_params->transferred_global_request_id =
      params.transferred_global_request_id;
  load_params->should_replace_current_entry =
      params.should_replace_current_entry;

  if (params.uses_post && !params.is_renderer_initiated) {
    load_params->load_type =
        content::NavigationController::LOAD_TYPE_BROWSER_INITIATED_HTTP_POST;
    load_params->browser_initiated_post_data =
        params.browser_initiated_post_data;
  }
}

void InitCreatedWebView(WebView* view,
                        scoped_ptr<content::WebContents> contents) {
  WebView::Params params;
  params.contents = contents.Pass();

  view->Init(&params);
}

// Qt input methods don’t generate key events, but a lot of web pages out there
// rely on keydown and keyup events to e.g. perform search-as-you-type or
// enable/disable a submit button based on the contents of a text input field,
// so we send a fake pair of keydown/keyup events.
// This mimicks what is done in GtkIMContextWrapper::HandlePreeditChanged(…)
// and GtkIMContextWrapper::HandleCommit(…)
// (see content/browser/renderer_host/gtk_im_context_wrapper.cc).
void SendFakeCompositionKeyEvent(content::RenderWidgetHostImpl* host,
                                 blink::WebInputEvent::Type type) {
  content::NativeWebKeyboardEvent fake_event;
  fake_event.windowsKeyCode = ui::VKEY_PROCESSKEY;
  fake_event.skip_in_browser = true;
  fake_event.type = type;
  host->ForwardKeyboardEvent(fake_event);
}

void CreateHelpers(content::WebContents* contents,
                   WebViewContentsHelper* opener = nullptr) {
  if (!opener) {
    new WebViewContentsHelper(contents);
  }
  else {
    new WebViewContentsHelper(contents, opener);
  }

#if defined(ENABLE_MEDIAHUB)
  new MediaWebContentsObserver(contents);
#endif
}


OXIDE_MAKE_ENUM_BITWISE_OPERATORS(ContentType)

base::LazyInstance<std::vector<WebView*> > g_all_web_views;

}

void WebView::WebFrameDeleter::operator()(WebFrame* frame) {
  WebFrame::Destroy(frame);
}

void WebView::WebContentsDeleter::operator()(content::WebContents* contents) {
  WebContentsUnloader::GetInstance()->Unload(make_scoped_ptr(contents));
};

WebViewIterator::WebViewIterator(const std::vector<WebView*>& views) {
  for (std::vector<WebView*>::const_iterator it = views.begin();
       it != views.end(); ++it) {
    views_.push_back((*it)->AsWeakPtr());
  }
  current_ = views_.begin();
}

WebViewIterator::~WebViewIterator() {}

bool WebViewIterator::HasMore() const {
  return current_ != views_.end();
}

WebView* WebViewIterator::GetNext() {
  while (current_ != views_.end()) {
    base::WeakPtr<WebView>& view = *current_;
    current_++;
    if (view.get()) {
      return view.get();
    }
  }

  return nullptr;
}

WebView::Params::Params()
    : context(nullptr),
      incognito(false) {}
WebView::Params::~Params() {}

// static
WebViewIterator WebView::GetAllWebViews() {
  return WebViewIterator(g_all_web_views.Get());
}

RenderWidgetHostView* WebView::GetRenderWidgetHostView() const {
  if (!web_contents_) {
    return nullptr;
  }

  return static_cast<RenderWidgetHostView *>(
      web_contents_->GetRenderWidgetHostView());
}

content::RenderViewHost* WebView::GetRenderViewHost() const {
  if (!web_contents_) {
    return nullptr;
  }

  return web_contents_->GetRenderViewHost();
}

content::RenderWidgetHostImpl* WebView::GetRenderWidgetHostImpl() const {
  if (!web_contents_) {
    return nullptr;
  }

  return content::RenderWidgetHostImpl::From(
      web_contents_->GetRenderViewHost());
}

void WebView::DispatchLoadFailed(const GURL& validated_url,
                                 int error_code,
                                 const base::string16& error_description,
                                 bool is_provisional_load) {
  if (error_code == net::ERR_ABORTED) {
    client_->LoadStopped(validated_url);
  } else {
    content::NavigationEntry* entry =
      web_contents_->GetController().GetLastCommittedEntry();
    client_->LoadFailed(validated_url,
                        error_code,
                        base::UTF16ToUTF8(error_description),
                        is_provisional_load ?
                            0 : entry->GetHttpStatusCode());
  }
}

void WebView::OnDidBlockDisplayingInsecureContent() {
  if (blocked_content_ & CONTENT_TYPE_MIXED_DISPLAY) {
    return;
  }

  blocked_content_ |= CONTENT_TYPE_MIXED_DISPLAY;

  client_->ContentBlocked();
}

void WebView::OnDidBlockRunningInsecureContent() {
  if (blocked_content_ & CONTENT_TYPE_MIXED_SCRIPT) {
    return;
  }

  blocked_content_ |= CONTENT_TYPE_MIXED_SCRIPT;

  client_->ContentBlocked();
}

bool WebView::ShouldScrollFocusedEditableNodeIntoView() {
  if (!HasFocus()) {
    return false;
  }

  if (!IsVisible()) {
    return false;
  }

  if (!IsInputPanelVisible()) {
    return false;
  }

  if (!focused_node_is_editable_) {
    return false;
  }

  return true;
}

void WebView::MaybeScrollFocusedEditableNodeIntoView() {
  if (!ShouldScrollFocusedEditableNodeIntoView()) {
    return;
  }

  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host) {
    return;
  }

  host->ScrollFocusedEditableNodeIntoRect(GetViewBoundsDip());
}

float WebView::GetFrameMetadataScaleToPix() {
  return compositor_frame_metadata().device_scale_factor *
         compositor_frame_metadata().page_scale_factor;
}

void WebView::InitializeTopControlsForHost(content::RenderViewHost* rvh,
                                           bool initial_host) {
  // Show the location bar if this is the initial RVH and the constraints
  // are set to blink::WebTopControlsBoth
  blink::WebTopControlsState current = location_bar_constraints_;
  if (initial_host &&
      location_bar_constraints_ == blink::WebTopControlsBoth) {
    current = blink::WebTopControlsShown;
  }

  rvh->Send(
      new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                          location_bar_constraints_,
                                          current,
                                          false));
}

WebFrame* WebView::CreateWebFrame(
    content::RenderFrameHost* render_frame_host) {
  WebFrame* frame = client_->CreateWebFrame(render_frame_host);
  if (frame) {
    return frame;
  }

  return new WebFrame(render_frame_host, this);
}

void WebView::DispatchPrepareToCloseResponse(bool proceed) {
  client_->PrepareToCloseResponseReceived(proceed);
}

size_t WebView::GetScriptMessageHandlerCount() const {
  return client_->GetScriptMessageHandlerCount();
}

const ScriptMessageHandler* WebView::GetScriptMessageHandlerAt(
    size_t index) const {
  return client_->GetScriptMessageHandlerAt(index);
}

void WebView::CompositorDidCommit() {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  pending_compositor_frame_metadata_ = rwhv->compositor_frame_metadata();

  rwhv->CompositorDidCommit();
}

void WebView::CompositorSwapFrame(uint32 surface_id,
                                  CompositorFrameHandle* frame) {
  received_surface_ids_.push(surface_id);

  if (current_compositor_frame_.get()) {
    previous_compositor_frames_.push_back(current_compositor_frame_);
  }
  current_compositor_frame_ = frame;

  cc::CompositorFrameMetadata old = compositor_frame_metadata_;
  compositor_frame_metadata_ = pending_compositor_frame_metadata_;

  // TODO(chrisccoulson): Merge these
  client_->FrameMetadataUpdated(old);
  client_->SwapCompositorFrame();
}

void WebView::WebPreferencesDestroyed() {
  client_->WebPreferencesDestroyed();
}

void WebView::Observe(int type,
                      const content::NotificationSource& source,
                      const content::NotificationDetails& details) {
  if (content::Source<content::NavigationController>(source).ptr() !=
      &GetWebContents()->GetController()) {
    return;
  }
  if (type == content::NOTIFICATION_NAV_LIST_PRUNED) {
    content::PrunedDetails* pruned_details =
        content::Details<content::PrunedDetails>(details).ptr();
    client_->NavigationListPruned(pruned_details->from_front,
                                  pruned_details->count);
  } else if (type == content::NOTIFICATION_NAV_ENTRY_CHANGED) {
    int index =
        content::Details<content::EntryChangedDetails>(details).ptr()->index;
    client_->NavigationEntryChanged(index);
  }
}

void WebView::EvictCurrentFrame() {
  current_compositor_frame_ = nullptr;
  client_->EvictCurrentFrame();
}

void WebView::UpdateCursor(const content::WebCursor& cursor) {
  client_->UpdateCursor(cursor);
}

void WebView::TextInputStateChanged(ui::TextInputType type,
                                    bool show_ime_if_needed) {
  if (type == text_input_type_ &&
      show_ime_if_needed == show_ime_if_needed_) {
    return;
  }

  text_input_type_ = type;
  show_ime_if_needed_ = show_ime_if_needed;

  client_->TextInputStateChanged();
}

void WebView::FocusedNodeChanged(bool is_editable_node) {
  focused_node_is_editable_ = is_editable_node;
  client_->FocusedNodeChanged();

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebView::ImeCancelComposition() {
  client_->ImeCancelComposition();
}

void WebView::SelectionBoundsChanged(const gfx::Rect& caret_rect,
                                     size_t selection_cursor_position,
                                     size_t selection_anchor_position) {
  if (caret_rect == caret_rect_ &&
      selection_cursor_position == selection_cursor_position_ &&
      selection_anchor_position == selection_anchor_position_) {
    return;
  }

  caret_rect_ = caret_rect;
  selection_cursor_position_ = selection_cursor_position;
  selection_anchor_position_ = selection_anchor_position;

  client_->SelectionBoundsChanged();
}

void WebView::SelectionChanged() {
  client_->SelectionChanged();
}

Compositor* WebView::GetCompositor() const {
  return compositor_.get();
}

content::WebContents* WebView::OpenURLFromTab(
    content::WebContents* source,
    const content::OpenURLParams& params) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (params.disposition != CURRENT_TAB &&
      params.disposition != NEW_FOREGROUND_TAB &&
      params.disposition != NEW_BACKGROUND_TAB &&
      params.disposition != NEW_POPUP &&
      params.disposition != NEW_WINDOW) {
    return nullptr;
  }

  // Block popups
  if ((params.disposition == NEW_FOREGROUND_TAB ||
       params.disposition == NEW_BACKGROUND_TAB ||
       params.disposition == NEW_WINDOW ||
       params.disposition == NEW_POPUP) &&
      !params.user_gesture && GetBrowserContext()->IsPopupBlockerEnabled()) {
    return nullptr;
  }

  WindowOpenDisposition disposition = params.disposition;
  content::OpenURLParams local_params(params);

  // Coerce all non CURRENT_TAB navigations that don't come from a user
  // gesture to NEW_POPUP
  if (disposition != CURRENT_TAB && !local_params.user_gesture) {
    disposition = NEW_POPUP;
  }

  // If we can't create new windows, this should be a CURRENT_TAB navigation
  // in the top-level frame
  if (!CanCreateWindows() && disposition != CURRENT_TAB) {
    disposition = CURRENT_TAB;
    local_params.frame_tree_node_id = -1;
  }

  // Navigations in a new window are always in the root frame
  if (disposition != CURRENT_TAB) {
    local_params.frame_tree_node_id = -1;
  }

  // Determine if this is a top-level navigation
  bool top_level = params.frame_tree_node_id == -1;
  if (!top_level) {
    WebFrame* frame = WebFrame::FromFrameTreeNodeID(params.frame_tree_node_id);
    DCHECK(frame);
    top_level = frame->parent() == nullptr;
  }

  // Give the application a chance to block the navigation if it is
  // renderer initiated and it's a top-level navigation or requires a
  // new webview
  if (local_params.is_renderer_initiated &&
      top_level &&
      !client_->ShouldHandleNavigation(local_params.url,
                                       disposition,
                                       local_params.user_gesture)) {
    return nullptr;
  }

  if (disposition == CURRENT_TAB) {
    content::NavigationController::LoadURLParams load_params(local_params.url);
    FillLoadURLParamsFromOpenURLParams(&load_params, local_params);

    web_contents_->GetController().LoadURLWithParams(load_params);

    return web_contents_.get();
  }

  // XXX(chrisccoulson): Is there a way to tell when the opener shouldn't
  // be suppressed?
  bool opener_suppressed = true;

  content::WebContents::CreateParams contents_params(
      GetBrowserContext(),
      opener_suppressed ? nullptr : web_contents_->GetSiteInstance());
  contents_params.initial_size = GetViewSizeDip();
  contents_params.initially_hidden = disposition == NEW_BACKGROUND_TAB;
  contents_params.opener = opener_suppressed ? nullptr : web_contents_.get();

  scoped_ptr<content::WebContents> contents(
      content::WebContents::Create(contents_params));
  if (!contents) {
    LOG(ERROR) << "Failed to create new WebContents for navigation";
    return nullptr;
  }

  CreateHelpers(contents.get(), web_contents_helper_);

  WebView* new_view =
      client_->CreateNewWebView(GetViewBoundsPix(), disposition);
  if (!new_view) {
    return nullptr;
  }

  InitCreatedWebView(new_view, contents.Pass());

  content::NavigationController::LoadURLParams load_params(local_params.url);
  FillLoadURLParamsFromOpenURLParams(&load_params, local_params);

  new_view->GetWebContents()->GetController().LoadURLWithParams(load_params);

  return new_view->GetWebContents();
}

void WebView::NavigationStateChanged(content::WebContents* source,
                                     content::InvalidateTypes changed_flags) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (changed_flags & content::INVALIDATE_TYPE_URL) {
    client_->URLChanged();
  }

  if (changed_flags & content::INVALIDATE_TYPE_TITLE) {
    client_->TitleChanged();
  }

  if (changed_flags & (content::INVALIDATE_TYPE_URL |
                       content::INVALIDATE_TYPE_LOAD)) {
    client_->CommandsUpdated();
  }
}

void WebView::VisibleSSLStateChanged(const content::WebContents* source) {
  DCHECK_VALID_SOURCE_CONTENTS

  content::NavigationEntry* entry =
      web_contents_->GetController().GetVisibleEntry();
  if (!entry) {
    return;
  }

  SecurityStatus old_status = security_status_;
  security_status_.Update(entry->GetSSL());

  client_->SecurityStatusChanged(old_status);
}

bool WebView::ShouldCreateWebContents(
    content::WebContents* source,
    int route_id,
    int main_frame_route_id,
    WindowContainerType window_container_type,
    const base::string16& frame_name,
    const GURL& target_url,
    const std::string& partition_id,
    content::SessionStorageNamespace* session_storage_namespace,
    WindowOpenDisposition disposition,
    bool user_gesture) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (disposition != NEW_FOREGROUND_TAB &&
      disposition != NEW_BACKGROUND_TAB &&
      disposition != NEW_POPUP &&
      disposition != NEW_WINDOW) {
    return false;
  }

  // Note that popup blocking was done on the IO thread

  if (!CanCreateWindows()) {
    return false;
  }

  return client_->ShouldHandleNavigation(
      target_url,
      user_gesture ? disposition : NEW_POPUP,
      user_gesture);
}

void WebView::HandleKeyboardEvent(
    content::WebContents* source,
    const content::NativeWebKeyboardEvent& event) {
  DCHECK_VALID_SOURCE_CONTENTS

  client_->UnhandledKeyboardEvent(event);
}

void WebView::WebContentsCreated(content::WebContents* source,
                                 int source_frame_id,
                                 const base::string16& frame_name,
                                 const GURL& target_url,
                                 content::WebContents* new_contents) {
  DCHECK_VALID_SOURCE_CONTENTS
  DCHECK(!WebView::FromWebContents(new_contents));

  CreateHelpers(new_contents, web_contents_helper_);
}

void WebView::AddNewContents(content::WebContents* source,
                             content::WebContents* new_contents,
                             WindowOpenDisposition disposition,
                             const gfx::Rect& initial_pos,
                             bool user_gesture,
                             bool* was_blocked) {
  DCHECK_VALID_SOURCE_CONTENTS
  DCHECK(disposition == NEW_FOREGROUND_TAB ||
         disposition == NEW_BACKGROUND_TAB ||
         disposition == NEW_POPUP ||
         disposition == NEW_WINDOW) << "Invalid disposition";
  DCHECK_EQ(GetBrowserContext(),
            BrowserContext::FromContent(new_contents->GetBrowserContext()));

  if (was_blocked) {
    *was_blocked = true;
  }

  scoped_ptr<content::WebContents> contents(new_contents);

  WebView* new_view =
      client_->CreateNewWebView(initial_pos,
                                user_gesture ? disposition : NEW_POPUP);
  if (!new_view) {
    return;
  }

  InitCreatedWebView(new_view, contents.Pass());

  if (was_blocked) {
    *was_blocked = false;
  }
}

void WebView::LoadProgressChanged(content::WebContents* source,
                                  double progress) {
  DCHECK_VALID_SOURCE_CONTENTS

  // XXX: Is there a way to update this when we adopt a WebContents?
  client_->LoadProgressChanged(progress);
}

void WebView::CloseContents(content::WebContents* source) {
  DCHECK_VALID_SOURCE_CONTENTS

  client_->CloseRequested();
}

bool WebView::AddMessageToConsole(content::WebContents* source,
                                  int32 level,
                                  const base::string16& message,
                                  int32 line_no,
                                  const base::string16& source_id) {
  DCHECK_VALID_SOURCE_CONTENTS

  return client_->AddMessageToConsole(level, message, line_no, source_id);
}

void WebView::BeforeUnloadFired(content::WebContents* source,
                                bool proceed,
                                bool* proceed_to_fire_unload) {
  DCHECK_VALID_SOURCE_CONTENTS

  // We take care of the unload handler on deletion
  *proceed_to_fire_unload = false;

  client_->PrepareToCloseResponseReceived(proceed);
}

content::JavaScriptDialogManager* WebView::GetJavaScriptDialogManager(
    content::WebContents* source) {
  DCHECK_VALID_SOURCE_CONTENTS
  return JavaScriptDialogManager::GetInstance();
}

void WebView::RunFileChooser(content::WebContents* source,
                             const content::FileChooserParams& params) {
  DCHECK_VALID_SOURCE_CONTENTS

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  FilePicker* file_picker = client_->CreateFilePicker(rvh);
  if (!file_picker) {
    std::vector<content::FileChooserFileInfo> empty;
    rvh->FilesSelectedInChooser(empty, params.mode);
    return;
  }

  file_picker->Run(params);
}

void WebView::EnterFullscreenModeForTab(content::WebContents* source,
                                        const GURL& origin) {
  DCHECK_VALID_SOURCE_CONTENTS

  client_->ToggleFullscreenMode(true);
}

void WebView::ExitFullscreenModeForTab(content::WebContents* source) {
  DCHECK_VALID_SOURCE_CONTENTS

  client_->ToggleFullscreenMode(false);
}

bool WebView::IsFullscreenForTabOrPending(
    const content::WebContents* source) const {
  DCHECK_VALID_SOURCE_CONTENTS

  return IsFullscreen();
}

void WebView::FindReply(content::WebContents* source,
                        int request_id,
                        int number_of_matches,
                        const gfx::Rect& selection_rect,
                        int active_match_ordinal,
                        bool final_update) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (active_match_ordinal != -1 &&
      find_in_page_.current != active_match_ordinal)  {
      find_in_page_.current = active_match_ordinal;
      client_->FindInPageCurrentChanged();
  }

  if (number_of_matches != -1 &&
      find_in_page_.count != number_of_matches)  {
      find_in_page_.count = number_of_matches;
      client_->FindInPageCountChanged();
  }
}

void WebView::RequestMediaAccessPermission(
    content::WebContents* source,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback) {
  DCHECK_VALID_SOURCE_CONTENTS

  if (request.video_type == content::MEDIA_DEVICE_AUDIO_OUTPUT ||
      request.audio_type == content::MEDIA_DEVICE_AUDIO_OUTPUT) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_INVALID_STATE,
                 nullptr);
    return;
  }

  if (request.video_type == content::MEDIA_NO_SERVICE &&
      request.audio_type == content::MEDIA_NO_SERVICE) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_INVALID_STATE,
                 nullptr);
    return;
  }

  // Desktop / tab capture not supported
  if (request.video_type == content::MEDIA_DESKTOP_VIDEO_CAPTURE ||
      request.audio_type == content::MEDIA_DESKTOP_AUDIO_CAPTURE ||
      request.video_type == content::MEDIA_TAB_VIDEO_CAPTURE ||
      request.audio_type == content::MEDIA_TAB_AUDIO_CAPTURE) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_NOT_SUPPORTED,
                 nullptr);
    return;
  }

  // Only MEDIA_GENERATE_STREAM is valid here - MEDIA_DEVICE_ACCESS doesn't
  // come from media stream, MEDIA_ENUMERATE_DEVICES doesn't trigger a
  // permission request and MEDIA_OPEN_DEVICE is used from pepper
  if (request.request_type != content::MEDIA_GENERATE_STREAM) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_NOT_SUPPORTED,
                 nullptr);
    return;
  }

  content::MediaCaptureDevices* devices =
      content::MediaCaptureDevices::GetInstance();

  if (request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE &&
      devices->GetAudioCaptureDevices().empty()) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_NO_HARDWARE,
                 nullptr);
    return;
  }

  if (request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE &&
      devices->GetVideoCaptureDevices().empty()) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_NO_HARDWARE,
                 nullptr);
    return;
  }

  content::RenderFrameHost* rfh =
      content::RenderFrameHost::FromID(request.render_process_id,
                                       request.render_frame_id);
  if (!rfh) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 nullptr);
    return;
  }

  WebFrame* frame = WebFrame::FromRenderFrameHost(rfh);
  if (!frame) {
    callback.Run(content::MediaStreamDevices(),
                 content::MEDIA_DEVICE_PERMISSION_DENIED,
                 nullptr);
    return;
  }

  scoped_ptr<MediaAccessPermissionRequest> req(
      new MediaAccessPermissionRequest(
        &permission_request_manager_,
        frame,
        request.security_origin,
        web_contents_->GetLastCommittedURL().GetOrigin(),
        request.audio_type == content::MEDIA_DEVICE_AUDIO_CAPTURE,
        request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE,
        callback));

  client_->RequestMediaAccessPermission(req.Pass());
}

void WebView::RenderFrameCreated(content::RenderFrameHost* render_frame_host) {
  // We get a RenderFrameHostChanged notification when any FrameTreeNode is
  // added to the FrameTree, which is when we want a notification. However,
  // we get that before the FrameTreeNode has its parent set, so we still
  // have to use RenderFrameCreated to add new nodes correctly

  if (WebFrame::FromRenderFrameHost(render_frame_host)) {
    // We already have a WebFrame for this host. This could be because the new
    // host is for the root frame, or it is a cross-process subframe
    DCHECK(!render_frame_host->GetParent() ||
           render_frame_host->IsCrossProcessSubframe());
    return;
  }

  WebFrame* parent =
      WebFrame::FromRenderFrameHost(render_frame_host->GetParent());
  DCHECK(parent);

  WebFrame* frame = CreateWebFrame(render_frame_host);
  DCHECK(frame);
  frame->InitParent(parent);
}

void WebView::RenderViewReady() {
  client_->CrashedStatusChanged();
}

void WebView::RenderProcessGone(base::TerminationStatus status) {
  client_->CrashedStatusChanged();
}

void WebView::RenderViewHostChanged(content::RenderViewHost* old_host,
                                    content::RenderViewHost* new_host) {
  if (old_host && old_host->GetView()) {
    static_cast<RenderWidgetHostView *>(old_host->GetView())->SetDelegate(nullptr);
  }
  if (new_host) {
    if (new_host->GetView()) {
      static_cast<RenderWidgetHostView *>(new_host->GetView())->SetDelegate(this);
    }

    InitializeTopControlsForHost(new_host, !old_host);
  }
}

void WebView::RenderFrameHostChanged(content::RenderFrameHost* old_host,
                                     content::RenderFrameHost* new_host) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(new_host);

  if (frame) {
    frame->SetRenderFrameHost(new_host);
    return;
  }

#if 0
  WebFrame* parent =
      WebFrame::FromRenderFrameHost(new_host->GetParent());
  DCHECK(parent);

  frame = CreateWebFrame(new_host);
  DCHECK(frame);
  frame->InitParent(parent);
#endif
}

void WebView::DidStartProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    bool is_error_frame,
    bool is_iframe_srcdoc) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (!frame) {
    return;
  }

  if (is_error_frame) {
    return;
  }

  if (!frame->parent()) {
    client_->LoadStarted(validated_url);
  }

  certificate_error_manager_.DidStartProvisionalLoadForFrame(frame);
}

void WebView::DidCommitProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& url,
    ui::PageTransition transition_type) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (frame) {
    frame->DidCommitNewURL();
  }

  if (frame->parent()) {
    return;
  }

  content::NavigationEntry* entry =
      web_contents_->GetController().GetLastCommittedEntry();
  client_->LoadCommitted(url,
                         entry->GetPageType() == content::PAGE_TYPE_ERROR,
                         entry->GetHttpStatusCode());
}

void WebView::DidFailProvisionalLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    int error_code,
    const base::string16& error_description) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (!frame) {
    return;
  }

  if (!frame->parent() &&
      validated_url.spec() != content::kUnreachableWebDataURL) {
    DispatchLoadFailed(validated_url, error_code, error_description, true);
  }

  if (error_code != net::ERR_ABORTED) {
    return;
  }

  certificate_error_manager_.DidStopProvisionalLoadForFrame(frame);
}

void WebView::DidNavigateMainFrame(
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  if (details.is_navigation_to_different_page()) {
    permission_request_manager_.CancelPendingRequests();

    blocked_content_ = CONTENT_TYPE_NONE;
    client_->ContentBlocked();

    RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
    if (rwhv) {
      rwhv->ResetGestureDetection();
    }
  }
}

void WebView::DidNavigateAnyFrame(
    content::RenderFrameHost* render_frame_host,
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (!frame) {
    return;
  }

  if (details.is_in_page) {
    return;
  }

  permission_request_manager_.CancelPendingRequestsForFrame(frame);
  certificate_error_manager_.DidNavigateFrame(frame);
}

void WebView::DidFinishLoad(content::RenderFrameHost* render_frame_host,
                            const GURL& validated_url) {
  if (render_frame_host->GetParent()) {
    return;
  }

  if (validated_url.spec() == content::kUnreachableWebDataURL) {
    return;
  }

  content::NavigationEntry* entry =
      web_contents_->GetController().GetLastCommittedEntry();
  client_->LoadSucceeded(validated_url, entry->GetHttpStatusCode());
}

void WebView::DidFailLoad(content::RenderFrameHost* render_frame_host,
                          const GURL& validated_url,
                          int error_code,
                          const base::string16& error_description) {
  if (render_frame_host->GetParent()) {
    return;
  }

  DispatchLoadFailed(validated_url, error_code, error_description);
}

void WebView::DidGetRedirectForResourceRequest(
      content::RenderFrameHost* render_frame_host,
      const content::ResourceRedirectDetails& details) {
  if (details.resource_type != content::RESOURCE_TYPE_MAIN_FRAME) {
    return;
  }

  client_->LoadRedirected(details.new_url,
                          details.original_url,
                          details.http_response_code);
}

void WebView::NavigationEntryCommitted(
    const content::LoadCommittedDetails& load_details) {
  client_->NavigationEntryCommitted();
}

void WebView::DidStartLoading() {
  client_->LoadingChanged();
}

void WebView::DidStopLoading() {
  client_->LoadingChanged();
}

void WebView::FrameDeleted(content::RenderFrameHost* render_frame_host) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(render_frame_host);
  if (!frame) {
    // When a frame is detached, we get notified before any of its children
    // are detached. If we hit this case, it means that this is a child of a
    // frame that's being detached, and we've already deleted the corresponding
    // WebFrame
    return;
  }

  // This is a bit of a hack, but we need to process children now - see the
  // comment above
  std::queue<WebFrame*> frames;
  frames.push(frame);
  while (!frames.empty()) {
    WebFrame* f = frames.front();
    for (size_t i = 0; i < f->GetChildCount(); ++i) {
      frames.push(f->GetChildAt(i));
    }
    certificate_error_manager_.FrameDetached(f);
    permission_request_manager_.CancelPendingRequestsForFrame(f);
    frames.pop();
  }

  WebFrame::Destroy(frame);
}

void WebView::TitleWasSet(content::NavigationEntry* entry, bool explicit_set) {
  if (!web_contents_) {
    return;
  }
  const content::NavigationController& controller = web_contents_->GetController();
  int count = controller.GetEntryCount();
  for (int i = 0; i < count; ++i) {
    if (controller.GetEntryAtIndex(i) == entry) {
      client_->NavigationEntryChanged(i);
      return;
    }
  }
}

void WebView::DidUpdateFaviconURL(
    const std::vector<content::FaviconURL>& candidates) {
  std::vector<content::FaviconURL>::const_iterator it;
  for (it = candidates.begin(); it != candidates.end(); ++it) {
    if (it->icon_type == content::FaviconURL::FAVICON) {
      client_->IconChanged(it->icon_url);
      return;
    }
  }
}

bool WebView::OnMessageReceived(const IPC::Message& msg,
                                content::RenderFrameHost* render_frame_host) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(WebView, msg)
    IPC_MESSAGE_HANDLER(OxideHostMsg_DidBlockDisplayingInsecureContent,
                        OnDidBlockDisplayingInsecureContent)
    IPC_MESSAGE_HANDLER(OxideHostMsg_DidBlockRunningInsecureContent,
                        OnDidBlockRunningInsecureContent)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

<<<<<<< TREE
void WebView::OnCrashedStatusChanged() {}

void WebView::OnURLChanged() {}
void WebView::OnTitleChanged() {}
void WebView::OnIconChanged(const GURL& icon) {}
void WebView::OnCommandsUpdated() {}

void WebView::OnLoadingChanged() {}
void WebView::OnLoadProgressChanged(double progress) {}

void WebView::OnLoadStarted(const GURL& validated_url) {}
void WebView::OnLoadRedirected(const GURL& url,
                               const GURL& original_url,
                               int http_status_code) {}
void WebView::OnLoadCommitted(const GURL& url,
                              bool is_error_page,
                              int http_status_code) {}
void WebView::OnLoadStopped(const GURL& validated_url) {}
void WebView::OnLoadFailed(const GURL& validated_url,
                           int error_code,
                           const std::string& error_description,
                           int http_status_code) {}
void WebView::OnLoadSucceeded(const GURL& validated_url,
                              int http_status_code) {}

void WebView::OnNavigationEntryCommitted() {}
void WebView::OnNavigationListPruned(bool from_front, int count) {}
void WebView::OnNavigationEntryChanged(int index) {}

bool WebView::OnAddMessageToConsole(int32 level,
                                    const base::string16& message,
                                    int32 line_no,
                                    const base::string16& source_id) {
  return false;
}

void WebView::OnToggleFullscreenMode(bool enter) {}

void WebView::OnWebPreferencesDestroyed() {}

void WebView::OnRequestGeolocationPermission(
    scoped_ptr<SimplePermissionRequest> request) {}
void WebView::OnRequestMediaAccessPermission(
    scoped_ptr<MediaAccessPermissionRequest> request) {}

void WebView::OnUnhandledKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {}

void WebView::OnFrameMetadataUpdated(const cc::CompositorFrameMetadata& old) {}

void WebView::OnDownloadRequested(const GURL& url,
				  const std::string& mimeType,
				  const bool shouldPrompt,
				  const base::string16& suggestedFilename,
				  const std::string& cookies,
				  const std::string& referrer) {}

void WebView::OnBasicAuthenticationRequested(
    LoginPromptDelegate* login_delegate) {}

bool WebView::ShouldHandleNavigation(const GURL& url,
                                     WindowOpenDisposition disposition,
                                     bool user_gesture) {
  return true;
}

WebFrame* WebView::CreateWebFrame(
    content::RenderFrameHost* render_frame_host) {
  return new WebFrame(render_frame_host, this);
}

WebPopupMenu* WebView::CreatePopupMenu(content::RenderFrameHost* rfh) {
  return nullptr;
}

WebView* WebView::CreateNewWebView(const gfx::Rect& initial_pos,
                                   WindowOpenDisposition disposition) {
  NOTREACHED() <<
      "Your CanCreateWindows() implementation should be returning false!";
  return nullptr;
}

FilePicker* WebView::CreateFilePicker(content::RenderViewHost* rvh) {
  return nullptr;
}

void WebView::OnEvictCurrentFrame() {}

void WebView::OnTextInputStateChanged() {}
void WebView::OnFocusedNodeChanged() {}
void WebView::OnSelectionBoundsChanged() {}
void WebView::OnImeCancelComposition() {}
void WebView::OnSelectionChanged() {}

void WebView::OnUpdateCursor(const content::WebCursor& cursor) {}

void WebView::OnSecurityStatusChanged(const SecurityStatus& old) {}
void WebView::OnCertificateError(scoped_ptr<CertificateError> error) {}
void WebView::OnContentBlocked() {}

void WebView::OnPrepareToCloseResponse(bool proceed) {}
void WebView::OnCloseRequested() {}

WebView::WebView()
    : text_input_type_(ui::TEXT_INPUT_TYPE_NONE),
=======
WebView::WebView(WebViewClient* client)
    : client_(client),
      text_input_type_(ui::TEXT_INPUT_TYPE_NONE),
>>>>>>> MERGE-SOURCE
      show_ime_if_needed_(false),
      focused_node_is_editable_(false),
      selection_cursor_position_(0),
      selection_anchor_position_(0),
      web_contents_helper_(nullptr),
      compositor_(Compositor::Create(this)),
      restore_type_(content::NavigationController::RESTORE_LAST_SESSION_EXITED_CLEANLY),
      initial_index_(0),
      is_fullscreen_(false),
      blocked_content_(CONTENT_TYPE_NONE),
      location_bar_height_pix_(0),
      location_bar_constraints_(blink::WebTopControlsBoth),
      location_bar_animated_(true),
      weak_factory_(this) {
  DCHECK(client_);
}

WebView::~WebView() {
  g_all_web_views.Get().erase(
      std::remove(g_all_web_views.Get().begin(),
                  g_all_web_views.Get().end(),
                  this),
      g_all_web_views.Get().end());

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetDelegate(nullptr);
  }

  // Stop WebContents from calling back in to us
  content::WebContentsObserver::Observe(nullptr);

  web_contents_->RemoveUserData(kWebViewKey);
}

void WebView::Init(Params* params) {
  CHECK(!web_contents_) << "Cannot initialize webview more than once";

  CompositorLock lock(compositor_.get());

  if (params->contents) {
    CHECK(!params->context) <<
        "Shouldn't specify a BrowserContext and WebContents at initialization";
    CHECK(params->contents->GetBrowserContext()) <<
        "Specified WebContents doesn't have a BrowserContext";
    CHECK(WebViewContentsHelper::FromWebContents(params->contents.get())) <<
        "Specified WebContents should already have a WebViewContentsHelper";
    CHECK(!FromWebContents(params->contents.get())) <<
        "Specified WebContents already belongs to a WebView";

    web_contents_.reset(static_cast<content::WebContentsImpl *>(
        params->contents.release()));

    RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
    if (rwhv) {
      rwhv->SetDelegate(this);
    }

    content::RenderViewHost* rvh = GetRenderViewHost();
    if (rvh) {
      InitializeTopControlsForHost(rvh, true);
    }

    // Sync WebContents with the state of the WebView
    WasResized();
    ScreenUpdated();
    VisibilityChanged();
    FocusChanged();
    InputPanelVisibilityChanged();
    UpdateWebPreferences();

    // Update SSL Status
    content::NavigationEntry* entry =
        web_contents_->GetController().GetVisibleEntry();
    if (entry) {
      security_status_.Update(entry->GetSSL());
    }
  } else {
    CHECK(params->context) << "Didn't specify a BrowserContext or WebContents";

    scoped_refptr<BrowserContext> context = params->incognito ?
        params->context->GetOffTheRecordContext() :
        params->context->GetOriginalContext();

    content::WebContents::CreateParams content_params(context.get());
    content_params.initial_size = GetViewSizeDip();
    content_params.initially_hidden = !IsVisible();
    web_contents_.reset(static_cast<content::WebContentsImpl *>(
        content::WebContents::Create(content_params)));
    CHECK(web_contents_.get()) << "Failed to create WebContents";

    if (!restore_state_.empty()) {
      ScopedVector<content::NavigationEntry> entries =
          sessions::ContentSerializedNavigationBuilder::ToNavigationEntries(
              restore_state_, context.get());
      web_contents_->GetController().Restore(
          initial_index_,
          content::NavigationController::RESTORE_LAST_SESSION_EXITED_CLEANLY,
          &entries.get());
      restore_state_.clear();
    }

    CreateHelpers(web_contents_.get());

    compositor_->SetViewportSize(GetViewSizePix());
    compositor_->SetVisibility(IsVisible());
  }

  compositor_->SetDeviceScaleFactor(GetScreenInfo().deviceScaleFactor);

  // Attach ourself to the WebContents
  web_contents_->SetDelegate(this);
  web_contents_->SetUserData(kWebViewKey, new WebViewUserData(this));

  content::WebContentsObserver::Observe(web_contents_.get());

  // Set the initial WebPreferences. This has to happen after attaching
  // ourself to the WebContents, as the pref update needs to call back in
  // to us (via CanCreateWindows)
  web_contents_helper_ =
      WebViewContentsHelper::FromWebContents(web_contents_.get());
  if (web_preferences()) {
    web_contents_helper_->SetWebPreferences(web_preferences());
  }
  web_contents_helper_->WebContentsAdopted();

  registrar_.Add(this, content::NOTIFICATION_NAV_LIST_PRUNED,
                 content::NotificationService::AllBrowserContextsAndSources());
  registrar_.Add(this, content::NOTIFICATION_NAV_ENTRY_CHANGED,
                 content::NotificationService::AllBrowserContextsAndSources());

  root_frame_.reset(CreateWebFrame(web_contents_->GetMainFrame()));
  DCHECK(root_frame_.get());

  if (params->context) {
    if (!initial_url_.is_empty()) {
      SetURL(initial_url_);
      initial_url_ = GURL();
    } else if (initial_data_) {
      web_contents_->GetController().LoadURLWithParams(*initial_data_);
      initial_data_.reset();
    }
  }

  web_contents_->GetController().LoadIfNecessary();

  SetIsFullscreen(is_fullscreen_);

  DCHECK(std::find(g_all_web_views.Get().begin(),
                   g_all_web_views.Get().end(),
                   this) ==
         g_all_web_views.Get().end());
  g_all_web_views.Get().push_back(this);

  client_->Initialized();
}

// static
WebView* WebView::FromWebContents(const content::WebContents* web_contents) {
  WebViewUserData* data = static_cast<WebViewUserData *>(
      web_contents->GetUserData(kWebViewKey));
  if (!data) {
    return nullptr;
  }

  return data->get();
}

// static
WebView* WebView::FromRenderViewHost(content::RenderViewHost* rvh) {
  return FromWebContents(content::WebContents::FromRenderViewHost(rvh));
}

// static
WebView* WebView::FromRenderFrameHost(content::RenderFrameHost* rfh) {
  return FromWebContents(content::WebContents::FromRenderFrameHost(rfh));
}

const GURL& WebView::GetURL() const {
  if (!web_contents_) {
    return initial_url_;
  }
  return web_contents_->GetVisibleURL();
}

void WebView::SetURL(const GURL& url) {
  if (!url.is_valid()) {
    return;
  }

  if (!web_contents_) {
    initial_url_ = url;
    initial_data_.reset();
    return;
  }

  content::NavigationController::LoadURLParams params(url);
  params.transition_type = ui::PAGE_TRANSITION_TYPED;
  web_contents_->GetController().LoadURLWithParams(params);
}

std::vector<sessions::SerializedNavigationEntry> WebView::GetState() const {
  std::vector<sessions::SerializedNavigationEntry> entries;
  if (!web_contents_) {
    return entries;
  }
  const content::NavigationController& controller = web_contents_->GetController();
  const int pending_index = controller.GetPendingEntryIndex();
  int entry_count = controller.GetEntryCount();
  if (entry_count == 0 && pending_index == 0) {
    entry_count++;
  }
  entries.resize(entry_count);
  for (int i = 0; i < entry_count; ++i) {
    content::NavigationEntry* entry = (i == pending_index) ?
        controller.GetPendingEntry() : controller.GetEntryAtIndex(i);
    entries[i] =
        sessions::ContentSerializedNavigationBuilder::FromNavigationEntry(i, *entry);
  }
  return entries;
}

void WebView::SetState(content::NavigationController::RestoreType type,
                       std::vector<sessions::SerializedNavigationEntry> state,
                       int index) {
  DCHECK(!web_contents_);
  restore_type_ = type;
  restore_state_ = state;
  initial_index_ = index;
}

void WebView::LoadData(const std::string& encodedData,
                       const std::string& mimeType,
                       const GURL& baseUrl) {
  std::string url("data:");
  url.append(mimeType);
  url.append(",");
  url.append(encodedData);

  content::NavigationController::LoadURLParams params((GURL(url)));
  params.load_type = content::NavigationController::LOAD_TYPE_DATA;
  params.base_url_for_data_url = baseUrl;
  params.virtual_url_for_data_url = baseUrl.is_empty() ? GURL(url::kAboutBlankURL) : baseUrl;
  params.can_load_local_resources = true;

  if (web_contents_) {
    web_contents_->GetController().LoadURLWithParams(params);
  } else {
    initial_data_.reset(new content::NavigationController::LoadURLParams(params));
    initial_url_ = GURL();
  }
}

std::string WebView::GetTitle() const {
  if (!web_contents_) {
    return std::string();
  }
  return base::UTF16ToUTF8(web_contents_->GetTitle());
}

bool WebView::CanGoBack() const {
  if (!web_contents_) {
    return false;
  }
  return web_contents_->GetController().CanGoBack();
}

bool WebView::CanGoForward() const {
  if (!web_contents_) {
    return false;
  }
  return web_contents_->GetController().CanGoForward();
}

void WebView::GoBack() {
  if (!CanGoBack()) {
    return;
  }

  web_contents_->GetController().GoBack();
}

void WebView::GoForward() {
  if (!CanGoForward()) {
    return;
  }

  web_contents_->GetController().GoForward();
}

void WebView::Stop() {
  web_contents_->Stop();
}

void WebView::Reload() {
  web_contents_->GetController().Reload(true);
}

bool WebView::IsIncognito() const {
  if (!GetBrowserContext()) {
    return false;
  }

  return GetBrowserContext()->IsOffTheRecord();
}

bool WebView::IsLoading() const {
  if (!web_contents_) {
    return false;
  }
  return web_contents_->IsLoading();
}

bool WebView::IsFullscreen() const {
  return is_fullscreen_;
}

void WebView::SetIsFullscreen(bool fullscreen) {
  if (fullscreen != is_fullscreen_) {
    is_fullscreen_ = fullscreen;
    if (web_contents_) {
      web_contents_->GetRenderViewHost()->WasResized();
    }
  }
}

void WebView::WasResized() {
  {
    CompositorLock lock(compositor_.get());
    compositor_->SetDeviceScaleFactor(GetScreenInfo().deviceScaleFactor);
    compositor_->SetViewportSize(GetViewSizePix());
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (rwhv) {
    rwhv->SetSize(GetViewSizeDip());
    GetRenderWidgetHostImpl()->SendScreenRects();
    rwhv->GetRenderWidgetHost()->WasResized();
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebView::ScreenUpdated() {
  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host) {
    return;
  }

  host->NotifyScreenInfoChanged();
}

void WebView::VisibilityChanged() {
  bool visible = IsVisible();

  compositor_->SetVisibility(visible);

  if (visible) {
    web_contents_->WasShown();
  } else {
    web_contents_->WasHidden();
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebView::FocusChanged() {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  if (HasFocus()) {
    rwhv->Focus();
  } else {
    rwhv->Blur();
  }

  MaybeScrollFocusedEditableNodeIntoView();
}

void WebView::InputPanelVisibilityChanged() {
  MaybeScrollFocusedEditableNodeIntoView();
}

void WebView::UpdateWebPreferences() {
  if (!web_contents_) {
    return;
  }

  content::RenderViewHost* rvh = web_contents_->GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->OnWebkitPreferencesChanged();
}

int WebView::GetFindInPageCount() const {
  return find_in_page_.count;
}

int WebView::GetFindInPageCurrent() const {
  return find_in_page_.current;
}

std::string WebView::GetFindInPageText() const {
  return find_in_page_.text;
}

bool WebView::GetFindInPageCaseSensitive() const {
  return find_in_page_.case_sensitive;
}

void WebView::SetFindInPageText(const std::string& text) {
  find_in_page_.text = text;
  RestartFindInPage();
}

void WebView::SetFindInPageCaseSensitive(bool case_sensitive) {
  find_in_page_.case_sensitive = case_sensitive;
  RestartFindInPage();
}

void WebView::RestartFindInPage() {
  if (!web_contents_) {
      return;
  }

  web_contents_->StopFinding(content::STOP_FIND_ACTION_CLEAR_SELECTION);

  find_in_page_.current = 0;
  client_->FindInPageCurrentChanged();
  find_in_page_.count = 0;
  client_->FindInPageCountChanged();

  if (!find_in_page_.text.empty()) {
    find_in_page_.request_id++;

    blink::WebFindOptions options;
    options.forward = true;
    options.findNext = false;
    options.matchCase = find_in_page_.case_sensitive;

    web_contents_->Find(find_in_page_.request_id,
                        base::UTF8ToUTF16(find_in_page_.text),
                        options);
  }
}

void WebView::FindInPageNext() {
  if (!web_contents_) {
    return;
  }

  blink::WebFindOptions options;
  options.forward = true;
  options.findNext = true;

  web_contents_->Find(find_in_page_.request_id,
                      base::UTF8ToUTF16(find_in_page_.text), options);
}

void WebView::FindInPagePrevious() {
  if (!web_contents_) {
    return;
  }

  blink::WebFindOptions options;
  options.forward = false;
  options.findNext = true;

  web_contents_->Find(find_in_page_.request_id,
                      base::UTF8ToUTF16(find_in_page_.text), options);
}

BrowserContext* WebView::GetBrowserContext() const {
  return BrowserContext::FromContent(web_contents_->GetBrowserContext());
}

content::WebContents* WebView::GetWebContents() const {
  return web_contents_.get();
}

int WebView::GetNavigationEntryCount() const {
  if (!web_contents_) {
    return 0;
  }
  return web_contents_->GetController().GetEntryCount();
}

int WebView::GetNavigationCurrentEntryIndex() const {
  if (!web_contents_) {
    return -1;
  }
  return web_contents_->GetController().GetCurrentEntryIndex();
}

void WebView::SetNavigationCurrentEntryIndex(int index) {
  if (web_contents_) {
    web_contents_->GetController().GoToIndex(index);
  }
}

int WebView::GetNavigationEntryUniqueID(int index) const {
  if (!web_contents_) {
    return 0;
  }
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return entry->GetUniqueID();
}

const GURL& WebView::GetNavigationEntryUrl(int index) const {
  if (!web_contents_) {
    return GURL::EmptyGURL();
  }
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return entry->GetURL();
}

std::string WebView::GetNavigationEntryTitle(int index) const {
  if (!web_contents_) {
    return std::string();
  }
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return base::UTF16ToUTF8(entry->GetTitle());
}

base::Time WebView::GetNavigationEntryTimestamp(int index) const {
  if (!web_contents_) {
    return base::Time();
  }
  const content::NavigationController& controller = web_contents_->GetController();
  content::NavigationEntry* entry = controller.GetEntryAtIndex(index);
  return entry->GetTimestamp();
}

WebFrame* WebView::GetRootFrame() const {
  return root_frame_.get();
}

WebPreferences* WebView::GetWebPreferences() {
  if (!web_contents_helper_) {
    return web_preferences();
  }

  return web_contents_helper_->GetWebPreferences();
}

void WebView::SetWebPreferences(WebPreferences* prefs) {
  WebPreferencesObserver::Observe(prefs);
  if (!web_contents_helper_) {
    return;
  }

  web_contents_helper_->SetWebPreferences(prefs);
}

gfx::Size WebView::GetViewSizePix() const {
  return GetViewBoundsPix().size();
}

gfx::Rect WebView::GetViewBoundsDip() const {
  float scale = 1.0f / GetScreenInfo().deviceScaleFactor;
  gfx::Rect bounds(GetViewBoundsPix());

  int x = std::lround(bounds.x() * scale);
  int y = std::lround(bounds.y() * scale);
  int width = std::lround(bounds.width() * scale);
  int height = std::lround(bounds.height() * scale);

  return gfx::Rect(x, y, width, height);
}

gfx::Size WebView::GetViewSizeDip() const {
  float scale = 1.0f / GetScreenInfo().deviceScaleFactor;
  gfx::Size size(GetViewSizePix());

  int width = std::lround(size.width() * scale);
  int height = std::lround(size.height() * scale);

  return gfx::Size(width, height);
}

gfx::Point WebView::GetCompositorFrameScrollOffsetPix() {
  // See https://launchpad.net/bugs/1336730
  const gfx::SizeF& viewport_size =
      compositor_frame_metadata().scrollable_viewport_size;
  float x_scale = GetFrameMetadataScaleToPix() *
                  viewport_size.width() / std::round(viewport_size.width());
  float y_scale = GetFrameMetadataScaleToPix() *
                  viewport_size.height() / std::round(viewport_size.height());

  gfx::Vector2dF offset =
      gfx::ScaleVector2d(compositor_frame_metadata().root_scroll_offset,
                         x_scale, y_scale);

  return gfx::Point(std::round(offset.x()), std::round(offset.y()));
}

gfx::Size WebView::GetCompositorFrameContentSizePix() {
  // See https://launchpad.net/bugs/1336730
  const gfx::SizeF& viewport_size =
      compositor_frame_metadata().scrollable_viewport_size;
  float x_scale = GetFrameMetadataScaleToPix() *
                  viewport_size.width() / std::round(viewport_size.width());
  float y_scale = GetFrameMetadataScaleToPix() *
                  viewport_size.height() / std::round(viewport_size.height());

  gfx::SizeF size =
      gfx::ScaleSize(compositor_frame_metadata().root_layer_size,
                     x_scale, y_scale);

  return gfx::Size(std::round(size.width()), std::round(size.height()));
}

gfx::Size WebView::GetCompositorFrameViewportSizePix() {
  gfx::SizeF size =
      gfx::ScaleSize(compositor_frame_metadata().scrollable_viewport_size,
                     GetFrameMetadataScaleToPix());

  return gfx::Size(std::round(size.width()), std::round(size.height()));
}

int WebView::GetLocationBarOffsetPix() {
  return compositor_frame_metadata().location_bar_offset.y() *
         compositor_frame_metadata().device_scale_factor;
}

int WebView::GetLocationBarContentOffsetPix() {
  return compositor_frame_metadata().location_bar_content_translation.y() *
         compositor_frame_metadata().device_scale_factor;
}

float WebView::GetLocationBarContentOffsetDip() {
  return compositor_frame_metadata().location_bar_content_translation.y();
}

float WebView::GetLocationBarHeightDip() const {
  return GetLocationBarHeightPix() / GetScreenInfo().deviceScaleFactor;
}

int WebView::GetLocationBarHeightPix() const {
  return location_bar_height_pix_;
}

void WebView::SetLocationBarHeightPix(int height) {
  DCHECK_GE(height, 0);

  if (height == location_bar_height_pix_) {
    return;
  }

  location_bar_height_pix_ = height;

  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host) {
    return;
  }

  host->WasResized();
}

void WebView::SetLocationBarConstraints(blink::WebTopControlsState constraints) {
  if (constraints == location_bar_constraints_) {
    return;
  }

  location_bar_constraints_ = constraints;

  if (!web_contents_) {
    return;
  }

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->Send(new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                                location_bar_constraints_,
                                                blink::WebTopControlsBoth,
                                                location_bar_animated_));
}

void WebView::ShowLocationBar(bool animate) {
  DCHECK_EQ(location_bar_constraints_, blink::WebTopControlsBoth);

  if (!web_contents_) {
    return;
  }

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->Send(new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                                location_bar_constraints_,
                                                blink::WebTopControlsShown,
                                                animate));
}

void WebView::HideLocationBar(bool animate) {
  DCHECK_EQ(location_bar_constraints_, blink::WebTopControlsBoth);

  if (!web_contents_) {
    return;
  }

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->Send(new OxideMsg_UpdateTopControlsState(rvh->GetRoutingID(),
                                                location_bar_constraints_,
                                                blink::WebTopControlsHidden,
                                                animate));
}

void WebView::SetCanTemporarilyDisplayInsecureContent(bool allow) {
  if (!web_contents_) {
    return;
  }

  if (!(blocked_content_ & CONTENT_TYPE_MIXED_DISPLAY) && allow) {
    return;
  }

  if (allow) {
    blocked_content_ &= ~CONTENT_TYPE_MIXED_DISPLAY;
    client_->ContentBlocked();
  }

  web_contents_->SendToAllFrames(
      new OxideMsg_SetAllowDisplayingInsecureContent(MSG_ROUTING_NONE, allow));
  web_contents_->GetMainFrame()->Send(
      new OxideMsg_ReloadFrame(web_contents_->GetMainFrame()->GetRoutingID()));
}

void WebView::SetCanTemporarilyRunInsecureContent(bool allow) {
  if (!web_contents_) {
    return;
  }

  if (!(blocked_content_ & CONTENT_TYPE_MIXED_SCRIPT) && allow) {
    return;
  }

  if (allow) {
    blocked_content_ &= ~CONTENT_TYPE_MIXED_DISPLAY;
    blocked_content_ &= ~CONTENT_TYPE_MIXED_SCRIPT;
    client_->ContentBlocked();
  }

  web_contents_->SendToAllFrames(
      new OxideMsg_SetAllowRunningInsecureContent(MSG_ROUTING_NONE, allow));
  web_contents_->GetMainFrame()->Send(
      new OxideMsg_ReloadFrame(web_contents_->GetMainFrame()->GetRoutingID()));
}

void WebView::PrepareToClose() {
  base::Closure no_before_unload_handler_response_task =
      base::Bind(&WebView::DispatchPrepareToCloseResponse,
                 AsWeakPtr(), true);

  if (!web_contents_) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE, no_before_unload_handler_response_task);
    return;
  }

  if (!web_contents_->NeedToFireBeforeUnload()) {
    base::MessageLoop::current()->PostTask(
        FROM_HERE, no_before_unload_handler_response_task);
    return;
  }

  // This is ok to call multiple times - RFHI tracks whether a response
  // is pending and won't dispatch another event if it is
  web_contents_->DispatchBeforeUnload(false);
}

void WebView::ShowContextMenu(content::RenderFrameHost* render_frame_host,
                              const content::ContextMenuParams& params) {
  WebContextMenu* menu = client_->CreateContextMenu(render_frame_host, params);
  if (!menu) {
    return;
  }

  menu->Show();
}

void WebView::ShowPopupMenu(content::RenderFrameHost* render_frame_host,
                            const gfx::Rect& bounds,
                            int selected_item,
                            const std::vector<content::MenuItem>& items,
                            bool allow_multiple_selection) {
  DCHECK(!active_popup_menu_);

  WebPopupMenu* menu = client_->CreatePopupMenu(render_frame_host);
  if (!menu) {
    static_cast<content::RenderFrameHostImpl *>(
        render_frame_host)->DidCancelPopupMenu();
    return;
  }

  active_popup_menu_ = menu->GetWeakPtr();

  menu->Show(bounds, items, selected_item, allow_multiple_selection);
}

void WebView::HidePopupMenu() {
  if (!active_popup_menu_) {
    return;
  }

  active_popup_menu_->Close();
}

void WebView::RequestGeolocationPermission(
    const GURL& requesting_frame,
    int bridge_id,
    const base::Callback<void(content::PermissionStatus)>& callback) {
  PermissionRequestID request_id(
      web_contents_->GetRenderProcessHost()->GetID(),
      web_contents_->GetRenderViewHost()->GetRoutingID(),
      bridge_id,
      requesting_frame);

  scoped_ptr<SimplePermissionRequest> request(
      new SimplePermissionRequest(
        &permission_request_manager_,
        request_id,
        requesting_frame,
        web_contents_->GetLastCommittedURL().GetOrigin(),
        callback));

  client_->RequestGeolocationPermission(request.Pass());
}

void WebView::CancelGeolocationPermissionRequest(
    const GURL& requesting_frame,
    int bridge_id) {
  PermissionRequestID request_id(
      web_contents_->GetRenderProcessHost()->GetID(),
      web_contents_->GetRenderViewHost()->GetRoutingID(),
      bridge_id,
      requesting_frame);

  permission_request_manager_.CancelPendingRequestForID(request_id);
}

void WebView::AllowCertificateError(
    content::RenderFrameHost* rfh,
    int cert_error,
    const net::SSLInfo& ssl_info,
    const GURL& request_url,
    content::ResourceType resource_type,
    bool overridable,
    bool strict_enforcement,
    const base::Callback<void(bool)>& callback,
    content::CertificateRequestResultType* result) {
  WebFrame* frame = WebFrame::FromRenderFrameHost(rfh);
  if (!frame) {
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_CANCEL;
    return;
  }

  DCHECK_EQ(frame->view(), this);

  // We can't safely allow the embedder to override errors for subresources or
  // subframes because they don't always result in the API indicating a
  // degraded security level. Mark these non-overridable for now and just
  // deny them outright
  // See https://launchpad.net/bugs/1368385
  if (!overridable || resource_type != content::RESOURCE_TYPE_MAIN_FRAME) {
    overridable = false;
    *result = content::CERTIFICATE_REQUEST_RESULT_TYPE_DENY;
  }

  scoped_ptr<CertificateError> error(new CertificateError(
      &certificate_error_manager_,
      frame,
      cert_error,
      ssl_info,
      request_url,
      resource_type,
      strict_enforcement,
      overridable ? callback : base::Callback<void(bool)>()));
  client_->OnCertificateError(error.Pass());
}

void WebView::HandleKeyEvent(const content::NativeWebKeyboardEvent& event) {
  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->ForwardKeyboardEvent(event);
}

void WebView::HandleMouseEvent(const blink::WebMouseEvent& event) {
  blink::WebMouseEvent e(event);

  if (e.type == blink::WebInputEvent::MouseEnter ||
      e.type == blink::WebInputEvent::MouseLeave) {
    global_mouse_position_.SetPoint(event.globalX, event.globalY);
    e.type = blink::WebInputEvent::MouseMove;
  }

  e.movementX = e.globalX - global_mouse_position_.x();
  e.movementY = e.globalY - global_mouse_position_.y();

  global_mouse_position_.SetPoint(e.globalX, e.globalY);

  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->ForwardMouseEvent(e);
}

void WebView::HandleTouchEvent(const ui::TouchEvent& event) {
  if (!touch_state_.Update(event)) {
    return;
  }

  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return;
  }

  rwhv->HandleTouchEvent(touch_state_);
}

void WebView::HandleWheelEvent(const blink::WebMouseWheelEvent& event) {
  content::RenderViewHost* rvh = GetRenderViewHost();
  if (!rvh) {
    return;
  }

  rvh->ForwardWheelEvent(event);
}

void WebView::ImeCommitText(const base::string16& text,
                            const gfx::Range& replacement_range) {
  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host) {
    return;
  }

  SendFakeCompositionKeyEvent(host, blink::WebInputEvent::RawKeyDown);
  host->ImeConfirmComposition(text, replacement_range, false);
  SendFakeCompositionKeyEvent(host, blink::WebInputEvent::KeyUp);
}

void WebView::ImeSetComposingText(
    const base::string16& text,
    const std::vector<blink::WebCompositionUnderline>& underlines,
    const gfx::Range& selection_range) {
  content::RenderWidgetHostImpl* host = GetRenderWidgetHostImpl();
  if (!host) {
    return;
  }

  SendFakeCompositionKeyEvent(host, blink::WebInputEvent::RawKeyDown);
  host->ImeSetComposition(text, underlines,
                          selection_range.start(),
                          selection_range.end());
  SendFakeCompositionKeyEvent(host, blink::WebInputEvent::KeyUp);
}

void WebView::DownloadRequested(
    const GURL& url,
    const std::string& mime_type,
    const bool should_prompt,
    const base::string16& suggested_filename,
    const std::string& cookies,
    const std::string& referrer,
    const std::string& user_agent) {
  client_->DownloadRequested(url,
                             mime_type,
                             should_prompt,
                             suggested_filename,
                             cookies,
                             referrer,
                             user_agent);
}

void WebView::BasicAuthenticationRequested(LoginPromptDelegate* login_delegate) {
  OnBasicAuthenticationRequested(login_delegate);
}

CompositorFrameHandle* WebView::GetCompositorFrameHandle() const {
  return current_compositor_frame_.get();
}

void WebView::DidCommitCompositorFrame() {
  DCHECK(!received_surface_ids_.empty());

  while (!received_surface_ids_.empty()) {
    uint32 surface_id = received_surface_ids_.front();
    received_surface_ids_.pop();

    compositor_->DidSwapCompositorFrame(
        surface_id,
        std::move(previous_compositor_frames_));
  }
}

blink::WebScreenInfo WebView::GetScreenInfo() const {
  return client_->GetScreenInfo();
}

gfx::Rect WebView::GetViewBoundsPix() const {
  return client_->GetViewBoundsPix();
}

bool WebView::IsVisible() const {
  return client_->IsVisible();
}

bool WebView::HasFocus() const {
  return client_->HasFocus();
}

bool WebView::IsInputPanelVisible() const {
  return client_->IsInputPanelVisible();
}

JavaScriptDialog* WebView::CreateJavaScriptDialog(
    content::JavaScriptMessageType javascript_message_type) {
  return client_->CreateJavaScriptDialog(javascript_message_type);
}

JavaScriptDialog* WebView::CreateBeforeUnloadDialog() {
  return client_->CreateBeforeUnloadDialog();
}

bool WebView::CanCreateWindows() const {
  return client_->CanCreateWindows();
}

base::string16 WebView::GetSelectedText() const {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return base::string16();
  }

  return rwhv->GetSelectedText();
}

const base::string16& WebView::GetSelectionText() const {
  RenderWidgetHostView* rwhv = GetRenderWidgetHostView();
  if (!rwhv) {
    return base::EmptyString16();
  }

  return rwhv->selection_text();
}

} // namespace oxide
