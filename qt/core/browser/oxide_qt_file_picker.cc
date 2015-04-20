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

#include <vector>

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "base/files/file_path.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/file_chooser_file_info.h"
#include "content/public/common/file_chooser_params.h"

#include "qt/core/glue/oxide_qt_file_picker_proxy.h"

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

FilePicker::~FilePicker() {}

void FilePicker::Run(const content::FileChooserParams& params) {
  FilePickerProxy::Mode mode;
  switch (params.mode) {
    case content::FileChooserParams::Open:
      mode = FilePickerProxy::Open;
      break;
    case content::FileChooserParams::OpenMultiple:
      mode = FilePickerProxy::OpenMultiple;
      break;
    case content::FileChooserParams::UploadFolder:
      mode = FilePickerProxy::UploadFolder;
      break;
    case content::FileChooserParams::Save:
      mode = FilePickerProxy::Save;
      break;
    default:
      Q_UNREACHABLE();
  }
  QString title = QString::fromStdString(base::UTF16ToUTF8(params.title));
  QFileInfo defaultFileName(QString::fromStdString(params.default_file_name.value()));
  QStringList acceptTypes;
  std::vector<base::string16>::const_iterator i;
  for (i = params.accept_types.begin(); i != params.accept_types.end(); ++i) {
    acceptTypes << QString::fromStdString(base::UTF16ToUTF8(*i));
  }
  proxy_->Show(mode, title, defaultFileName, acceptTypes);
}

void FilePicker::Hide() {
  proxy_->Hide();
}

void FilePicker::done(const QFileInfoList& files,
                      FilePickerProxy::Mode mode) {
  std::vector<content::FileChooserFileInfo> selection;
  if (mode == FilePickerProxy::UploadFolder) {
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
    case FilePickerProxy::Open:
      permissions = content::FileChooserParams::Open;
      break;
    case FilePickerProxy::OpenMultiple:
      permissions = content::FileChooserParams::OpenMultiple;
      break;
    case FilePickerProxy::UploadFolder:
      permissions = content::FileChooserParams::UploadFolder;
      break;
    case FilePickerProxy::Save:
      permissions = content::FileChooserParams::Save;
      break;
    default:
      Q_UNREACHABLE();
  }

  Done(selection, permissions);
}

FilePicker::FilePicker(content::RenderViewHost* rvh)
    : oxide::FilePicker(rvh) {}

void FilePicker::SetProxy(FilePickerProxy* proxy) {
  proxy_.reset(proxy);
}

} // namespace qt
} // namespace oxide
