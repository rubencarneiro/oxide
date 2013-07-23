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
      'target_name': 'oxideqmlplugin',
      'type': 'shared_library',
      'dependencies': [
        '../system.gyp:Qt5Core',
        '../system.gyp:Qt5Gui',
        '../system.gyp:Qt5Quick',
        '../../oxide.gyp:oxideprivate'
      ],
      'include_dirs': [
        '.',
        '<(INTERMEDIATE_DIR)'
      ],
      'sources': [
        '<(INTERMEDIATE_DIR)/moc_oxide_qquick_web_view.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_qquick_web_view_context.cc',
        'oxide_qml_plugin.cc',
        'oxide_qquick_web_view.cc',
        'oxide_qquick_web_view.h',
        'oxide_qquick_web_view_context.cc',
        'oxide_qquick_web_view_context.h'
      ],
      'actions': [
        {
          'action_name': 'moc_oxide_qquick_web_view.cc',
          'moc_input': 'oxide_qquick_web_view.h',
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
          'moc_input': 'oxide_qquick_web_view_context.h',
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
          'action_name': 'oxide_qml_plugin.moc',
          'moc_input': 'oxide_qml_plugin.cc',
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
