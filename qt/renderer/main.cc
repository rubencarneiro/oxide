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

#include "qt/core/app/oxide_qt_allocator_hooks.h"
#include "qt/core/app/oxide_qt_main.h"
#include "shared/allocator/features.h"

#if BUILDFLAG(USE_UNIFIED_ALLOCATOR_SHIM)
#include "base/allocator/allocator_shim.h" // nogncheck
#endif
#if defined(COMPONENT_BUILD)
#include "content/public/common/content_client.h" // nogncheck
#endif
#if BUILDFLAG(USE_TCMALLOC)
#include "third_party/tcmalloc/chromium/src/gperftools/tcmalloc.h" // nogncheck
#include "third_party/tcmalloc/chromium/src/gperftools/malloc_extension.h" // nogncheck
#endif

class AllocatorHooksImpl : public oxide::qt::AllocatorHooks {
 public:
  void ReleaseFreeMemory() override {
#if defined(USE_TCMALLOC)
    MallocExtension::instance()->ReleaseFreeMemory();
#endif
  }

  void* UncheckedAlloc(size_t size) override {
#if defined(USE_TCMALLOC)
    return tc_malloc_skip_new_handler(size);
#else
    return malloc(size);
#endif
  }

  void EnableTerminationOnOutOfMemory() override {
#if BUILDFLAG(USE_UNIFIED_ALLOCATOR_SHIM)
    base::allocator::SetCallNewHandlerOnMallocFailure(true);
#elif BUILDFLAG(USE_TCMALLOC)
    tc_set_new_mode(1);
#endif
  }
};

int main(int argc, const char* argv[]) {
#if defined(COMPONENT_BUILD)
  // Gross hack for component build
  // see https://code.google.com/p/chromium/issues/detail?id=374712
  content::SetContentClient(nullptr);
#endif

  AllocatorHooksImpl allocator_hooks;

  return oxide::qt::OxideMain(argc, argv, &allocator_hooks);
}
