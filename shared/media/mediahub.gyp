# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'mediahub_lib',
      'type': 'static_library',
      'hard_dependency': 1,
      'cflags_cc+': [ '-std=c++11', '-fexceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'link_settings': {
        'libraries': [
          '-lmedia-hub-client',
        ],
      },
      'include_dirs': [
        '..',
      ],
      'sources': [
        'player.cc',
        'player.h',
      ],
    },
  ],
}
