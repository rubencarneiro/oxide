// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_FILE_PICKER_H_
#define _OXIDE_SHARED_BROWSER_FILE_PICKER_H_

#include <vector>

#include "base/memory/weak_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/common/file_chooser_params.h"
#include "ui/shell_dialogs/selected_file_info.h"

namespace content {
class RenderViewHost;
}

namespace oxide {

class FilePicker : public content::WebContentsObserver,
                   public base::SupportsWeakPtr<FilePicker> {
 public:
  virtual ~FilePicker();

  virtual void Run(const content::FileChooserParams& params) = 0;
  void Done(const std::vector<ui::SelectedFileInfo>& files,
            content::FileChooserParams::Mode permissions);

 protected:
  FilePicker(content::RenderViewHost* rvh);

 private:
  void RenderViewDeleted(content::RenderViewHost* rvh) FINAL;

  content::RenderViewHost* render_view_host_;
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_FILE_PICKER_H_
