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

#include "oxide_web_contents_view_client.h"

#include <cmath>

#include "base/logging.h"

#include "oxide_web_contents_view.h"

namespace oxide {

WebContentsViewClient::WebContentsViewClient()
    : view_(nullptr) {}

WebContentsViewClient::~WebContentsViewClient() {
  if (view_) {
    view_->SetClient(nullptr);
  }
  DCHECK(!view_);
}

gfx::Rect WebContentsViewClient::GetBoundsDip() const {
  float scale = 1.0f / GetScreenInfo().deviceScaleFactor;
  gfx::Rect bounds(GetBoundsPix());

  int x = std::lround(bounds.x() * scale);
  int y = std::lround(bounds.y() * scale);
  int width = std::lround(bounds.width() * scale);
  int height = std::lround(bounds.height() * scale);

  return gfx::Rect(x, y, width, height);
}

WebContextMenu* WebContentsViewClient::CreateContextMenu(
    content::RenderFrameHost* rfh,
    const content::ContextMenuParams& params) {
  NOTIMPLEMENTED();
  return nullptr;
}

WebPopupMenu* WebContentsViewClient::CreatePopupMenu(
    content::RenderFrameHost* rfh) {
  NOTIMPLEMENTED();
  return nullptr;
}

} // namespace oxide
