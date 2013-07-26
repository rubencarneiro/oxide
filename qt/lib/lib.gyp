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
  'targets': [
    {
      'target_name': 'oxide_qt',
      'type': 'static_library',
      'all_dependent_settings': {
        'include_dirs': [
          '../..',
          '<(DEPTH)'
        ]
      },
      'dependencies': [
        '../system.gyp:Qt5Core',
        '../system.gyp:Qt5Gui',
        '../system.gyp:Qt5Quick',
        '../../shared/shared.gyp:oxide_shared_generated',
        '../../shared/shared.gyp:oxide_shared',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/skia/skia.gyp:skia'
      ],
      'export_dependent_settings': [
        '<(DEPTH)/skia/skia.gyp:skia'
      ],
      'include_dirs': [
        '../..',
        '<(DEPTH)'
      ],
      'sources': [
        '<(INTERMEDIATE_DIR)/moc_oxide_qquick_web_view.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_qquick_web_view_context.cc',
        'browser/oxide_qquick_web_view.cc',
        'browser/oxide_qquick_web_view.h',
        'browser/oxide_qquick_web_view_context.cc',
        'browser/oxide_qquick_web_view_context.h',
        'browser/oxide_qt_backing_store.cc',
        'browser/oxide_qt_backing_store.h',
        'browser/oxide_qt_content_browser_client.cc',
        'browser/oxide_qt_content_browser_client.h',
        'browser/oxide_qt_message_pump.cc',
        'browser/oxide_qt_message_pump.h',
        'browser/oxide_qt_render_widget_host_view_qquick.cc',
        'browser/oxide_qt_render_widget_host_view_qquick.h',
        'common/oxide_qt_content_main_delegate.cc',
        'common/oxide_qt_content_main_delegate.h'
      ],
      'variables': {
        'chromium_code': 1
      },
      'actions': [
        {
          'action_name': 'moc_oxide_qquick_web_view.cc',
          'moc_input': 'browser/oxide_qquick_web_view.h',
          'inputs': [
            '<(_moc_input)'
          ],
          'outputs': [
            '<(INTERMEDIATE_DIR)/<(_action_name)'
          ],
          'action': [
            'moc',
            '-o',
            '<(INTERMEDIATE_DIR)/<(_action_name)',
            '<(_moc_input)'
          ]
        },
        {
          'action_name': 'moc_oxide_qquick_web_view_context.cc',
          'moc_input': 'browser/oxide_qquick_web_view_context.h',
          'inputs': [
            '<(_moc_input)'
          ],
          'outputs': [
            '<(INTERMEDIATE_DIR)/<(_action_name)'
          ],
          'action': [
            'moc',
            '-o',
            '<(INTERMEDIATE_DIR)/<(_action_name)',
            '<(_moc_input)'
          ]
        }
      ]
    }
  ]
}
