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

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/path_service.h"

namespace oxide {

namespace {

const base::FilePath::CharType kGoogleTalkPluginDirs[][25] = {
  FILE_PATH_LITERAL("/opt/google/talkplugin"),
  FILE_PATH_LITERAL("/opt/google/chrome")
};
const base::FilePath::CharType kGoogleTalkPluginFilename[] =
    FILE_PATH_LITERAL("libppgoogletalk.so");
const base::FilePath::CharType kO1DPluginFilename[] =
    FILE_PATH_LITERAL("libppo1d.so");

bool PathProvider(int key, base::FilePath* result) {
  switch (key) {
    case DIR_GTALK_PLUGIN: {
      for (size_t i = 0; i < arraysize(kGoogleTalkPluginDirs); ++i) {
        base::FilePath path(kGoogleTalkPluginDirs[i]);
        if (base::PathExists(path.Append(kGoogleTalkPluginFilename)) &&
            base::PathExists(path.Append(kO1DPluginFilename))) {
          *result = path;
          return true;
        }
      }
      return false;
    }

    case FILE_GTALK_PLUGIN: {
      base::FilePath path;
      if (!PathService::Get(DIR_GTALK_PLUGIN, &path)) {
        return false;
      }
      *result = path.Append(kGoogleTalkPluginFilename);
      return true;
    }

    case FILE_O1D_PLUGIN: {
      base::FilePath path;
      if (!PathService::Get(DIR_GTALK_PLUGIN, &path)) {
        return false;
      }
      *result = path.Append(kO1DPluginFilename);
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
