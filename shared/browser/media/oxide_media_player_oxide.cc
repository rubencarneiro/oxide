// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_media_player_oxide.h"

#include "base/logging.h"

namespace oxide {

MediaPlayerOxide::MediaPlayerOxide(
    int player_id)
  :  player_id_(player_id) {
}

MediaPlayerOxide::~MediaPlayerOxide() {}

GURL MediaPlayerOxide::GetUrl() {
  return GURL();
}

GURL MediaPlayerOxide::GetFirstPartyForCookies() {
  return GURL();
}

}  // namespace media
