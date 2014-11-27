// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _OXIDE_MEDIA_PLAYER_OXIDE_H
#define _OXIDE_MEDIA_PLAYER_OXIDE_H

#include <string>

#include "base/callback.h"
#include "base/time/time.h"
#include "url/gurl.h"

namespace oxide {

class BrowserCdm;
class MediaPlayerManager;

class MediaPlayer {
 public:
  virtual ~MediaPlayer();

  // Error types for MediaErrorCB.
  enum MediaErrorType {
    MEDIA_ERROR_FORMAT,
    MEDIA_ERROR_DECODE,
    MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK,
    MEDIA_ERROR_INVALID_CODE,
  };

  // Start playing the media.
  virtual void Start() = 0;

  // Pause the media.
  virtual void Pause(bool is_media_related_action) = 0;

  // Seek to a particular position, based on renderer signaling actual seek
  // with OxdeHostMsg_MediaPlayer_Seek. If eventual success, OnSeekComplete() will be
  // called.
  virtual void SeekTo(base::TimeDelta timestamp) = 0;

  // Release the player resources.
  virtual void Release() = 0;

  // Set the player volume.
  virtual void SetVolume(double volume) = 0;

  // Get the media information from the player.
  virtual int GetVideoWidth() = 0;
  virtual int GetVideoHeight() = 0;
  virtual base::TimeDelta GetDuration() = 0;
  virtual base::TimeDelta GetCurrentTime() = 0;
  virtual bool IsPlaying() = 0;
  virtual bool IsPlayerReady() = 0;
  virtual bool CanPause() = 0;
  virtual bool CanSeekForward() = 0;
  virtual bool CanSeekBackward() = 0;
  virtual GURL GetUrl();
  virtual GURL GetFirstPartyForCookies();

  int player_id() { return player_id_; }

 protected:
  MediaPlayer(int player_id);

 private:
  // Player ID assigned to this player.
  int player_id_;

  DISALLOW_COPY_AND_ASSIGN(MediaPlayer);
};

}  // namespace media

#endif  // _OXIDE_MEDIA_PLAYER_OXIDE_H
