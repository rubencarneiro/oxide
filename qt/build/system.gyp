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
    'pkg_config': 'pkg-config'
  },
  'targets': [
    {
      'target_name': 'Qt5Core',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags_cc': [
          '<!@(<(pkg_config) --cflags Qt5Core)'
        ]
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg_config) --libs-only-L --libs-only-other Qt5Core)',
        ],
        'libraries': [
          '<!@(<(pkg_config) --libs-only-l Qt5Core)',
        ],
      },
    },
    {
      'target_name': 'Qt5Core-private',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags_cc': [
          '-I<!(<(pkg_config) --variable includedir Qt5Core)/QtCore/<!(<(pkg_config) --modversion Qt5Core)',
          '-I<!(<(pkg_config) --variable includedir Qt5Core)/QtCore/<!(<(pkg_config) --modversion Qt5Core)/QtCore',
        ],
      },
    },
    {
      'target_name': 'Qt5Gui',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags_cc': [
          '<!@(<(pkg_config) --cflags Qt5Gui)'
        ]
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg_config) --libs-only-L --libs-only-other Qt5Gui)',
        ],
        'libraries': [
          '<!@(<(pkg_config) --libs-only-l Qt5Gui)',
        ],
      },
    },
    {
      'target_name': 'Qt5Gui-private',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags_cc': [
          '-I<!(<(pkg_config) --variable includedir Qt5Gui)/QtGui/<!(<(pkg_config) --modversion Qt5Gui)',
          '-I<!(<(pkg_config) --variable includedir Qt5Gui)/QtGui/<!(<(pkg_config) --modversion Qt5Gui)/QtGui',
        ],
      },
    },
    {
      'target_name': 'Qt5Network',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags_cc': [
          '<!@(<(pkg_config) --cflags Qt5Network)'
        ]
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg_config) --libs-only-L --libs-only-other Qt5Network)',
        ],
        'libraries': [
          '<!@(<(pkg_config) --libs-only-l Qt5Network)',
        ],
      },
    },
    {
      'target_name': 'Qt5Positioning',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags_cc': [
          '<!@(<(pkg_config) --cflags Qt5Positioning)'
        ]
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg_config) --libs-only-L --libs-only-other Qt5Positioning)',
        ],
        'libraries': [
          '<!@(<(pkg_config) --libs-only-l Qt5Positioning)',
        ],
      },
    },
  ],
}
