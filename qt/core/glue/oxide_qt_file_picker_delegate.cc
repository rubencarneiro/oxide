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

#include "oxide_qt_file_picker_delegate.h"

#include <vector>

#include <QFileInfo>

#include "base/files/file_path.h"
#include "content/public/common/file_chooser_params.h"
#include "ui/shell_dialogs/selected_file_info.h"

#include "qt/core/browser/oxide_qt_file_picker.h"

namespace oxide {
namespace qt {

FilePickerDelegate::FilePickerDelegate() :
    file_picker_(NULL) {}

FilePickerDelegate::~FilePickerDelegate() {}

void FilePickerDelegate::Done(const QList<QFileInfo>& files,
                              FilePickerDelegate::Mode mode) {
  std::vector<ui::SelectedFileInfo> selection;
  Q_FOREACH(const QFileInfo& file, files) {
    base::FilePath path(file.filePath().toStdString());
    selection.push_back(ui::SelectedFileInfo(path, path));
  }
  content::FileChooserParams::Mode permissions;
  if (mode == FilePickerDelegate::Open) {
    permissions = content::FileChooserParams::Open;
  } else if (mode == FilePickerDelegate::OpenMultiple) {
    permissions = content::FileChooserParams::OpenMultiple;
  } else if (mode == FilePickerDelegate::UploadFolder) {
    permissions = content::FileChooserParams::UploadFolder;
  } else if (mode == FilePickerDelegate::Save) {
    permissions = content::FileChooserParams::Save;
  }
  file_picker_->Done(selection, permissions);
}

} // namespace qt
} // namespace oxide
