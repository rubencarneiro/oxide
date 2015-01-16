// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_renderer_media_player_manager.h"

#include "shared/common/oxide_messages.h"
#include "content/public/common/renderer_preferences.h"
#include "content/renderer/render_view_impl.h"
#include "ui/gfx/rect_f.h"

#include "oxide_web_media_player.h"

namespace oxide {

RendererMediaPlayerManager::RendererMediaPlayerManager(
    content::RenderFrame* render_frame)
    : RenderFrameObserver(render_frame)
    , RenderFrameObserverTracker<RendererMediaPlayerManager>(render_frame)
    , next_media_player_id_(0) {
}

RendererMediaPlayerManager::~RendererMediaPlayerManager() {
  std::map<int, WebMediaPlayer*>::iterator player_it;
  for (player_it = media_players_.begin();
      player_it != media_players_.end(); ++player_it) {
    WebMediaPlayer* player = player_it->second;
    player->Detach();
  }
}

bool RendererMediaPlayerManager::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(RendererMediaPlayerManager, msg)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_MediaMetadataChanged,
                        OnMediaMetadataChanged)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_MediaPlaybackCompleted,
                        OnMediaPlaybackCompleted)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_MediaBufferingUpdate,
                        OnMediaBufferingUpdate)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_SeekRequest, OnSeekRequest)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_SeekCompleted, OnSeekCompleted)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_MediaError, OnMediaError)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_MediaVideoSizeChanged,
                        OnVideoSizeChanged)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_MediaTimeUpdate, OnTimeUpdate)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_MediaPlayerReleased,
                        OnMediaPlayerReleased)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_DidMediaPlayerPlay, OnPlayerPlay)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_DidMediaPlayerPause, OnPlayerPause)
    IPC_MESSAGE_HANDLER(OxideMsg_MediaPlayer_PauseVideo, OnPauseVideo)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void RendererMediaPlayerManager::Initialize(
    OxideHostMsg_MediaPlayer_Initialize_Type type,
    int player_id,
    const GURL& url,
    const GURL& first_party_for_cookies) {

  OxideHostMsg_MediaPlayer_Initialize_Params params;
  params.type = type;
  params.player_id = player_id;
  params.url = url;
  params.first_party_for_cookies = first_party_for_cookies;

  Send(new OxideHostMsg_MediaPlayer_Initialize(
      routing_id(), params));
}

void RendererMediaPlayerManager::Start(int player_id) {
  Send(new OxideHostMsg_MediaPlayer_Start(routing_id(), player_id));
}

void RendererMediaPlayerManager::Pause(
    int player_id,
    bool is_media_related_action) {
  Send(new OxideHostMsg_MediaPlayer_Pause(
      routing_id(), player_id, is_media_related_action));
}

void RendererMediaPlayerManager::Seek(
    int player_id,
    const base::TimeDelta& time) {
  Send(new OxideHostMsg_MediaPlayer_Seek(routing_id(), player_id, time));
}

void RendererMediaPlayerManager::SetVolume(int player_id, double volume) {
  Send(new OxideHostMsg_MediaPlayer_SetVolume(routing_id(), player_id, volume));
}

void RendererMediaPlayerManager::SetPoster(int player_id, const GURL& poster) {
  Send(new OxideHostMsg_MediaPlayer_SetPoster(routing_id(), player_id, poster));
}

void RendererMediaPlayerManager::ReleaseResources(int player_id) {
  Send(new OxideHostMsg_MediaPlayer_Release(routing_id(), player_id));
}

void RendererMediaPlayerManager::DestroyPlayer(int player_id) {
  Send(new OxideHostMsg_MediaPlayer_DestroyMediaPlayer(routing_id(), player_id));
}

void RendererMediaPlayerManager::OnMediaMetadataChanged(
    int player_id,
    base::TimeDelta duration,
    int width,
    int height,
    bool success) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnMediaMetadataChanged(duration, width, height, success);
  }
}

void RendererMediaPlayerManager::OnMediaPlaybackCompleted(int player_id) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnPlaybackComplete();
  }
}

void RendererMediaPlayerManager::OnMediaBufferingUpdate(int player_id,
                                                        int percent) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnBufferingUpdate(percent);
  }
}

void RendererMediaPlayerManager::OnSeekRequest(
    int player_id,
    const base::TimeDelta& time_to_seek) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnSeekRequest(time_to_seek);
  }
}

void RendererMediaPlayerManager::OnSeekCompleted(
    int player_id,
    const base::TimeDelta& current_time) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnSeekComplete(current_time);
  }
}

void RendererMediaPlayerManager::OnMediaError(int player_id, int error) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnMediaError(error);
  }
}

void RendererMediaPlayerManager::OnVideoSizeChanged(int player_id,
                                                    int width,
                                                    int height) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnVideoSizeChanged(width, height);
  }
}

void RendererMediaPlayerManager::OnTimeUpdate(int player_id,
                                              base::TimeDelta current_time) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnTimeUpdate(current_time);
  }
}

void RendererMediaPlayerManager::OnMediaPlayerReleased(int player_id) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnPlayerReleased();
  }
}

void RendererMediaPlayerManager::OnPlayerPlay(int player_id) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnMediaPlayerPlay();
  }
}

void RendererMediaPlayerManager::OnPlayerPause(int player_id) {
  WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player) {
    player->OnMediaPlayerPause();
  }
}

void RendererMediaPlayerManager::OnPauseVideo() {
  ReleaseVideoResources();
}

int RendererMediaPlayerManager::RegisterMediaPlayer(
    WebMediaPlayer* player) {
  media_players_[next_media_player_id_] = player;
  return next_media_player_id_++;
}

void RendererMediaPlayerManager::UnregisterMediaPlayer(int player_id) {
  media_players_.erase(player_id);
}

void RendererMediaPlayerManager::ReleaseVideoResources() {
  std::map<int, WebMediaPlayer*>::iterator player_it;
  for (player_it = media_players_.begin(); player_it != media_players_.end();
       ++player_it) {
    WebMediaPlayer* player = player_it->second;

    // Do not release if an audio track is still playing
    if (player && (player->paused() || player->hasVideo())) {
      player->ReleaseMediaResources();
    }
  }
}

WebMediaPlayer* RendererMediaPlayerManager::GetMediaPlayer(
    int player_id) {
  std::map<int, WebMediaPlayer*>::iterator iter =
      media_players_.find(player_id);
  if (iter != media_players_.end()) {
    return iter->second;
  }
  return nullptr;
}

}  // namespace content
