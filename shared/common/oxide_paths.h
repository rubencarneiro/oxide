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

#ifndef _OXIDE_SHARED_COMMON_PATHS_H_
#define _OXIDE_SHARED_COMMON_PATHS_H_

namespace oxide {

enum {
  PATH_START = 1000,

  DIR_GTALK_PLUGIN,
  FILE_GTALK_PLUGIN,
  FILE_O1D_PLUGIN,
  DIR_PEPPER_FLASH_PLUGIN,
  FILE_PEPPER_FLASH_PLUGIN,

  PATH_END
};

void RegisterPathProvider();

} // namespace oxide

#endif // _OXIDE_SHARED_COMMON_PATHS_H_
