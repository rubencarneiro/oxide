// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2015 Canonical Ltd.

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

#ifndef _OXIDE_SHARED_BROWSER_TOUCH_SELECTION_CONTROLLER_CLIENT_H_
#define _OXIDE_SHARED_BROWSER_TOUCH_SELECTION_CONTROLLER_CLIENT_H_

#include "base/macros.h"
#include "ui/touch_selection/touch_selection_controller.h"

namespace oxide {

class RenderWidgetHostView;

class TouchSelectionControllerClient final
    : public ui::TouchSelectionControllerClient {
 public:
  explicit TouchSelectionControllerClient(RenderWidgetHostView* rwhv);
  ~TouchSelectionControllerClient() override;

  void OnScrollStarted();
  void OnScrollCompleted();

 private:
  // ui::TouchSelectionControllerClient:
  bool SupportsAnimation() const override;
  void SetNeedsAnimate() override;
  void MoveCaret(const gfx::PointF& position) override;
  void MoveRangeSelectionExtent(const gfx::PointF& extent) override;
  void SelectBetweenCoordinates(const gfx::PointF& base,
                                const gfx::PointF& extent) override;
  void OnSelectionEvent(ui::SelectionEventType event) override;
  scoped_ptr<ui::TouchHandleDrawable> CreateDrawable() override;

  RenderWidgetHostView* rwhv_;

  DISALLOW_COPY_AND_ASSIGN(TouchSelectionControllerClient);
};

} // namespace oxide

#endif // _OXIDE_SHARED_BROWSER_TOUCH_SELECTION_CONTROLLER_CLIENT_H_
