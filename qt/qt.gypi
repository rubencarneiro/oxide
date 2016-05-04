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
    'oxide_lib_target': '../qt/core/core.gyp:<(oxide_lib)',
    'oxide_renderer_target': '../qt/renderer/renderer.gyp:<(oxide_renderer)',
    'oxide_platform_test_targets': [
      '../qt/core/core.gyp:oxide_qt_unittests',
    ],
  },
  'target_defaults': {
    'defines': [
      # XXX(chrisccoulson): Rename this to OXIDE_PLATFORM_QT and automatically
      #  add it in build/common.gypi based on oxide_build, or remove if
      #  possible - it's only used in one place
      'OXIDE_BUILD_QT',
    ],
  },
}
