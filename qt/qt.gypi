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
    'oxide_port_extra_targets': [
      'qt/qmlplugin/qmlplugin.gyp:*',
    ],
    'oxide_port_libname': 'oxide-qt',
    'oxide_port_libversion': '0',
    'oxide_port_renderer_target': 'qt/renderer/renderer.gyp:oxide-renderer',
    'oxide_port_resource_subpath': 'oxide-qt',
    'oxide_port_staticlib_target': 'qt/lib/lib.gyp:oxide_qt'
  },
  'target_defaults': {
    'defines': [
      'OXIDE_BUILD=qt'
    ]
  }
}
