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

#include "oxide_pepper_flash_plugin_detection.h"

#include "base/files/file_util.h"

namespace oxide {

namespace {

const base::FilePath::CharType* kPepperFlashPluginDirs[] = {
  FILE_PATH_LITERAL("/usr/lib/adobe-flashplugin"),
  FILE_PATH_LITERAL("/opt/google/chrome/PepperFlash"),
};
const base::FilePath::CharType kPepperFlashPluginFilename[] =
  FILE_PATH_LITERAL("libpepflashplayer.so");

}

base::FilePath GetPepperFlashPluginPath() {
  for (size_t i = 0; i < arraysize(kPepperFlashPluginDirs); ++i) {
    base::FilePath path(
        base::FilePath(kPepperFlashPluginDirs[i])
          .Append(kPepperFlashPluginFilename));
    if (base::PathExists(path)) {
      return path;
    }
  }

  return base::FilePath();
}

} // namespace oxide
