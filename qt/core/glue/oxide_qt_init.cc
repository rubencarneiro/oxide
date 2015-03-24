// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013-2015 Canonical Ltd.

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

#include "oxide_qt_init.h"

#include "qt/core/browser/oxide_qt_browser_startup.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
#include "qt/core/gpu/oxide_qt_gl_context_depdendent.h"
#endif

namespace oxide {
namespace qt {

#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
void SetSharedGLContext(QOpenGLContext* context) {
  scoped_refptr<GLContextDependent> c(GLContextDependent::Create(context));
  BrowserStartup::GetInstance()->SetSharedGLContext(c.get());
}
#endif

void EnsureChromiumStarted() {
  BrowserStartup::GetInstance()->EnsureChromiumStarted();
}

} // namespace qt
} // namespace oxide
