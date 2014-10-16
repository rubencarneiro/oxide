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

#ifndef _OXIDE_QT_QUICK_FILE_PICKER_DELEGATE_H_
#define _OXIDE_QT_QUICK_FILE_PICKER_DELEGATE_H_

#include <QScopedPointer>
#include <QtGlobal>

#include "qt/core/glue/oxide_qt_file_picker_delegate.h"

class OxideQQuickWebView;

QT_BEGIN_NAMESPACE
class QQmlContext;
class QQuickItem;
QT_END_NAMESPACE

namespace oxide {
namespace qquick {

class FilePickerDelegate final : public oxide::qt::FilePickerDelegate {
 public:
  FilePickerDelegate(OxideQQuickWebView* webview);

 protected:
  void Show(Mode mode,
            const QString& title,
            const QFileInfo& defaultFileName,
            const QStringList& acceptTypes);
  void Hide();

 private:
  OxideQQuickWebView* web_view_;

  QScopedPointer<QQmlContext> context_;
  QScopedPointer<QQuickItem> item_;
};

} // namespace qquick
} // namespace oxide

#endif // _OXIDE_QT_QUICK_FILE_PICKER_DELEGATE_H_
