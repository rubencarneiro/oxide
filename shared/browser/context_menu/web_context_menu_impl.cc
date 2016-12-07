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

#include <libintl.h>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_task_runner_handle.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_url_parameters.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/referrer.h"
#include "net/http/http_request_headers.h"
#include "ui/base/clipboard/scoped_clipboard_writer.h"
#include "url/gurl.h"

#include "shared/browser/chrome_controller.h"
#include "shared/browser/web_contents_client.h"
#include "shared/browser/web_contents_helper.h"
#include "shared/common/oxide_constants.h"

#include "web_context_menu_actions.h"
#include "web_context_menu_sections.h"

namespace oxide {

namespace {

void WriteURLToClipboard(const GURL& url) {
  if (url.is_empty() || !url.is_valid()) {
    return;
  }

  ui::ScopedClipboardWriter scw(ui::CLIPBOARD_TYPE_COPY_PASTE);
  scw.WriteURL(base::UTF8ToUTF16(url.spec()));
}

}

class WebContextMenuImpl::MenuBuilder {
 public:
  MenuBuilder(WebContextMenuImpl* menu,
              std::vector<content::MenuItem>* items)
      : menu_(menu),
        items_(items) {}

  void BeginSection(WebContextMenuSection section) {
    content::MenuItem item;
    item.type = content::MenuItem::GROUP;
    item.action = static_cast<unsigned>(section);

    items_->push_back(item);
  }

  void AppendMenuItem(const base::StringPiece& label,
                      WebContextMenuAction action) {
    content::MenuItem item;
    item.label = base::UTF8ToUTF16(label);
    item.type = content::MenuItem::OPTION;
    item.action = static_cast<unsigned>(action);
    item.enabled = menu_->IsCommandEnabled(action);

    items_->push_back(item);
  }

 private:
  WebContextMenuImpl* menu_;
  std::vector<content::MenuItem>* items_;
};

content::Referrer WebContextMenuImpl::GetReferrer(const GURL& url) const {
  GURL referring_url =
      params_.frame_url.is_empty() ? params_.page_url : params_.frame_url;
  return content::Referrer::SanitizeForRequest(
      url,
      content::Referrer(referring_url.GetAsReferrer(),
                        params_.referrer_policy));
}

void WebContextMenuImpl::OpenURL(const GURL& url,
                                 WindowOpenDisposition disposition) {
  content::OpenURLParams params(url, GetReferrer(url), disposition,
                                ui::PAGE_TRANSITION_LINK,
                                false, true);
  web_contents()->OpenURL(params);
}

void WebContextMenuImpl::SaveLink() {
  content::BrowserContext* context = web_contents()->GetBrowserContext();
  content::DownloadManager* dlm =
      content::BrowserContext::GetDownloadManager(context);
  const GURL& url = params_.link_url;
  std::unique_ptr<content::DownloadUrlParameters> dl_params(
      content::DownloadUrlParameters::CreateForWebContentsMainFrame(
          web_contents(), url));
  dl_params->set_referrer(GetReferrer(url));
  dl_params->set_referrer_encoding(params_.frame_charset);
  dl_params->set_suggested_name(params_.suggested_filename);
  dl_params->set_prompt(true);

  dlm->DownloadUrl(std::move(dl_params));
}

void WebContextMenuImpl::SaveImage() {
  bool is_large_data_url =
      params_.has_image_contents && params_.src_url.is_empty();
  if (is_large_data_url) {
    render_frame_host_->SaveImageAt(params_.x, params_.y);
  }

  const GURL& url = params_.src_url;

  // XXX(oSoMoN): see comment in
  // oxide::ResourceDispatcherHostDelegate::DispatchDownloadRequest(…).
  auto it = params_.properties.find(oxide::kImageContextMenuPropertiesMimeType);
  std::string headers;
  if (it != params_.properties.cend()) {
    headers.append(net::HttpRequestHeaders::kContentType);
    headers.append(": ");
    headers.append(it->second);
  }

  web_contents()->SaveFrameWithHeaders(url, GetReferrer(url), headers);
}

void WebContextMenuImpl::CopyImage() {
  render_frame_host_->CopyImageAt(params_.x, params_.y);
}

void WebContextMenuImpl::SaveMedia() {
  const GURL& url = params_.src_url;
  web_contents()->SaveFrame(url, GetReferrer(url));
}

void WebContextMenuImpl::AppendLinkItems(std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(web_contents())->client();
  if (contents_client && contents_client->CanCreateWindows()) {
    builder.BeginSection(WebContextMenuSection::OpenLink);

    builder.AppendMenuItem(
        dgettext(OXIDE_GETTEXT_DOMAIN, "Open link in new tab"),
        WebContextMenuAction::OpenLinkInNewTab);
    builder.AppendMenuItem(
        dgettext(OXIDE_GETTEXT_DOMAIN, "Open link in new background tab"),
        WebContextMenuAction::OpenLinkInNewBackgroundTab);
    builder.AppendMenuItem(
        dgettext(OXIDE_GETTEXT_DOMAIN, "Open link in new window"),
        WebContextMenuAction::OpenLinkInNewWindow);
  }

  builder.BeginSection(WebContextMenuSection::Link);

  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy link address"),
                         WebContextMenuAction::CopyLinkLocation);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Save link"),
                         WebContextMenuAction::SaveLink);
}

