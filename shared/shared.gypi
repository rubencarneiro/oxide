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
  'target_defaults': {
    'target_conditions': [
      ['_target_name=="content_browser"', {
        'sources/': [
          ['exclude', 'render_widget_host_view_aura\\.cc'],
          ['exclude', 'touch_editable_impl_aura\\.cc'],
          ['exclude', 'web_contents_view_aura\\.cc'],
        ],
      }],
      ['_target_name=="gl"', {
        'sources/': [
          ['exclude', 'gl_context_ozone\\.cc'],
          ['exclude', 'gl_implementation_ozone\\.cc'],
          ['exclude', 'gl_surface_ozone\\.cc'],
        ],
      }],
    ]
  }
}
