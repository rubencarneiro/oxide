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

#include "oxide_qt_file_picker.h"

#include <QFileInfo>
#include <QString>
#include <QStringList>

#include "base/strings/utf_string_conversions.h"

#include "qt/core/glue/oxide_qt_file_picker_delegate.h"

namespace oxide {
namespace qt {

FilePicker::FilePicker(FilePickerDelegate* delegate,
                       content::RenderViewHost* rvh) :
    oxide::FilePicker(rvh),
    delegate_(delegate) {
  delegate_->file_picker_ = this;
}

void FilePicker::Run(const content::FileChooserParams& params) {
  FilePickerDelegate::Mode mode;
  if (params.mode == content::FileChooserParams::Open) {
    mode = FilePickerDelegate::Open;
  } else if (params.mode == content::FileChooserParams::OpenMultiple) {
    mode = FilePickerDelegate::OpenMultiple;
  } else if (params.mode == content::FileChooserParams::UploadFolder) {
    mode = FilePickerDelegate::UploadFolder;
  } else if (params.mode == content::FileChooserParams::Save) {
    mode = FilePickerDelegate::Save;
  }
  QString title = QString::fromStdString(base::UTF16ToUTF8(params.title));
  QFileInfo defaultFileName(QString::fromStdString(params.default_file_name.value()));
  QStringList acceptTypes;
  std::vector<base::string16>::const_iterator i;
  for (i = params.accept_types.begin(); i != params.accept_types.end(); ++i) {
    acceptTypes << QString::fromStdString(base::UTF16ToUTF8(*i));
  }
  delegate_->Show(mode, title, defaultFileName, acceptTypes);
}

} // namespace qt
} // namespace oxide
