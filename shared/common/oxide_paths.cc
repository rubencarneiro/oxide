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

#include "oxide_paths.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/path_service.h"

#include "oxide_constants.h"

namespace oxide {

namespace {

bool PathProvider(int key, base::FilePath* result) {
  switch (key) {
    case FILE_PEPPER_FLASH_PLUGIN: {
      base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
      if (!command_line->HasSwitch(switches::kPepperFlashPluginPath)) {
        return false;
      }

      *result =
          command_line->GetSwitchValuePath(switches::kPepperFlashPluginPath);
      return true;
    }

    default:
      return false;
  }
}

} // namespace

void RegisterPathProvider() {
  PathService::RegisterProvider(PathProvider, PATH_START, PATH_END);
}

} // namespace oxide
