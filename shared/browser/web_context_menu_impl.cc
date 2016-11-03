// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2016 Canonical Ltd.

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

#include "web_context_menu_impl.h"

#include "base/threading/thread_task_runner_handle.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_url_parameters.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/referrer.h"
#include "net/http/http_request_headers.h"

#include "shared/common/oxide_constants.h"

#include "web_contents_client.h"
#include "web_contents_helper.h"

namespace oxide {

namespace {

const GURL& GetDocumentURL(const content::ContextMenuParams& params) {
  return params.frame_url.is_empty() ? params.page_url : params.frame_url;
}

content::Referrer CreateSaveAsReferrer(
    const GURL& url,
    const content::ContextMenuParams& params) {
  const GURL& referring_url = GetDocumentURL(params);
  return content::Referrer::SanitizeForRequest(
      url,
      content::Referrer(referring_url.GetAsReferrer(), params.referrer_policy));
}

}

content::WebContents* WebContextMenuImpl::GetWebContents() const {
  return web_contents();
}

void WebContextMenuImpl::Close() {
  if (menu_) {
    menu_->Hide();
  }

  render_frame_host_ = nullptr;
  base::ThreadTaskRunnerHandle::Get()->DeleteSoon(FROM_HERE, this);
}

void WebContextMenuImpl::CopyImage() const {
  if (params_.media_type != blink::WebContextMenuData::MediaTypeCanvas &&
      params_.media_type != blink::WebContextMenuData::MediaTypeImage) {
    return;
  }

  render_frame_host_->CopyImageAt(params_.x, params_.y);
}

void WebContextMenuImpl::SaveLink() const {
  content::BrowserContext* context = web_contents()->GetBrowserContext();
  content::DownloadManager* dlm =
      content::BrowserContext::GetDownloadManager(context);
  const GURL& url = params_.link_url;
  std::unique_ptr<content::DownloadUrlParameters> dl_params(
      content::DownloadUrlParameters::CreateForWebContentsMainFrame(
          web_contents(), url));
  dl_params->set_referrer(CreateSaveAsReferrer(url, params_));
  dl_params->set_referrer_encoding(params_.frame_charset);
  dl_params->set_suggested_name(params_.suggested_filename);
  dl_params->set_prompt(true);

  dlm->DownloadUrl(std::move(dl_params));
}

void WebContextMenuImpl::SaveMedia() const {
  bool is_large_data_url =
      params_.has_image_contents && params_.src_url.is_empty();
  if ((params_.media_type == blink::WebContextMenuData::MediaTypeCanvas) ||
      ((params_.media_type == blink::WebContextMenuData::MediaTypeImage) &&
      is_large_data_url)) {
    render_frame_host_->SaveImageAt(params_.x, params_.y);
  } else {
    const GURL& url = params_.src_url;
    content::Referrer referrer = CreateSaveAsReferrer(url, params_);
    if (params_.media_type == blink::WebContextMenuData::MediaTypeImage) {
      // XXX(oSoMoN): see comment in
      // oxide::ResourceDispatcherHostDelegate::DispatchDownloadRequest(…).
      std::map<std::string, std::string>::const_iterator it;
      it = params_.properties.find(oxide::kImageContextMenuPropertiesMimeType);
      std::string headers;
      if (it != params_.properties.cend()) {
        headers.append(net::HttpRequestHeaders::kContentType);
        headers.append(": ");
        headers.append(it->second);
      }
      web_contents()->SaveFrameWithHeaders(url, referrer, headers);
    } else {
      web_contents()->SaveFrame(url, referrer);
    }
  }
}

void WebContextMenuImpl::RenderFrameDeleted(content::RenderFrameHost* rfh) {
  if (rfh != render_frame_host_) {
    return;
  }

  Close();
}

WebContextMenuImpl::WebContextMenuImpl(
    content::RenderFrameHost* render_frame_host,
    const content::ContextMenuParams& params)
    : content::WebContentsObserver(
          content::WebContents::FromRenderFrameHost(render_frame_host)),
      render_frame_host_(render_frame_host),
      params_(params) {}

WebContextMenuImpl::~WebContextMenuImpl() = default;

void WebContextMenuImpl::Show() {
  WebContentsHelper* helper =
      WebContentsHelper::FromWebContents(web_contents());
  if (!helper->client()) {
    Close();
    return;
  }

  menu_ = helper->client()->CreateContextMenu(params_, this);

  if (!menu_) {
    Close();
    return;
  }

  menu_->Show();
}

void WebContextMenuImpl::Hide() {
  Close();
}

} // namespace oxide
