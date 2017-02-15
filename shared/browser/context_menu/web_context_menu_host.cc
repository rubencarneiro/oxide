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

#include "web_context_menu_host.h"

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

#include "web_context_menu.h"
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

class WebContextMenuHost::MenuBuilder {
 public:
  MenuBuilder(WebContextMenuHost* menu,
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
  WebContextMenuHost* menu_;
  std::vector<content::MenuItem>* items_;
};

content::Referrer WebContextMenuHost::GetReferrer(const GURL& url) const {
  GURL referring_url =
      params_.frame_url.is_empty() ? params_.page_url : params_.frame_url;
  return content::Referrer::SanitizeForRequest(
      url,
      content::Referrer(referring_url.GetAsReferrer(),
                        params_.referrer_policy));
}

void WebContextMenuHost::OpenURL(const GURL& url,
                                 WindowOpenDisposition disposition) {
  content::OpenURLParams params(url, GetReferrer(url), disposition,
                                ui::PAGE_TRANSITION_LINK,
                                false, true);
  GetWebContents()->OpenURL(params);
}

void WebContextMenuHost::SaveLink() {
  content::BrowserContext* context = GetWebContents()->GetBrowserContext();
  content::DownloadManager* dlm =
      content::BrowserContext::GetDownloadManager(context);
  const GURL& url = params_.link_url;
  std::unique_ptr<content::DownloadUrlParameters> dl_params(
      content::DownloadUrlParameters::CreateForWebContentsMainFrame(
          GetWebContents(), url));
  dl_params->set_referrer(GetReferrer(url));
  dl_params->set_referrer_encoding(params_.frame_charset);
  dl_params->set_suggested_name(params_.suggested_filename);
  dl_params->set_prompt(true);

  dlm->DownloadUrl(std::move(dl_params));
}

void WebContextMenuHost::SaveImage() {
  bool is_large_data_url =
      params_.has_image_contents && params_.src_url.is_empty();
  if (is_large_data_url) {
    GetRenderFrameHost()->SaveImageAt(params_.x, params_.y);
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

  GetWebContents()->SaveFrameWithHeaders(url, GetReferrer(url), headers);
}

void WebContextMenuHost::CopyImage() {
  GetRenderFrameHost()->CopyImageAt(params_.x, params_.y);
}

void WebContextMenuHost::SaveMedia() {
  const GURL& url = params_.src_url;
  GetWebContents()->SaveFrame(url, GetReferrer(url));
}

void WebContextMenuHost::AppendLinkItems(std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(GetWebContents())->client();
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

void WebContextMenuHost::AppendImageItems(
    std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(GetWebContents())->client();
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

void WebContextMenuHost::AppendCanvasItems(
    std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Save image"),
                         WebContextMenuAction::SaveImage);
  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy image"),
                         WebContextMenuAction::CopyImage);
}

void WebContextMenuHost::AppendAudioItems(
    std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(GetWebContents())->client();
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

void WebContextMenuHost::AppendVideoItems(
    std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(GetWebContents())->client();
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

void WebContextMenuHost::AppendEditableItems(
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

void WebContextMenuHost::AppendCopyItems(std::vector<content::MenuItem>* items) {
  MenuBuilder builder(this, items);

  builder.BeginSection(WebContextMenuSection::Copy);

  builder.AppendMenuItem(dgettext(OXIDE_GETTEXT_DOMAIN, "Copy"),
                         WebContextMenuAction::Copy);
}

bool WebContextMenuHost::IsCommandEnabled(WebContextMenuAction action) const {
  WebContentsClient* contents_client =
      WebContentsHelper::FromWebContents(GetWebContents())->client();

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

std::vector<content::MenuItem> WebContextMenuHost::BuildItems() {
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

content::WebContents* WebContextMenuHost::GetWebContents() const {
  return content::WebContents::FromRenderFrameHost(GetRenderFrameHost());
}

void WebContextMenuHost::Close() {
  if (on_close_callback_.is_null()) {
    return;
  }

  base::Closure on_close_callback = std::move(on_close_callback_);

  if (menu_) {
    menu_->Hide();
  }

  on_close_callback.Run();
}

void WebContextMenuHost::ExecuteCommand(WebContextMenuAction action) {
  if (on_close_callback_.is_null()) {
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
      return GetWebContents()->Undo();

    case WebContextMenuAction::Redo:
      return GetWebContents()->Redo();

    case WebContextMenuAction::Cut:
      return GetWebContents()->Cut();

    case WebContextMenuAction::Copy:
      return GetWebContents()->Copy();

    case WebContextMenuAction::Paste:
      return GetWebContents()->Paste();

    case WebContextMenuAction::Erase:
      return GetWebContents()->Delete();

    case WebContextMenuAction::SelectAll:
      return GetWebContents()->SelectAll();
  };
}

WebContextMenuHost::WebContextMenuHost(
    content::RenderFrameHost* render_frame_host,
    const content::ContextMenuParams& params,
    const base::Closure& on_close_callback)
    : render_frame_host_id_(render_frame_host),
      params_(params),
      on_close_callback_(on_close_callback) {
  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(render_frame_host);
  DCHECK(web_contents);

  WebContentsHelper* helper =
      WebContentsHelper::FromWebContents(web_contents);
  if (!helper->client()) {
    return;
  }

  ChromeController* chrome_controller =
      ChromeController::FromWebContents(web_contents);
  if (chrome_controller) {
    params_.y += chrome_controller->GetTopContentOffset();
  }

  menu_ = helper->client()->CreateContextMenu(params_, BuildItems(), this);
}

WebContextMenuHost::~WebContextMenuHost() = default;

void WebContextMenuHost::Show() {
  if (!menu_) {
    Close();
    return;
  }

  menu_->Show();
}

content::RenderFrameHost* WebContextMenuHost::GetRenderFrameHost() const {
  return render_frame_host_id_.ToInstance();
}

} // namespace oxide
