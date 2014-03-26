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

#ifndef _OXIDE_QT_CORE_GLUE_FILE_PICKER_DELEGATE_H_
#define _OXIDE_QT_CORE_GLUE_FILE_PICKER_DELEGATE_H_

#include <QFileInfo>
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QString;
class QStringList;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class FilePicker;

class Q_DECL_EXPORT FilePickerDelegate {
 public:
  // Matches chromium’s content/public/common/file_chooser_params.h
  enum Mode {
    Open,
    OpenMultiple,
    UploadFolder,
    Save,
  };

  virtual ~FilePickerDelegate();

  virtual void Show(Mode mode,
                    const QString& title,
                    const QFileInfo& defaultFileName,
                    const QStringList& acceptTypes) = 0;
  void Done(const QFileInfoList& files, Mode mode);

 protected:
  FilePickerDelegate();

  friend class FilePicker;

private:
  FilePicker* file_picker_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_FILE_PICKER_DELEGATE_H_
