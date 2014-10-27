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
    'oxide_subprocess_path': '<(oxide_subprocess_dir)/<(oxide_renderer_name)',
    'oxide_all_targets': [
      '../qt/core/core.gyp:<(oxide_core_name)',
      '../qt/renderer/renderer.gyp:<(oxide_renderer_name)',
    ],
  },
  'target_defaults': {
    'defines': [
      'OXIDE_BUILD=qt'
    ],
    'target_conditions': [
      ['_target_name=="content_browser"', {
        'sources/': [
          ['exclude', 'native_web_keyboard_event_aura\\.cc'],
        ],
      }],
    ],
  },
}
