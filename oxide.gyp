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
      'target_name': 'oxide',
      'type': 'none',
      'dependencies': [
        '<(oxide_port_libname)',
        'oxide_packed_resources',
        '<(oxide_port_renderer_target)',
        '<(DEPTH)/sandbox/sandbox.gyp:chrome_sandbox'
      ]
    },
    {
      'target_name': '<(oxide_port_libname)',
      'type': 'shared_library',
      'shared_library_version': '<(oxide_port_libversion)',
      'dependencies': [
        '<(oxide_port_staticlib_target)'
      ],
      'export_dependent_settings': [
        '<@(_dependencies)'
      ]
    },
    {
      'target_name': 'oxide_packed_resources',
      'type': 'none',
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py'
      },
      'dependencies': [
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/ui/ui.gyp:ui_resources'
      ],
      'actions': [
        {
          'action_name': 'repack_oxide',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/oxide/oxide_resources.pak'
            ]
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)'
          ],
          'outputs': [
            '<(PRODUCT_DIR)/oxide.pak'
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)']
        },
        {
          'action_name': 'repack_oxide_100_percent',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_100_percent.pak'
            ]
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)'
          ],
          'outputs': [
            '<(PRODUCT_DIR)/oxide_100_percent.pak'
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)']
        }
      ]
    }
  ]
}
