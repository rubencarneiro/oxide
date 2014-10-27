// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include "shared/gl/oxide_gl_implementation.h"

#include <QGuiApplication>
#include <QString>

#include "base/logging.h"

namespace oxide {

void GetAllowedGLImplementations(std::vector<gfx::GLImplementation>* impls) {
  QString platform = QGuiApplication::platformName();
  if (platform == "xcb") {
    impls->push_back(gfx::kGLImplementationDesktopGL);
    impls->push_back(gfx::kGLImplementationEGLGLES2);
    impls->push_back(gfx::kGLImplementationOSMesaGL);
  } else if (platform.startsWith("ubuntu")) {
    impls->push_back(gfx::kGLImplementationEGLGLES2);
  } else if (platform == "minimal") {
    // None
  } else {
    DLOG(WARNING) << "Unrecognized platform: " << qPrintable(platform);
  }
}

}
