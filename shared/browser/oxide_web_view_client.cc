// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015-2016 Canonical Ltd.

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

namespace oxide {

WebViewClient::~WebViewClient() {}

void WebViewClient::URLChanged() {}

void WebViewClient::TitleChanged() {}

void WebViewClient::FaviconChanged() {}

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

bool WebViewClient::AddMessageToConsole(int32_t level,
                                        const base::string16& message,
                                        int32_t line_no,
                                        const base::string16& source_id) {
  return false;
}

void WebViewClient::FrameMetadataUpdated(
    const cc::CompositorFrameMetadata& old) {}

FilePicker* WebViewClient::CreateFilePicker(content::RenderFrameHost* rfh) {
  return nullptr;
}

void WebViewClient::ContentBlocked() {}

void WebViewClient::PrepareToCloseResponseReceived(bool proceed) {}

void WebViewClient::CloseRequested() {}

void WebViewClient::TargetURLChanged() {}

void WebViewClient::OnEditingCapabilitiesChanged() {}

} // namespace oxide
