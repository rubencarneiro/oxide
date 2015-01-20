// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _OXIDE_MEDIA_PLAYER_MEDIAHUB_H_
#define _OXIDE_MEDIA_PLAYER_MEDIAHUB_H_

#include <map>
#include <string>

#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "oxide_media_player.h"
#include "shared/browser/media/mediahub_player_shim.h"
#include "url/gurl.h"

namespace core {
namespace ubuntu {
namespace media {
class Player;
}
}
}

namespace oxide {

class BrowserMediaPlayerManager;


class MediaPlayerMediaHub : public MediaPlayer,
  public MediaHubDelegate {
 public:
  MediaPlayerMediaHub(int player_id,
                      const GURL& url,
                      const GURL& first_party_for_cookies,
                      const std::string& user_agent,
                      oxide::BrowserMediaPlayerManager* manager);
  ~MediaPlayerMediaHub();

  // MediaPlayerOxide implementation.
  void Initialize();

  void Start() override;
  void Pause(bool is_media_related_action) override;
  void SeekTo(base::TimeDelta timestamp) override;
  void Release() override;
  void SetVolume(double volume) override;
  int GetVideoWidth() override;
  int GetVideoHeight() override;
  base::TimeDelta GetCurrentTime() override;
  base::TimeDelta GetDuration() override;
  bool IsPlaying() override;
  bool CanPause() override;
  bool CanSeekForward() override;
  bool CanSeekBackward() override;
  bool IsPlayerReady() override;
  GURL GetUrl() override;
  GURL GetFirstPartyForCookies() override;

  // MediaHubDelegate
  void seeked_to(int64_t pos);
  void end_of_stream();
  void playback_status_changed(MediaHubDelegate::Status status, int64_t duration);

 protected:
  void SetDuration(base::TimeDelta time);

 private:
  void OnCookiesRetrieved(const std::string& cookies);
  void CheckStatus();

  // Whether the player is prepared for playback.
  bool prepared_;

  // Pending play event while player is preparing.
  bool pending_play_;

  bool use_fixed_session_;

  // Pending seek time while player is preparing.
  base::TimeDelta pending_seek_;

  // Url for playback.
  GURL url_;
  GURL first_party_for_cookies_;
  // User agent string to be used for media player.
  const std::string user_agent_;
  BrowserMediaPlayerManager* manager_;

  int width_;
  int height_;
  unsigned long long duration_;

  // Cookies for |url_|.
  std::string cookies_;

  MediaHubClientHandle media_hub_client_;

  base::WeakPtrFactory<MediaPlayerMediaHub> weak_factory_;

  base::RepeatingTimer<MediaPlayerMediaHub> timer_;

  DISALLOW_COPY_AND_ASSIGN(MediaPlayerMediaHub);
};


} // oxide

#endif //  _OXIDE_MEDIA_PLAYER_MEDIAHUB_H_
