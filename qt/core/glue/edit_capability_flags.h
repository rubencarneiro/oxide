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

#ifndef _OXIDE_QT_CORE_GLUE_EDIT_CAPABILITY_FLAGS_H_
#define _OXIDE_QT_CORE_GLUE_EDIT_CAPABILITY_FLAGS_H_

#include <QFlags>

namespace oxide {
namespace qt {

enum EditCapabilityFlag {
  EDIT_CAPABILITY_NONE = 0,
  EDIT_CAPABILITY_UNDO = 1 << 0,
  EDIT_CAPABILITY_REDO = 1 << 1,
  EDIT_CAPABILITY_CUT = 1 << 2,
  EDIT_CAPABILITY_COPY = 1 << 3,
  EDIT_CAPABILITY_PASTE = 1 << 4,
  EDIT_CAPABILITY_ERASE = 1 << 5,
  EDIT_CAPABILITY_SELECT_ALL = 1 << 6
};

Q_DECLARE_FLAGS(EditCapabilityFlags, EditCapabilityFlag);

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GLUE_EDIT_CAPABILITY_FLAGS_H_
