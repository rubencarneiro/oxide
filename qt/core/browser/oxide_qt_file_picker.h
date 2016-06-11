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

#ifndef _OXIDE_QT_CORE_BROWSER_FILE_PICKER_H_
#define _OXIDE_QT_CORE_BROWSER_FILE_PICKER_H_

#include <memory>

#include "base/macros.h"

#include "qt/core/glue/oxide_qt_file_picker_proxy_client.h"
#include "shared/browser/oxide_file_picker.h"

namespace oxide {
namespace qt {

class FilePickerProxy;

class FilePicker : public oxide::FilePicker,
                   public FilePickerProxyClient {
 public:
  FilePicker(content::RenderFrameHost* rfh);

  void SetProxy(FilePickerProxy* proxy);

 private:
  ~FilePicker() override;

  // oxide::FilePicker implementation
  void Run(const content::FileChooserParams& params) override;
  void Hide() override;

  // FilePickerProxyClient implementation
  void done(const QFileInfoList& files,
            FilePickerProxy::Mode mode) override;

  std::unique_ptr<FilePickerProxy> proxy_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(FilePicker);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_FILE_PICKER_H_
