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
    'clang': 0,
    'host_clang': 0,
    'print_ld_stats%': 0,
    'enable_tcmalloc%': 0,
    'disable_nacl': 1,
    'linux_use_gold_binary': 0,
    'linux_use_gold_flags': 0,
    'use_allocator': 'none',
    'sysroot': '',
    'use_aura': 1,
    'use_gconf': 0,
    'use_gnome_keyring': 0,
    'use_mojo': 0,
    'use_ozone': 1,
    'toolkit_views': 0,
    'toolkit_uses_gtk': 0,
    'ozone_platform': 'oxide',
    'ozone_auto_platforms': 0,
    'remoting': 0,
    'external_ozone_platforms': [
      'oxide',
    ],
    'enable_printing': 0,
    'variables': {
      'conditions': [
        ['target_arch=="arm"', {
          'arm_neon': 0,
          # Only really works correctly on Android, eg WebRtc_GetCPUFeaturesARM
          # is missing on non-Android Linux
          'arm_neon_optional': 0,
        }],
      ],
    },
    'conditions': [
      ['arm_version==7', {
        # Ubuntu-specific?
        'arm_float_abi': 'hard',
      }],
      ['host_arch=="arm"', {
        # This is desparate - we're trying to avoid linker OOM on native ARM
        # builds. This is unnecessary on ARM cross builds, hence the test for
        # "host_arch".
        'remove_webcore_debug_symbols': 1,
      }],
    ],
  },
  'target_defaults': {
    'cflags!': [
      # Should remove this
      '-Werror',
    ],
    'ldflags': [
      '-B<(PRODUCT_DIR)/../../../gold',
    ],
    'ldflags!': [
      # Currently get a bunch of "warning: hidden symbol" warnings from harfbuzz with gold
      '-Wl,--fatal-warnings',
      # Applicable only for BFD linker
      '-Wl,--reduce-memory-overheads',
    ],
    'conditions': [
      ['print_ld_stats==1', {
        'ldflags': [
          '-Wl,--stats',
        ],
      }],
      ['host_arch=="arm"', {
        'ldflags': [
          # Try to work around linker OOM - we only want these on native
          # ARM builds though, hence the test for "host_arch"
          '-Wl,--no-map-whole-files',
          '-Wl,--no-keep-memory',
          '-Wl,--no-keep-files-mapped',
        ],
      }],
    ],
  }
}
