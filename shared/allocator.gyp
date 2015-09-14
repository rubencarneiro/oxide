# Copyright (C) 2015 Canonical Ltd.

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

# This file is based on base/allocator/allocator.gyp:

# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Note, all of the !linux bits have been removed

{
  'target_defaults': {
    'variables': {
      # This code gets run a lot and debugged rarely, so it should be fast
      # by default. See http://crbug.com/388949.
      'debug_optimize': '2',
    },
  },
  'variables': {
    'tcmalloc_dir': '../third_party/chromium/src/third_party/tcmalloc/chromium',
  },
  'targets': [
    # Only executables and not libraries should depend on the
    # allocator target; only the application (the final executable)
    # knows what allocator makes sense.
    {
      'target_name': 'allocator',
      'type': 'static_library',
      # Make sure the allocation library is optimized to
      # the hilt in official builds.
      'variables': {
        'optimize': 'max',
      },
      'dependencies': [
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
      ],
      'configurations': {
        'Debug_Base': {
          'variables': {
            # Provide a way to force disable debugallocation in Debug builds,
            # e.g. for profiling (it's more rare to profile Debug builds,
            # but people sometimes need to do that).
            'disable_debugallocation%': 0,
          },
          'conditions': [
            ['disable_debugallocation==0', {
              'defines': [
                # Use debugallocation for Debug builds to catch problems early
                # and cleanly, http://crbug.com/30715 .
                'TCMALLOC_FOR_DEBUGALLOCATION',
              ],
            }],
          ],
        },
      },
      # Disable the heap checker in tcmalloc.
      'defines': [
        'NO_HEAP_CHECK',
      ],
      'include_dirs': [
        '.',
        '<(tcmalloc_dir)/src/base',
        '<(tcmalloc_dir)/src',
        '<(DEPTH)',
      ],
      'sources': [
        # Generated for our configuration from tcmalloc's build
        # and checked in.
        '<(tcmalloc_dir)/src/config.h',
        '<(tcmalloc_dir)/src/config_linux.h',

        # all tcmalloc native and forked files
        '<(tcmalloc_dir)/src/base/abort.cc',
        '<(tcmalloc_dir)/src/base/abort.h',
        '<(tcmalloc_dir)/src/base/arm_instruction_set_select.h',
        '<(tcmalloc_dir)/src/base/atomicops-internals-arm-generic.h',
        '<(tcmalloc_dir)/src/base/atomicops-internals-arm-v6plus.h',
        '<(tcmalloc_dir)/src/base/atomicops-internals-x86.cc',
        '<(tcmalloc_dir)/src/base/atomicops-internals-x86.h',
        # We don't list dynamic_annotations.c since its copy is already
        # present in the dynamic_annotations target.
        '<(tcmalloc_dir)/src/base/dynamic_annotations.h',
        '<(tcmalloc_dir)/src/base/elf_mem_image.cc',
        '<(tcmalloc_dir)/src/base/linuxthreads.cc',
        '<(tcmalloc_dir)/src/base/linuxthreads.h',
        '<(tcmalloc_dir)/src/base/logging.cc',
        '<(tcmalloc_dir)/src/base/logging.h',
        '<(tcmalloc_dir)/src/base/low_level_alloc.cc',
        '<(tcmalloc_dir)/src/base/low_level_alloc.h',
        '<(tcmalloc_dir)/src/base/spinlock.cc',
        '<(tcmalloc_dir)/src/base/spinlock.h',
        '<(tcmalloc_dir)/src/base/spinlock_internal.cc',
        '<(tcmalloc_dir)/src/base/spinlock_internal.h',
        '<(tcmalloc_dir)/src/base/synchronization_profiling.h',
        '<(tcmalloc_dir)/src/base/sysinfo.cc',
        '<(tcmalloc_dir)/src/base/sysinfo.h',
        '<(tcmalloc_dir)/src/base/vdso_support.cc',
        '<(tcmalloc_dir)/src/base/vdso_support.h',
        '<(tcmalloc_dir)/src/central_freelist.cc',
        '<(tcmalloc_dir)/src/central_freelist.h',
        '<(tcmalloc_dir)/src/common.cc',
        '<(tcmalloc_dir)/src/common.h',
        '<(tcmalloc_dir)/src/free_list.cc',
        '<(tcmalloc_dir)/src/free_list.h',
        '<(tcmalloc_dir)/src/heap-profile-table.cc',
        '<(tcmalloc_dir)/src/heap-profile-table.h',
        '<(tcmalloc_dir)/src/heap-profiler.cc',
        '<(tcmalloc_dir)/src/internal_logging.cc',
        '<(tcmalloc_dir)/src/internal_logging.h',
        '<(tcmalloc_dir)/src/linked_list.h',
        '<(tcmalloc_dir)/src/malloc_extension.cc',
        '<(tcmalloc_dir)/src/malloc_hook-inl.h',
        '<(tcmalloc_dir)/src/malloc_hook.cc',
        '<(tcmalloc_dir)/src/maybe_threads.cc',
        '<(tcmalloc_dir)/src/maybe_threads.h',
        '<(tcmalloc_dir)/src/memory_region_map.cc',
        '<(tcmalloc_dir)/src/memory_region_map.h',
        '<(tcmalloc_dir)/src/page_heap.cc',
        '<(tcmalloc_dir)/src/page_heap.h',
        '<(tcmalloc_dir)/src/raw_printer.cc',
        '<(tcmalloc_dir)/src/raw_printer.h',
        '<(tcmalloc_dir)/src/sampler.cc',
        '<(tcmalloc_dir)/src/sampler.h',
        '<(tcmalloc_dir)/src/span.cc',
        '<(tcmalloc_dir)/src/span.h',
        '<(tcmalloc_dir)/src/stack_trace_table.cc',
        '<(tcmalloc_dir)/src/stack_trace_table.h',
        '<(tcmalloc_dir)/src/stacktrace.cc',
        '<(tcmalloc_dir)/src/static_vars.cc',
        '<(tcmalloc_dir)/src/static_vars.h',
        '<(tcmalloc_dir)/src/symbolize.cc',
        '<(tcmalloc_dir)/src/symbolize.h',
        '<(tcmalloc_dir)/src/system-alloc.cc',
        '<(tcmalloc_dir)/src/thread_cache.cc',
        '<(tcmalloc_dir)/src/thread_cache.h',

        '<(DEPTH)/base/allocator/debugallocation_shim.cc',
      ],
      # We enable all warnings by default, but upstream disables a few.
      # Keep "-Wno-*" flags in sync with upstream by comparing against:
      # http://code.google.com/p/google-perftools/source/browse/trunk/Makefile.am
      'cflags': [
        '-Wno-sign-compare',
        '-Wno-unused-result',
      ],
      'cflags!': [
        '-fvisibility=hidden',
      ],
      'link_settings': {
        'ldflags': [
          # Don't let linker rip this symbol out, otherwise the heap&cpu
          # profilers will not initialize properly on startup.
          '-Wl,-uIsHeapProfilerRunning,-uProfilerStart',
          # Do the same for heap leak checker.
          '-Wl,-u_Z21InitialMallocHook_NewPKvj,-u_Z22InitialMallocHook_MMapPKvS0_jiiix,-u_Z22InitialMallocHook_SbrkPKvi',
          '-Wl,-u_Z21InitialMallocHook_NewPKvm,-u_Z22InitialMallocHook_MMapPKvS0_miiil,-u_Z22InitialMallocHook_SbrkPKvl',
          '-Wl,-u_ZN15HeapLeakChecker12IgnoreObjectEPKv,-u_ZN15HeapLeakChecker14UnIgnoreObjectEPKv',
      ]},
    },
  ],
}
