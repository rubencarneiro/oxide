// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#include "oxide_web_view_client.h"

#include "base/logging.h"

namespace oxide {

WebViewClient::~WebViewClient() {}

void WebViewClient::Initialized() {}

bool WebViewClient::IsInputPanelVisible() const {
  return false;
}

JavaScriptDialog* WebViewClient::CreateJavaScriptDialog(
    content::JavaScriptMessageType javascript_message_type) {
  return nullptr;
}

JavaScriptDialog* WebViewClient::CreateBeforeUnloadDialog() {
  return nullptr;
}

bool WebViewClient::CanCreateWindows() const {
  return false;
}

void WebViewClient::CrashedStatusChanged() {}

void WebViewClient::URLChanged() {}

void WebViewClient::TitleChanged() {}

void WebViewClient::IconChanged(const GURL& icon) {}

void WebViewClient::CommandsUpdated() {}

void WebViewClient::LoadingChanged() {}

void WebViewClient::LoadProgressChanged(double progress) {}

void WebViewClient::LoadStarted(const GURL& validated_url) {}

void WebViewClient::LoadRedirected(const GURL& url,
                                   const GURL& original_url,
                                   int http_status_code) {}

void WebViewClient::LoadCommitted(const GURL& url,
                                    bool is_error_page,
                                    int http_status_code) {}

void WebViewClient::LoadStopped(const GURL& validated_url) {}

void WebViewClient::LoadFailed(const GURL& validated_url,
                               int error_code,
                               const std::string& error_description,
                               int http_status_code) {}

void WebViewClient::LoadSucceeded(const GURL& validated_url,
                                    int http_status_code) {}

void WebViewClient::NavigationEntryCommitted() {}

void WebViewClient::NavigationListPruned(bool from_front, int count) {}

void WebViewClient::NavigationEntryChanged(int index) {}

bool WebViewClient::AddMessageToConsole(int32_t level,
                                        const base::string16& message,
                                        int32_t line_no,
                                        const base::string16& source_id) {
  return false;
}

void WebViewClient::ToggleFullscreenMode(bool enter) {}

void WebViewClient::WebPreferencesDestroyed() {}

void WebViewClient::UnhandledKeyboardEvent(
    const content::NativeWebKeyboardEvent& event) {}

void WebViewClient::FrameMetadataUpdated(
    const cc::CompositorFrameMetadata& old) {}

void WebViewClient::DownloadRequested(const GURL& url,
                                      const std::string& mime_type,
                                      const bool should_prompt,
                                      const base::string16& suggested_filename,
                                      const std::string& cookies,
                                      const std::string& referrer,
                                      const std::string& user_agent) {}

bool WebViewClient::ShouldHandleNavigation(const GURL& url,
                                           WindowOpenDisposition disposition,
                                           bool user_gesture) {
  return true;
}

WebFrame* WebViewClient::CreateWebFrame(
    content::RenderFrameHost* render_frame_host) {
  return nullptr;
}

WebContextMenu* WebViewClient::CreateContextMenu(
    content::RenderFrameHost* rfh,
    const content::ContextMenuParams& params) {
  return nullptr;
}

WebPopupMenu* WebViewClient::CreatePopupMenu(
    content::RenderFrameHost* rfh) {
  return nullptr;
}

WebView* WebViewClient::CreateNewWebView(const gfx::Rect& initial_pos,
                                         WindowOpenDisposition disposition) {
  NOTREACHED() <<
      "Your CanCreateWindows() implementation should be returning false!";
  return nullptr;
}

FilePicker* WebViewClient::CreateFilePicker(content::RenderViewHost* rvh) {
  return nullptr;
}

void WebViewClient::EvictCurrentFrame() {}

void WebViewClient::TextInputStateChanged() {}

void WebViewClient::FocusedNodeChanged() {}

void WebViewClient::SelectionBoundsChanged() {}

void WebViewClient::ImeCancelComposition() {}

void WebViewClient::SelectionChanged() {}

void WebViewClient::UpdateCursor(const content::WebCursor& cursor) {}

void WebViewClient::SecurityStatusChanged(const SecurityStatus& old) {}

void WebViewClient::OnCertificateError(scoped_ptr<CertificateError> error) {}

void WebViewClient::ContentBlocked() {}

void WebViewClient::PrepareToCloseResponseReceived(bool proceed) {}

void WebViewClient::CloseRequested() {}

void WebViewClient::FindInPageCurrentChanged() {}

void WebViewClient::FindInPageCountChanged() {}

void WebViewClient::HttpAuthenticationRequested(
    ResourceDispatcherHostLoginDelegate* login_delegate) {}

} // namespace oxide