void WebContextMenuImpl::AppendImageItems(
    std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(web_contents())->client();
  if (contents_client && contents_client->CanCreateWindows()) {
    builder.AppendMenuItem(
        dgettext(OXIDE_GETTEXT_DOMAIN, "Open image in new tab"),
        WebContextMenuAction::OpenImageInNewTab);
  }
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy image address"),
                         WebContextMenuAction::CopyImageLocation);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Save image"),
                         WebContextMenuAction::SaveImage);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy image"),
                         WebContextMenuAction::CopyImage);
}

void WebContextMenuImpl::AppendCanvasItems(
    std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Save image"),
                         WebContextMenuAction::SaveImage);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy image"),
                         WebContextMenuAction::CopyImage);
}

void WebContextMenuImpl::AppendAudioItems(
    std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(web_contents())->client();
  if (contents_client && contents_client->CanCreateWindows()) {
    builder.AppendMenuItem(
        dgettext(OXIDE_GETTEXT_DOMAIN, "Open audio in new tab"),
        WebContextMenuAction::OpenMediaInNewTab);
  }
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy audio address"),
                         WebContextMenuAction::CopyMediaLocation);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Save audio"),
                         WebContextMenuAction::SaveMedia);
}

void WebContextMenuImpl::AppendVideoItems(
    std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(web_contents())->client();
  if (contents_client && contents_client->CanCreateWindows()) {
    builder.AppendMenuItem(
        dgettext(OXIDE_GETTEXT_DOMAIN, "Open video in new tab"),
        WebContextMenuAction::OpenMediaInNewTab);
  }
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy video address"),
                         WebContextMenuAction::CopyMediaLocation);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Save video"),
                         WebContextMenuAction::SaveMedia);
}

void WebContextMenuImpl::AppendEditableItems(
    std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  builder.BeginSection(WebContextMenuSection::Undo);

  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Undo"),
                         WebContextMenuAction::Undo);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Redo"),
                         WebContextMenuAction::Redo);

  builder.BeginSection(WebContextMenuSection::Editing);

  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Cut"),
                         WebContextMenuAction::Cut);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy"),
                         WebContextMenuAction::Copy);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Paste"),
                         WebContextMenuAction::Paste);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Erase"),
                         WebContextMenuAction::Erase);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Select all"),
                         WebContextMenuAction::SelectAll);
}

void WebContextMenuImpl::AppendCopyItems(std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  builder.BeginSection(WebContextMenuSection::Copy);

  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy"),
                         WebContextMenuAction::Copy);
}

bool WebContextMenuImpl::IsCommandEnabled(WebContextMenuAction action) const {
  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(web_contents())->client();

  bool can_create_windows =
      contents_client && contents_client->CanCreateWindows();

  switch (action) {
    case WebContextMenuAction::OpenLinkInNewTab:
    case WebContextMenuAction::OpenLinkInNewBackgroundTab:
    case WebContextMenuAction::OpenLinkInNewWindow:
      return can_create_windows && params_.link_url.is_valid();

    case WebContextMenuAction::CopyLinkLocation:
    case WebContextMenuAction::SaveLink:
      return params_.link_url.is_valid();

    case WebContextMenuAction::OpenImageInNewTab:
      return can_create_windows && params_.src_url.is_valid();

    case WebContextMenuAction::CopyImageLocation:
      return params_.src_url.is_valid();

    case WebContextMenuAction::SaveImage:
    case WebContextMenuAction::CopyImage:
      return params_.has_image_contents;

    case WebContextMenuAction::OpenMediaInNewTab:
      return can_create_windows &&
             params_.src_url.is_valid() &&
             params_.media_flags & blink::WebContextMenuData::MediaCanSave;

    case WebContextMenuAction::CopyMediaLocation:
      return params_.src_url.is_valid();

    case WebContextMenuAction::SaveMedia:
      return params_.src_url.is_valid() &&
             params_.media_flags & blink::WebContextMenuData::MediaCanSave;

    case WebContextMenuAction::Undo:
      return params_.edit_flags & blink::WebContextMenuData::CanUndo;

    case WebContextMenuAction::Redo:
      return params_.edit_flags & blink::WebContextMenuData::CanRedo;

    case WebContextMenuAction::Cut:
      return params_.edit_flags & blink::WebContextMenuData::CanCut;

    case WebContextMenuAction::Copy:
      return params_.edit_flags & blink::WebContextMenuData::CanCopy;

    case WebContextMenuAction::Paste:
      return params_.edit_flags & blink::WebContextMenuData::CanPaste;

    case WebContextMenuAction::Erase:
      return params_.edit_flags & blink::WebContextMenuData::CanDelete;

    case WebContextMenuAction::SelectAll:
      return params_.edit_flags & blink::WebContextMenuData::CanSelectAll;
  }

  NOTREACHED();
  return false;
}

