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

#ifndef _OXIDE_QT_CORE_GLUE_MENU_ITEM_H_
#define _OXIDE_QT_CORE_GLUE_MENU_ITEM_H_

#include <QString>

#include "qt/core/api/oxideqglobal.h"

namespace oxide {
namespace qt {

struct OXIDE_QTCORE_EXPORT MenuItem {
  MenuItem();
  MenuItem(const MenuItem& other);
  ~MenuItem();

  enum class Type {
    Option = 0,
    Group = 2,
    Separator = 3
  };

  QString label;
  QString tooltip;
  Type type = Type::Option;
  unsigned action = 0;
  bool enabled = false;
  bool checked = false;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_MENU_ITEM_H_
