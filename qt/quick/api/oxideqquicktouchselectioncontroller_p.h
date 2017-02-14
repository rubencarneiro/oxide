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

#ifndef _OXIDE_QT_QUICK_API_TOUCH_SELECTION_CONTROLLER_P_H_
#define _OXIDE_QT_QUICK_API_TOUCH_SELECTION_CONTROLLER_P_H_

#include <QPointer>
#include <QRectF>

#include "qt/core/glue/legacy_touch_editing_client.h"
#include "qt/quick/api/oxideqquicktouchselectioncontroller.h"

QT_BEGIN_NAMESPACE
class QQmlComponent;
QT_END_NAMESPACE

class OxideQQuickTouchSelectionControllerPrivate
    : public oxide::qt::LegacyTouchEditingClient {
  Q_DECLARE_PUBLIC(OxideQQuickTouchSelectionController)
  Q_DISABLE_COPY(OxideQQuickTouchSelectionControllerPrivate)

 public:
  static OxideQQuickTouchSelectionControllerPrivate* get(
      OxideQQuickTouchSelectionController* q);

 private:
  OxideQQuickTouchSelectionControllerPrivate(
      OxideQQuickTouchSelectionController* q);

  // oxide::qt::LegacyTouchSelectionControllerPrivate
  void StatusChanged(ActiveStatus status,
                     const QRectF& bounds,
                     bool handle_drag_in_progress) override;
  void InsertionHandleTapped() override;
  void ContextMenuIntercepted() override;

  OxideQQuickTouchSelectionController* q_ptr;

  QPointer<QQmlComponent> handle_;
  QRectF bounds_;
  bool handle_drag_in_progress_;
  OxideQQuickTouchSelectionController::Status status_;
};

#endif // _OXIDE_QT_QUICK_API_TOUCH_SELECTION_CONTROLLER_P_H_
