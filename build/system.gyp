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

{
  'variables': {
    'pkg_config': 'pkg-config'
  },
  'targets': [
    {
      'target_name': 'gdkpixbuf',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags_cc': [
          '<!@(<(pkg_config) --cflags gdk-pixbuf-2.0)'
        ]
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg_config) --libs-only-L --libs-only-other gdk-pixbuf-2.0)',
        ],
        'libraries': [
          '<!@(<(pkg_config) --libs-only-l gdk-pixbuf-2.0)',
        ],
      },
    },
    {
      'target_name': 'libnotify',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags_cc': [
          '<!@(<(pkg_config) --cflags libnotify)'
        ]
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg_config) --libs-only-L --libs-only-other libnotify)',
        ],
        'libraries': [
          '<!@(<(pkg_config) --libs-only-l libnotify)',
        ],
      },
    },
  ],
  'conditions': [
    ['enable_hybris==1', {
      'targets': [
        {
          'target_name': 'android-properties',
          'type': 'none',
          'direct_dependent_settings': {
            'cflags_cc': [
              '<!@(<(pkg_config) --cflags libandroid-properties)'
            ]
          },
          'link_settings': {
            'ldflags': [
              '<!@(<(pkg_config) --libs-only-L --libs-only-other libandroid-properties)',
            ],
            'libraries': [
              '<!@(<(pkg_config) --libs-only-l libandroid-properties)',
            ],
          },
        },
      ],
    }],
  ],
}
