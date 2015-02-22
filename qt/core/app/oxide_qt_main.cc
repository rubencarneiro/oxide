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

#include "oxide_qt_main.h"

#include "base/allocator/allocator_extension.h"

#include "shared/app/oxide_main.h"

#include "oxide_qt_platform_delegate.h"

namespace oxide {
namespace qt {

int OxideMain(int argc,
              const char** argv,
              ReleaseFreeMemoryFunction release_free_memory_function) {
  base::allocator::SetReleaseFreeMemoryFunction(release_free_memory_function);

  PlatformDelegate delegate;

  oxide::OxideMainParams params(&delegate);
  params.argc = argc;
  params.argv = argv;

  return oxide::OxideMain(params);
}

} // namespace qt
} // namespace oxide
