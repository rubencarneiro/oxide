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
      'target_name': 'qmloxideplugin',
      'type': 'shared_library',
      'dependencies': [
        '../system.gyp:Qt5Core',
        '../system.gyp:Qt5Quick',
        '../../oxide.gyp:<(oxide_port_libname)'
      ],
      'include_dirs': [
        '<(INTERMEDIATE_DIR)'
      ],
      'sources': [
        'oxide_qml_plugin.cc'
      ],
      'actions': [
        {
          'action_name': 'oxide_qml_plugin.moc',
          'moc_input': 'oxide_qml_plugin.cc',
          'includes': [ '../moc.gypi' ]
        }
      ]
    }
  ]
}
