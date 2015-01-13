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

#include "oxide_qquick_init.h"

#include <QtGlobal>
#if defined(ENABLE_COMPOSITING)
#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
#include <QtQuick/private/qsgcontext_p.h>
#else
#include <QtGui/private/qopenglcontext_p.h>
#endif
#endif

#include "qt/core/glue/oxide_qt_init.h"

namespace oxide {
namespace qquick {

void EnsureChromiumStarted() {
  static bool started = false;
  if (started) {
    return;
  }
  started = true;
#if defined(ENABLE_COMPOSITING)
  oxide::qt::SetSharedGLContext(
#if QT_VERSION < QT_VERSION_CHECK(5, 3, 0)
      QSGContext::sharedOpenGLContext()
#else
      QOpenGLContextPrivate::globalShareContext()
#endif
  );
#endif
  oxide::qt::EnsureChromiumStarted();
}

} // namespace qquick
} // namespace oxide
