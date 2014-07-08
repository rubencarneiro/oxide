# Copyright (C) 2014 Canonical Ltd.

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
      'target_name': 'oxide_chromedriver_internal_deps',
      'type': 'none',
      'hard_dependency': 1,
      'dependencies': [
        # Needed since there is a dependancy issue in chromium's chrome/ gyp tree
        # and with the repack_locales.py script. The script expects the pak file
        # from libaddressinput to exist but it is only built w/ the browser_test.gypi
        # which we dont depend on (dont need to).
        '<(DEPTH)/third_party/libaddressinput/libaddressinput.gyp:libaddressinput',
      ],
    },
    {
      'target_name': 'oxide_chromedriver',
      'type': 'none',
      'hard_dependency': 1,
      'variables': { 'protoc_out_dir': '<(SHARED_INTERMEDIATE_DIR)/protoc_out', },
      'dependencies': [
        'oxide_chromedriver_internal_deps',
        '<(DEPTH)/chrome/chrome.gyp:chromedriver',
      ],
    },
  ],
}
