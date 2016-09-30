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

#ifndef _OXIDE_QT_QUICK_API_LOCATION_BAR_CONTROLLER_P_H_
#define _OXIDE_QT_QUICK_API_LOCATION_BAR_CONTROLLER_P_H_

#include <QtGlobal>

#include <memory>

#include "qt/core/glue/chrome_controller_client.h"
#include "qt/core/glue/web_contents_id.h"

class OxideQQuickLocationBarController;

namespace oxide {
namespace qt {
class ChromeController;
}
}

class OxideQQuickLocationBarControllerPrivate
    : public oxide::qt::ChromeControllerClient {
  Q_DECLARE_PUBLIC(OxideQQuickLocationBarController)
  Q_DISABLE_COPY(OxideQQuickLocationBarControllerPrivate)

 public:
  ~OxideQQuickLocationBarControllerPrivate();

  static std::unique_ptr<OxideQQuickLocationBarController> create();

  static OxideQQuickLocationBarControllerPrivate* get(
      OxideQQuickLocationBarController* q);

  void init(oxide::qt::WebContentsID web_contents_id);

 private:
  OxideQQuickLocationBarControllerPrivate(OxideQQuickLocationBarController* q);

  // oxide::qt::ChromeControllerClient implementation
  void ChromePositionUpdated() override;

  OxideQQuickLocationBarController* q_ptr;

  std::unique_ptr<oxide::qt::ChromeController> chrome_controller_;
};

#endif // _OXIDE_QT_QUICK_API_LOCATION_BAR_CONTROLLER_P_H_
