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

#ifndef _OXIDE_QT_UITK_LIB_AUXILIARY_UI_FACTORY_H_
#define _OXIDE_QT_UITK_LIB_AUXILIARY_UI_FACTORY_H_

#include <QtGlobal>

#include "qt/quick/auxiliary_ui_factory.h"

namespace oxide {
namespace uitk {

class AuxiliaryUIFactory : public qquick::AuxiliaryUIFactory {
 public:
  AuxiliaryUIFactory();
  ~AuxiliaryUIFactory() override;

 private:
  // qquick::AuxiliaryUIFactory implementation
  std::unique_ptr<qt::WebContextMenu> CreateWebContextMenu(
      const qt::WebContextMenuParams& params,
      const std::vector<qt::MenuItem>& items,
      qt::WebContextMenuClient* client) override;

  Q_DISABLE_COPY(AuxiliaryUIFactory)
};

} // namespace uitk
} // namespace oxide

#endif // _OXIDE_QT_UITK_LIB_AUXILIARY_UI_FACTORY_H_
