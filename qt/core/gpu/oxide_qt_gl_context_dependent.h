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

#ifndef _OXIDE_QT_CORE_BROWSER_GL_CONTEXT_DEPENDENT_H_
#define _OXIDE_QT_CORE_BROWSER_GL_CONTEXT_DEPENDENT_H_

#include <QtGlobal>

#include "base/macros.h"
#include "ui/gl/gl_implementation.h"

#include "shared/gpu/oxide_gl_context_dependent.h"

QT_BEGIN_NAMESPACE
class QOpenGLContext;
QT_END_NAMESPACE

namespace oxide {
namespace qt {

class GLContextDependent : public oxide::GLContextDependent {
 public:
  static scoped_refptr<GLContextDependent> Create(QOpenGLContext* context);

  gfx::GLImplementation implementation() const { return implementation_; }

 private:
  GLContextDependent(void* handle,
                     gfx::GLImplementation implementation);

  gfx::GLImplementation implementation_;

  DISALLOW_COPY_AND_ASSIGN(GLContextDependent);
};

} // namespace qt
} // namespace oxide

#endif // _OXIDE_QT_CORE_BROWSER_GL_CONTEXT_DEPENDENT_H_
