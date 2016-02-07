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

#ifndef _OXIDE_QT_CORE_BROWSER_CONTENTS_NATIVE_VIEW_DATA_H_
#define _OXIDE_QT_CORE_BROWSER_CONTENTS_NATIVE_VIEW_DATA_H_

#include <QPointer>
#include <QtGlobal>

#include "base/macros.h"
#include "content/public/browser/web_contents_user_data.h"

QT_BEGIN_NAMESPACE
class QObject;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class ContentsNativeViewData
    : public content::WebContentsUserData<ContentsNativeViewData> {
 public:
  ~ContentsNativeViewData() override;

  static void CreateForWebContents(content::WebContents* contents,
                                   QObject* native_view);

  QObject* GetNativeView() const;

 private:
  ContentsNativeViewData(QObject* native_view);

  QPointer<QObject> native_view_;

  DISALLOW_COPY_AND_ASSIGN(ContentsNativeViewData);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_CONTENTS_NATIVE_VIEW_DATA_H_
