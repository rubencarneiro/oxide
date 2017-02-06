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

#ifndef _OXIDE_UITK_LIB_CONTENTS_VIEW_H_
#define _OXIDE_UITK_LIB_CONTENTS_VIEW_H_

#include <QtGlobal>

#include "qt/quick/contents_view.h"

namespace oxide {
namespace uitk {

class ContentsView : public qquick::ContentsView {
  Q_DISABLE_COPY(ContentsView)

 public:
  ContentsView(QQuickItem* item);
  ~ContentsView() override;

 private:
  // qt::ContentsViewClient implementation
  std::unique_ptr<qt::WebPopupMenu> CreateWebPopupMenu(
      const std::vector<qt::MenuItem>& items,
      bool allow_multiple_selection,
      const QRect& bounds,
      qt::WebPopupMenuClient* client) override;
  qt::TouchHandleDrawableProxy* CreateTouchHandleDrawable() override;
  void TouchSelectionChanged(
      qt::TouchSelectionControllerActiveStatus status,
      const QRectF& bounds,
      bool handle_drag_in_progress,
      bool insertion_handle_tapped) override;
  void ContextMenuIntercepted() const override;
};

} // namespace uitk
} // namespace oxide

#endif // _OXIDE_UITK_LIB_CONTENTS_VIEW_H_