std::vector<content::MenuItem> WebContextMenuImpl::BuildItems() {
  std::vector<content::MenuItem> items;

  if (!params_.unfiltered_link_url.is_empty()) {
    AppendLinkItems(&items);
  }

  if (params_.media_type == blink::WebContextMenuData::MediaTypeImage ||
      params_.media_type == blink::WebContextMenuData::MediaTypeCanvas ||
      params_.media_type == blink::WebContextMenuData::MediaTypeAudio ||
      params_.media_type == blink::WebContextMenuData::MediaTypeVideo) {
    MenuBuilder(this, &items).BeginSection(WebContextMenuSection::Media);

    if (params_.media_type == blink::WebContextMenuData::MediaTypeImage) {
      AppendImageItems(&items);
    } else if (params_.media_type == blink::WebContextMenuData::MediaTypeCanvas) {
      AppendCanvasItems(&items);
    } else if (params_.media_type == blink::WebContextMenuData::MediaTypeAudio) {
      AppendAudioItems(&items);
    } else if (params_.media_type == blink::WebContextMenuData::MediaTypeVideo) {
      AppendVideoItems(&items);
    }
  }

  if (params_.is_editable) {
    AppendEditableItems(&items);
  } else if (!params_.selection_text.empty()) {
    AppendCopyItems(&items);
  }

  return std::move(items);
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

void WebContextMenuImpl::ExecuteCommand(WebContextMenuAction action) {
  if (!render_frame_host_) {
    return;
  }

  if (!IsCommandEnabled(action)) {
    LOG(ERROR)
        << "Attempting to execute disabled action: "
        << static_cast<unsigned>(action);
    return;
  }

  switch (action) {
    case WebContextMenuAction::OpenLinkInNewTab:
      return OpenURL(params_.link_url,
                     WindowOpenDisposition::NEW_FOREGROUND_TAB);

    case WebContextMenuAction::OpenLinkInNewBackgroundTab:
      return OpenURL(params_.link_url,
                     WindowOpenDisposition::NEW_BACKGROUND_TAB);

    case WebContextMenuAction::OpenLinkInNewWindow:
      return OpenURL(params_.link_url,
                     WindowOpenDisposition::NEW_WINDOW);

    case WebContextMenuAction::CopyLinkLocation:
      return WriteURLToClipboard(params_.unfiltered_link_url);

    case WebContextMenuAction::SaveLink:
      return SaveLink();

    case WebContextMenuAction::OpenImageInNewTab:
    case WebContextMenuAction::OpenMediaInNewTab:
      return OpenURL(params_.src_url,
                     WindowOpenDisposition::NEW_BACKGROUND_TAB);

    case WebContextMenuAction::CopyImageLocation:
    case WebContextMenuAction::CopyMediaLocation:
      return WriteURLToClipboard(params_.src_url);

    case WebContextMenuAction::SaveImage:
      return SaveImage();

    case WebContextMenuAction::CopyImage:
      return CopyImage();

    case WebContextMenuAction::SaveMedia:
      return SaveMedia();

    case WebContextMenuAction::Undo:
      return web_contents()->Undo();

    case WebContextMenuAction::Redo:
      return web_contents()->Redo();

    case WebContextMenuAction::Cut:
      return web_contents()->Cut();

    case WebContextMenuAction::Copy:
      return web_contents()->Copy();

    case WebContextMenuAction::Paste:
      return web_contents()->Paste();

    case WebContextMenuAction::Erase:
      return web_contents()->Delete();

    case WebContextMenuAction::SelectAll:
      return web_contents()->SelectAll();
  };
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

  ChromeController* chrome_controller =
      ChromeController::FromWebContents(web_contents());
  if (chrome_controller) {
    params_.y += chrome_controller->GetTopContentOffset();
  }

  DCHECK(!menu_);
  menu_ = helper->client()->CreateContextMenu(params_, BuildItems(), this);

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
