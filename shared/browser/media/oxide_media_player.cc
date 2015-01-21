// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_media_player.h"

#include "base/logging.h"

namespace oxide {

MediaPlayer::MediaPlayer(
    int player_id)
  :  player_id_(player_id) {
}

MediaPlayer::~MediaPlayer() {}

GURL MediaPlayer::GetUrl() {
  return GURL();
}

GURL MediaPlayer::GetFirstPartyForCookies() {
  return GURL();
}

}  // namespace media
