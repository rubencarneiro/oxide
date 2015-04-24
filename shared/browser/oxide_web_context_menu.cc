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

#include "oxide_web_context_menu.h"

#include "base/logging.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

#include "oxide_web_contents_view.h"

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

} // namespace

namespace oxide {

void WebContextMenu::RenderFrameDeleted(content::RenderFrameHost* rfh) {
  if (rfh != render_frame_host_) {
    return;
  }

  Close();
}

void WebContextMenu::Hide() {}

WebContextMenu::WebContextMenu(content::RenderFrameHost* rfh,
                               const content::ContextMenuParams& params)
    : content::WebContentsObserver(content::WebContents::FromRenderFrameHost(rfh)),
      params_(params),
      render_frame_host_(static_cast<content::RenderFrameHostImpl *>(rfh)),
      weak_ptr_factory_(this) {}

WebContextMenu::~WebContextMenu() {
  DCHECK(!render_frame_host_);
}

void WebContextMenu::Close() {
  weak_ptr_factory_.InvalidateWeakPtrs();
  Hide();
  render_frame_host_ = nullptr;
  content::BrowserThread::DeleteSoon(
      content::BrowserThread::UI, FROM_HERE, this);
}

void WebContextMenu::Undo() const {
  if (params_.edit_flags & blink::WebContextMenuData::CanUndo) {
    web_contents()->Undo();
  }
}

void WebContextMenu::Redo() const {
  if (params_.edit_flags & blink::WebContextMenuData::CanRedo) {
    web_contents()->Redo();
  }
}

void WebContextMenu::Cut() const {
  if (params_.edit_flags & blink::WebContextMenuData::CanCut) {
    web_contents()->Cut();
  }
}

void WebContextMenu::Copy() const {
  if (params_.edit_flags & blink::WebContextMenuData::CanCopy) {
    web_contents()->Copy();
  }
}

void WebContextMenu::Paste() const {
  if (params_.edit_flags & blink::WebContextMenuData::CanPaste) {
    web_contents()->Paste();
  }
}

void WebContextMenu::Erase() const {
  if (params_.edit_flags & blink::WebContextMenuData::CanDelete) {
    web_contents()->Delete();
  }
}

void WebContextMenu::SelectAll() const {
  if (params_.edit_flags & blink::WebContextMenuData::CanSelectAll) {
    web_contents()->SelectAll();
  }
}

void WebContextMenu::SaveLink() const {
  content::BrowserContext* context = web_contents()->GetBrowserContext();
  content::DownloadManager* dlm =
      content::BrowserContext::GetDownloadManager(context);
  const GURL& url = params_.link_url;
  scoped_ptr<content::DownloadUrlParameters> dl_params(
      content::DownloadUrlParameters::FromWebContents(web_contents(), url));
  dl_params->set_referrer(CreateSaveAsReferrer(url, params_));
  dl_params->set_referrer_encoding(params_.frame_charset);
  dl_params->set_suggested_name(params_.suggested_filename);
  dl_params->set_prompt(true);
  dlm->DownloadUrl(dl_params.Pass());
}

void WebContextMenu::SaveImage() const {
  bool is_large_data_url =
      params_.has_image_contents && params_.src_url.is_empty();
  if ((params_.media_type == blink::WebContextMenuData::MediaTypeCanvas) ||
      ((params_.media_type == blink::WebContextMenuData::MediaTypeImage) &&
          is_large_data_url)) {
    render_frame_host_->GetRenderViewHost()->SaveImageAt(params_.x, params_.y);
  } else {
    const GURL& url = params_.src_url;
    content::Referrer referrer = CreateSaveAsReferrer(url, params_);
    web_contents()->SaveFrame(url, referrer);
  }
}

} // namespace oxide
