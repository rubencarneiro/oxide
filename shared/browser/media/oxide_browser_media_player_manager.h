// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014 Canonical Ltd.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _OXIDE_BROWSER_MEDIA_PLAYER_MANAGER_H
#define _OXIDE_BROWSER_MEDIA_PLAYER_MANAGER_H

#include "base/memory/scoped_vector.h"
#include "ipc/ipc_message.h"


namespace base {
class TimeDelta;
}

namespace content {
class RenderFrameHost;
}

struct MediaPlayerHostMsg_Initialize_Params;

namespace oxide {

class WebView;
class MediaPlayerOxide;

class BrowserMediaPlayerManager {
 public:
  typedef base::Callback<void(const std::string&)> GetCookieCB;

  static BrowserMediaPlayerManager* Create(WebView* webView, content::RenderFrameHost* rfh);

  BrowserMediaPlayerManager(WebView* webView, content::RenderFrameHost* rfh);
  ~BrowserMediaPlayerManager();

  void OnInitialize(const MediaPlayerHostMsg_Initialize_Params& media_player_params);
  void OnStart(int player_id);
  void OnSeek(int player_id, const base::TimeDelta& pos);
  void OnPause(int player_id, bool is_media_related_action);
  void OnSetVolume(int player_id, double volume);
  void OnSetPoster(int player_id, const GURL& poster);
  void OnReleaseResources(int player_id);
  void OnDestroyPlayer(int player_id);

  void OnMediaMetadataChanged(
      int player_id,
      base::TimeDelta duration,
      int width,
      int height,
      bool success);
  void OnPlaybackComplete(int player_id);
  void OnPlayerPlay(int player_id);
  void OnPlayerPause(int player_id);
  void OnTimeUpdate(int player_id, const base::TimeDelta& current_time);

  void GetCookies(
      const GURL& url,
      const GURL& first_party_for_cookies,
      const GetCookieCB& callback);

 private:
  MediaPlayerOxide* CreateMediaPlayer(
      const MediaPlayerHostMsg_Initialize_Params& params);
  MediaPlayerOxide* GetPlayer(int player_id);
  void AddPlayer(MediaPlayerOxide *player);
  void RemovePlayer(int player_id);

  int RoutingID();
  bool Send(IPC::Message* msg);

  WebView* web_view_;
  content::RenderFrameHost* const render_frame_host_;

  ScopedVector<MediaPlayerOxide> players_;

//  base::WeakPtrFactory<BrowserMediaPlayerManager> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(BrowserMediaPlayerManager);
};

}

#endif // _OXIDE_BROWSER_MEDIA_PLAYER_MANAGER_H
