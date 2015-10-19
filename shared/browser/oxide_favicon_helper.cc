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

#include "oxide_favicon_helper.h"

#include "base/logging.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_details.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/favicon_url.h"
#include "url/gurl.h"

#include "oxide_web_view.h"
#include "oxide_web_view_client.h"

namespace oxide {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(FaviconHelper);

FaviconHelper::FaviconHelper(content::WebContents* contents)
    : content::WebContentsObserver(contents) {}

void FaviconHelper::NotifyWebViewOfChange() {
  WebView* view = WebView::FromWebContents(web_contents());
  if (!view) {
    return;
  }

  view->client()->FaviconChanged();
}

void FaviconHelper::DidNavigateMainFrame(
    const content::LoadCommittedDetails& details,
    const content::FrameNavigateParams& params) {
  if (!details.is_navigation_to_different_page()) {
    return;
  }

  NotifyWebViewOfChange();
}

void FaviconHelper::DidUpdateFaviconURL(
    const std::vector<content::FaviconURL>& candidates) {
  DCHECK(!candidates.empty());

  content::NavigationEntry* entry =
      web_contents()->GetController().GetLastCommittedEntry();
  if (!entry) {
    return;
  }

  bool did_change = false;
  for (const auto& candidate : candidates) {
    if (candidate.icon_type != content::FaviconURL::FAVICON) {
      continue;
    }

    if (candidate.icon_url == entry->GetFavicon().url) {
      continue;
    }

    entry->GetFavicon().url = candidate.icon_url;
    did_change = true;
    break;
  }

  if (!did_change) {
    return;
  }

  NotifyWebViewOfChange();
}

FaviconHelper::~FaviconHelper() {}

const GURL& FaviconHelper::GetFaviconURL() const {
  content::NavigationEntry* entry =
      web_contents()->GetController().GetLastCommittedEntry();
  if (!entry) {
    return GURL::EmptyGURL();
  }

  return entry->GetFavicon().url;
}

} // namespace oxide
