# Copyright (C) 2013 Canonical Ltd.

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

{
  'variables': {
    'oxide': 1,
    # Build is broken without this since
    # https://chromium.googlesource.com/angle/angle.git/+/35e65e921394713f89df8d9bfe243b4abb5e9bd7
    # (hitting unsupported platform error at the top of third_party/angle/src/libANGLE/Display.cpp)
    'angle_enable_gl': 0,
    'clang': 0,
    'host_clang': 0,
    'print_ld_stats%': 0,
    'enable_app_list': 0,
    'enable_tcmalloc%': 0,
    'disable_nacl': 1,
    'ffmpeg_component': 'shared_library',
    'linux_use_bundled_gold': 0,
    'linux_use_bundled_binutils': 0,
    'linux_use_gold_flags': 1,
    'use_allocator': 'none',
    'sysroot': '',
    'use_ash': 0,
    'use_aura': 1,
    'use_cups': 0,
    'use_gconf': 0,
    'use_gnome_keyring': 0,
    'use_libpci': 1,
    'use_ozone': 1,
    'toolkit_views': 0,
    'enable_basic_printing': 0,
    'enable_print_preview': 0,
    'variables': {
      'conditions': [
        ['target_arch=="arm"', {
          'arm_neon': 1,
          'arm_neon_optional': 0,
        }],
      ],
    },
    'conditions': [
      ['arm_version==7', {
        # Ubuntu-specific?
        'arm_float_abi': 'hard',
      }],
      ['(host_arch=="arm" or host_arch=="ia32") and component=="static_library"', {
        # This is desparate - we're trying to avoid linker OOM on native ARM
        # and x86 builds. This is unnecessary on ARM cross builds, hence the
        # test for "host_arch".
        'remove_webcore_debug_symbols': 1,
      }],
    ],
  },
  'target_defaults': {
    'cflags!': [
      # Should remove this
      '-Werror',
    ],
    'ldflags!': [
      # Currently get a bunch of "warning: hidden symbol" warnings from harfbuzz with gold
      '-Wl,--fatal-warnings',
    ],
    'conditions': [
      ['print_ld_stats==1', {
        'ldflags': [
          '-Wl,--stats',
        ],
      }],
      ['(host_arch=="arm" or host_arch=="ia32") and component=="static_library"', {
        'ldflags': [
          # Try to work around linker OOM - we only want these on native
          # ARM and x86 builds though, hence the test for "host_arch"
          '-Wl,--no-keep-files-mapped',
        ],
      }],
    ],
  }
}
