// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_player_media_hub.h"
#include "oxide_browser_media_player_manager.h"

#include "base/basictypes.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/string_split.h"
#include "shared/common/oxide_constants.h"

namespace oxide {

MediaPlayerMediaHub::MediaPlayerMediaHub(
    int player_id,
    const GURL& url,
    const GURL& first_party_for_cookies,
    const std::string& user_agent,
    BrowserMediaPlayerManager* manager)
    : MediaPlayer(player_id),
      prepared_(false),
      pending_play_(false),
      use_fixed_session_(false),
      url_(url),
      first_party_for_cookies_(first_party_for_cookies),
      user_agent_(user_agent),
      manager_(manager),
      width_(0),
      height_(0),
      duration_(0),
      media_hub_client_(0),
      weak_factory_(this) {

  const base::CommandLine& command_line = *base::CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kMediaHubFixedSessionDomains)) {
    std::string ds = command_line.GetSwitchValueASCII(switches::kMediaHubFixedSessionDomains);
    std::vector<std::string> dl =
      base::SplitString(ds, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
    for (std::vector<std::string>::iterator it = dl.begin(); it != dl.end(); ++it) {
      if (first_party_for_cookies_.DomainIs(it->c_str())) {
        use_fixed_session_ = true;
        break;
      }
    }
  }

  if (use_fixed_session_) {
    media_hub_client_ = mediahub_create_fixed_player(player_id, first_party_for_cookies_.host(), this);
  } else {
    media_hub_client_ = mediahub_create_player(player_id, this);
  }
}

MediaPlayerMediaHub::~MediaPlayerMediaHub() {
  timer_.Stop();
  if (!use_fixed_session_) {
    mediahub_stop(media_hub_client_);
  }
  mediahub_release(media_hub_client_);
}

void MediaPlayerMediaHub::Initialize() {
  if (url_.SchemeIsHTTPOrHTTPS()) {
    manager_->GetCookies(url_,
                         first_party_for_cookies_,
                         base::Bind(&MediaPlayerMediaHub::OnCookiesRetrieved,
                                    weak_factory_.GetWeakPtr()));

  } else {
    prepared_ = true;
    mediahub_open_uri(media_hub_client_, url_.spec());
  }
}

void MediaPlayerMediaHub::SetDuration(base::TimeDelta duration) {
}

void MediaPlayerMediaHub::Start() {
  if (prepared_) {
    timer_.Start(FROM_HERE,
                 base::TimeDelta::FromMilliseconds(1000),
                 this,
                 &MediaPlayerMediaHub::CheckStatus);
    mediahub_play(media_hub_client_);
  } else {
    pending_play_ = true;
  }
}

void MediaPlayerMediaHub::Pause(bool is_media_related_action) {
  timer_.Stop();
  mediahub_pause(media_hub_client_);
}

bool MediaPlayerMediaHub::IsPlaying() {
  return mediahub_is_playing(media_hub_client_);
}

int MediaPlayerMediaHub::GetVideoWidth() {
  return 0;
}

int MediaPlayerMediaHub::GetVideoHeight() {
  return 0;
}

void MediaPlayerMediaHub::SeekTo(base::TimeDelta timestamp) {
  mediahub_seek_to(media_hub_client_, timestamp.InMicroseconds());
}

base::TimeDelta MediaPlayerMediaHub::GetCurrentTime() {
  return base::TimeDelta::FromMilliseconds(mediahub_get_position(media_hub_client_));
}

base::TimeDelta MediaPlayerMediaHub::GetDuration() {
  return base::TimeDelta::FromMilliseconds(mediahub_get_duration(media_hub_client_));
}

void MediaPlayerMediaHub::Release() {
}

void MediaPlayerMediaHub::SetVolume(double volume) {
  mediahub_set_volume(media_hub_client_, volume);
}

bool MediaPlayerMediaHub::CanPause() {
  return mediahub_can_pause(media_hub_client_) != 0;
}

bool MediaPlayerMediaHub::CanSeekForward() {
  return mediahub_can_seek_forward(media_hub_client_) != 0;
}

bool MediaPlayerMediaHub::CanSeekBackward() {
  return mediahub_can_seek_backward(media_hub_client_) != 0;
}

bool MediaPlayerMediaHub::IsPlayerReady() {
  return mediahub_is_player_ready(media_hub_client_) != 0;
}

GURL MediaPlayerMediaHub::GetUrl() {
  return url_;
}

GURL MediaPlayerMediaHub::GetFirstPartyForCookies() {
  return first_party_for_cookies_;
}

void MediaPlayerMediaHub::seeked_to(int64_t pos) {
  manager_->OnTimeUpdate(player_id(), base::TimeDelta::FromMilliseconds(pos));
}

void MediaPlayerMediaHub::end_of_stream() {
  manager_->OnPlaybackComplete(player_id());
}

void MediaPlayerMediaHub::playback_status_changed(MediaHubDelegate::Status status) {
  switch (status) {
  case null:
  case ready:
    manager_->OnMediaMetadataChanged(player_id(),
                                      base::TimeDelta::FromMilliseconds(mediahub_get_duration(media_hub_client_)),
                                      0, 0, true);
    break;
  case playing:
    manager_->OnMediaMetadataChanged(player_id(),
                                      base::TimeDelta::FromMilliseconds(mediahub_get_duration(media_hub_client_)),
                                      0, 0, true);
    manager_->OnPlayerPlay(player_id());
    break;
  case paused:
    manager_->OnMediaMetadataChanged(player_id(),
                                      base::TimeDelta::FromMilliseconds(mediahub_get_duration(media_hub_client_)),
                                      0, 0, true);
    manager_->OnPlayerPause(player_id());
    break;
  case stopped:
    break;
  }
}


void MediaPlayerMediaHub::OnCookiesRetrieved(const std::string& cookies) {
  prepared_ = true;
  cookies_ = cookies;

  mediahub_open_uri(media_hub_client_, url_.spec(), cookies_, user_agent_);

  if (pending_play_) {
    Start();
    pending_play_ = false;
  }
}

void MediaPlayerMediaHub::CheckStatus() {
  unsigned long long duration = mediahub_get_duration(media_hub_client_);

  if (duration != duration_) {
    manager_->OnMediaMetadataChanged(player_id(),
                                      base::TimeDelta::FromMilliseconds(duration),
                                      0, 0, true);
    duration_ = duration;
  }

  manager_->OnTimeUpdate(player_id(),
      base::TimeDelta::FromMilliseconds(
          mediahub_get_position(media_hub_client_)));
}

}  // namespace media
