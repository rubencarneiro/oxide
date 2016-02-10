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

#include "oxide_qt_contents_native_view_data.h"

#include <QObject>

namespace oxide {
namespace qt {

DEFINE_WEB_CONTENTS_USER_DATA_KEY(ContentsNativeViewData);

ContentsNativeViewData::ContentsNativeViewData(QObject* native_view)
    : native_view_(native_view) {}

ContentsNativeViewData::~ContentsNativeViewData() {}

// static
void ContentsNativeViewData::CreateForWebContents(
    content::WebContents* contents,
    QObject* native_view) {
  DCHECK(contents);
  DCHECK(native_view);
  if (!FromWebContents(contents)) {
    contents->SetUserData(UserDataKey(),
                          new ContentsNativeViewData(native_view));
  }
}

QObject* ContentsNativeViewData::GetNativeView() const {
  return native_view_;
}

} // namespace qt
} // namespace oxide
