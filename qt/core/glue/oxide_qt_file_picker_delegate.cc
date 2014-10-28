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

#include <QDir>

#include "base/files/file_path.h"
#include "content/public/common/file_chooser_file_info.h"
#include "content/public/common/file_chooser_params.h"

#include "qt/core/browser/oxide_qt_file_picker.h"

namespace oxide {
namespace qt {

namespace {

content::FileChooserFileInfo MakeFileInfo(const QFileInfo& fi) {
  content::FileChooserFileInfo info;
  info.file_path = base::FilePath(fi.absoluteFilePath().toStdString());
  return info;
}

std::vector<content::FileChooserFileInfo> Enumerate(const QDir& dir) {
  std::vector<content::FileChooserFileInfo> enumerated;
  QDir::Filters filters =
      QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden;
  Q_FOREACH (const QFileInfo& file, dir.entryInfoList(filters)) {
    if (file.isDir()) {
      content::FileChooserFileInfo info;
      QString directoryPath = file.absoluteFilePath() + QStringLiteral("/.");
      info.file_path = base::FilePath(directoryPath.toStdString());
      enumerated.push_back(info);
      std::vector<content::FileChooserFileInfo> contents =
          Enumerate(file.absoluteFilePath());
      enumerated.insert(enumerated.end(), contents.begin(), contents.end());
    } else {
      enumerated.push_back(MakeFileInfo(file));
    }
  }
  return enumerated;
}

}

FilePickerDelegate::FilePickerDelegate() :
    file_picker_(NULL) {}

FilePickerDelegate::~FilePickerDelegate() {}

void FilePickerDelegate::Done(const QFileInfoList& files,
                              FilePickerDelegate::Mode mode) {
  std::vector<content::FileChooserFileInfo> selection;
  if (mode == FilePickerDelegate::UploadFolder) {
    if (!files.isEmpty() && files.first().isDir()) {
      // XXX: chrome does this asynchronously on a background thread
      // (see net::DirectoryLister)
      selection = Enumerate(files.first().absoluteFilePath());
    }
  } else {
    Q_FOREACH (const QFileInfo& file, files) {
      selection.push_back(MakeFileInfo(file));
    }
  }
  content::FileChooserParams::Mode permissions;
  switch (mode) {
    case FilePickerDelegate::Open:
      permissions = content::FileChooserParams::Open;
      break;
    case FilePickerDelegate::OpenMultiple:
      permissions = content::FileChooserParams::OpenMultiple;
      break;
    case FilePickerDelegate::UploadFolder:
      permissions = content::FileChooserParams::UploadFolder;
      break;
    case FilePickerDelegate::Save:
      permissions = content::FileChooserParams::Save;
      break;
    default:
      Q_UNREACHABLE();
  }
  file_picker_->Done(selection, permissions);
}

} // namespace qt
} // namespace oxide
