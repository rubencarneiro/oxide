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
    'disable_nacl': 1,
    'linux_dump_symbols': 1,
    'linux_use_gold_binary': 0,
    'linux_use_gold_flags': 0,
    'linux_use_tcmalloc': 0,
    'sysroot': '',
    'toolkit_uses_gtk': 0,
    'use_aura': 1,
    'use_gconf': 0,
    'conditions': [
      ['target_arch=="arm"', {
        # XXX: Make Ubuntu-specific
        'arm_neon': 0,

        'conditions': [
          ['arm_version==7', {
            'arm_float_abi': 'hard',
          }],
        ],
      }],
    ],
  },
  'target_defaults': {
    'cflags!': [
      '-Werror',
    ],
    'ldflags': [
      '-B<(DEPTH)/../../build/gold'
    ],
  }
}
