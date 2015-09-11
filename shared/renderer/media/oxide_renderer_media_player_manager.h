// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _OXIDE_RENDERER_MEDIA_PLAYER_MANAGER_H_
#define _OXIDE_RENDERER_MEDIA_PLAYER_MANAGER_H_

#include <map>
#include <string>

#include "base/basictypes.h"
#include "base/time/time.h"
#include "content/public/renderer/render_frame_observer.h"
#include "content/public/renderer/render_frame_observer_tracker.h"
#include "url/gurl.h"
#include "shared/common/oxide_message_enums.h"

namespace blink {
class WebFrame;
}

namespace gfx {
class RectF;
}

namespace oxide {

class WebMediaPlayer;

class RendererMediaPlayerManager :
  public content::RenderFrameObserver,
  public content::RenderFrameObserverTracker<RendererMediaPlayerManager> {
 public:
  explicit RendererMediaPlayerManager(content::RenderFrame* render_frame);
  ~RendererMediaPlayerManager();

  bool OnMessageReceived(const IPC::Message& msg) override;

  void Initialize(OxideHostMsg_MediaPlayer_Initialize_Type type,
                  int player_id,
                  const GURL& url,
                  const GURL& first_party_for_cookies);

  void Start(int player_id);

  // Pauses the player.
  // is_media_related_action should be true if this pause is coming from an
  // an action that explicitly pauses the video (user pressing pause, JS, etc.)
  // Otherwise it should be false if Pause is being called due to other reasons
  // (cleanup, freeing resources, etc.)
  void Pause(int player_id, bool is_media_related_action);

  // Performs seek on the player.
  void Seek(int player_id, const base::TimeDelta& time);

  // Sets the player volume.
  void SetVolume(int player_id, double volume);

  void SetRate(int player_id, double rate);

  // Sets the poster image.
  void SetPoster(int player_id, const GURL& poster);

  // Releases resources for the player.
  void ReleaseResources(int player_id);

  // Destroys the player in the browser process
  void DestroyPlayer(int player_id);

  // Registers and unregisters a WebMediaPlayer object.
  int RegisterMediaPlayer(WebMediaPlayer* player);
  void UnregisterMediaPlayer(int player_id);

  // Gets the pointer to WebMediaPlayer given the |player_id|.
  WebMediaPlayer* GetMediaPlayer(int player_id);

 private:
  // Message handlers.
  void OnMediaMetadataChanged(int player_id,
                              base::TimeDelta duration,
                              int width,
                              int height,
                              bool success);
  void OnMediaPlaybackCompleted(int player_id);
  void OnMediaBufferingUpdate(int player_id, int percent);
  void OnSeekRequest(int player_id, const base::TimeDelta& time_to_seek);
  void OnSeekCompleted(int player_id, const base::TimeDelta& current_time);
  void OnMediaError(int player_id, int error);
  void OnVideoSizeChanged(int player_id, int width, int height);
  void OnTimeUpdate(int player_id, base::TimeDelta current_time);
  void OnMediaPlayerReleased(int player_id);
  void OnPlayerPlay(int player_id);
  void OnPlayerPause(int player_id);
  void OnPauseVideo();

  void ReleaseVideoResources();

  std::map<int, WebMediaPlayer*> media_players_;

  int next_media_player_id_;

  DISALLOW_COPY_AND_ASSIGN(RendererMediaPlayerManager);
};

}  // namespace content

#endif  // _OXIDE_RENDERER_MEDIA_PLAYER_MANAGER_H_
