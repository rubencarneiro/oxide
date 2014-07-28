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

#ifndef _OXIDE_QT_CORE_GL_SHARED_GL_CONTEXT_H_
#define _OXIDE_QT_CORE_GL_SHARED_GL_CONTEXT_H_

#include <QtGlobal>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "ui/gl/gl_implementation.h"

#include "shared/gl/oxide_shared_gl_context.h"

QT_BEGIN_NAMESPACE
class QOpenGLContext;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class SharedGLContext FINAL : public oxide::SharedGLContext {
 public:
  SharedGLContext(QOpenGLContext* context);

  void* GetHandle() FINAL { return handle_; }
  gfx::GLImplementation GetImplementation() FINAL { return implementation_; }

 private:
  void* handle_;
  gfx::GLImplementation implementation_;
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_GL_SHARED_GL_CONTEXT_H_
