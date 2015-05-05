// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2015 Canonical Ltd.

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

#include "oxide_file_picker.h"

#include "base/logging.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/file_chooser_file_info.h"

namespace oxide {

FilePicker::FilePicker(content::RenderViewHost* rvh)
    : content::WebContentsObserver(content::WebContents::FromRenderViewHost(rvh)),
      render_view_host_(rvh) {}

FilePicker::~FilePicker() {
  DCHECK(!render_view_host_);
}

void FilePicker::RenderViewDeleted(content::RenderViewHost* rvh) {
  if (rvh != render_view_host_) {
    return;
  }
  render_view_host_ = nullptr;
  Hide();
  content::BrowserThread::DeleteSoon(
      content::BrowserThread::UI, FROM_HERE, this);
}

void FilePicker::Hide() {}

void FilePicker::Done(const std::vector<content::FileChooserFileInfo>& files,
                      content::FileChooserParams::Mode permissions) {
  render_view_host_->FilesSelectedInChooser(files, permissions);
  render_view_host_ = nullptr;
  Hide();
  content::BrowserThread::DeleteSoon(
      content::BrowserThread::UI, FROM_HERE, this);
}

} // namespace oxide
