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
      'target_name': 'oxide_qt_private',
      'type': '<(component)',
      'defines': [
        'OXIDE_QT_CORE_IMPLEMENTATION'
      ],
      'dependencies': [
        'Qt5Core',
        'Qt5Gui',
        'Qt5Quick',
        '../../oxide/oxide_common.gyp:oxide_private_generated',
        '../../oxide/oxide_common.gyp:oxide_private_common',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/skia/skia.gyp:skia'
      ],
      'include_dirs': [
        '.',
        '../..',
        '<(DEPTH)'
      ],
      'sources': [
        'browser/oxide_qt_backing_store.cc',
        'browser/oxide_qt_backing_store.h',
        'browser/oxide_qt_content_browser_client.cc',
        'browser/oxide_qt_content_browser_client.h',
        'browser/oxide_qt_message_pump.cc',
        'browser/oxide_qt_message_pump.h',
        'browser/oxide_qt_render_widget_host_view_qquick.cc',
        'browser/oxide_qt_render_widget_host_view_qquick.h',
        'browser/oxide_qt_web_view_host_qquick.cc',
        'browser/oxide_qt_web_view_host_qquick.h',
        'browser/oxide_qt_web_view_host_qquick_delegate.h',
        'common/oxide_qt_content_main_delegate.cc',
        'common/oxide_qt_content_main_delegate.h'
      ]
    },
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
      'target_name': 'Qt5Quick',
      'type': 'none',
      'direct_dependent_settings': {
        'cflags_cc': [
          '<!@(<(pkg_config) --cflags Qt5Quick)'
        ]
      },
      'link_settings': {
        'ldflags': [
          '<!@(<(pkg_config) --libs-only-L --libs-only-other Qt5Quick)',
        ],
        'libraries': [
          '<!@(<(pkg_config) --libs-only-l Qt5Quick)',
        ],
      },
    }
  ]
}
