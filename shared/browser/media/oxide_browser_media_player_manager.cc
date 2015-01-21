#include "oxide_media_player.h"
#include "oxide_player_media_hub.h"
#include "oxide_browser_media_player_manager.h"
#include "shared/common/oxide_content_client.h"
#include "shared/browser/oxide_web_view.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/common/oxide_messages.h"
#include "content/public/browser/render_frame_host.h"
#include "net/base/net_errors.h"
#include "net/cookies/cookie_store.h"

namespace oxide {

BrowserMediaPlayerManager::BrowserMediaPlayerManager(
    content::RenderFrameHost* rfh)
    : render_frame_host_(rfh) {}

BrowserMediaPlayerManager::~BrowserMediaPlayerManager() {}

void BrowserMediaPlayerManager::OnInitialize(const OxideHostMsg_MediaPlayer_Initialize_Params& media_player_params) {

  RemovePlayer(media_player_params.player_id);

  MediaPlayer* player = CreateMediaPlayer(media_player_params);

  if (!player) {
    return;
  }

  AddPlayer(player);
}

void BrowserMediaPlayerManager::OnStart(int player_id) {
  MediaPlayer* player = GetPlayer(player_id);
  if (!player) {
    return;
  }

  player->Start();
}

void BrowserMediaPlayerManager::OnSeek(
    int player_id,
    const base::TimeDelta& time) {
  MediaPlayer* player = GetPlayer(player_id);

  if (player) {
    player->SeekTo(time);
  }
}

void BrowserMediaPlayerManager::OnPause(
    int player_id,
    bool is_media_related_action) {

  MediaPlayer* player = GetPlayer(player_id);

  if (player) {
    player->Pause(is_media_related_action);
  }
}

void BrowserMediaPlayerManager::OnSetVolume(int player_id, double volume) {
  MediaPlayer* player = GetPlayer(player_id);

  if (player) {
    player->SetVolume(volume);
  }
}

void BrowserMediaPlayerManager::OnSetPoster(int player_id, const GURL& url) {
  // To be overridden by subclasses.
}

void BrowserMediaPlayerManager::OnReleaseResources(int player_id) {
  MediaPlayer* player = GetPlayer(player_id);

  if (player) {
    player->Release();
  }
}

void BrowserMediaPlayerManager::OnDestroyPlayer(int player_id) {
  RemovePlayer(player_id);
}

void BrowserMediaPlayerManager::OnMediaMetadataChanged(
    int player_id, base::TimeDelta duration, int width, int height,
    bool success) {

  Send(new OxideMsg_MediaPlayer_MediaMetadataChanged(
      GetRoutingID(), player_id, duration, width, height, success));
}

void BrowserMediaPlayerManager::OnPlaybackComplete(int player_id) {
  Send(new OxideMsg_MediaPlayer_MediaPlaybackCompleted(GetRoutingID(), player_id));
}

void BrowserMediaPlayerManager::OnPlayerPlay(int player_id) {
  Send(new OxideMsg_MediaPlayer_DidMediaPlayerPlay(GetRoutingID(), player_id));
}

void BrowserMediaPlayerManager::OnPlayerPause(int player_id) {
  Send(new OxideMsg_MediaPlayer_DidMediaPlayerPause(GetRoutingID(), player_id));
}

void BrowserMediaPlayerManager::OnTimeUpdate(int player_id, const base::TimeDelta& current_time) {
  Send(new OxideMsg_MediaPlayer_MediaTimeUpdate(GetRoutingID(), player_id, current_time));
}

void BrowserMediaPlayerManager::GetCookies(
      const GURL& url,
      const GURL& first_party_for_cookies,
      const BrowserMediaPlayerManager::GetCookieCB& callback) {
  WebView* web_view = WebView::FromRenderFrameHost(render_frame_host_);
  if (!web_view) {
    callback.Run("");
    return;
  }

  BrowserContext* browser_context = web_view->GetBrowserContext();
  net::StaticCookiePolicy policy(browser_context->GetCookiePolicy());
  if (policy.CanGetCookies(url, first_party_for_cookies) != net::OK) {
    callback.Run("");
    return;
  }

  scoped_refptr<net::CookieStore> cookie_store = browser_context->GetCookieStore();
  net::CookieOptions cookie_options;

  cookie_options.set_include_httponly();
  cookie_store->GetCookiesWithOptionsAsync(
                  url,
                  cookie_options,
                  callback);
}

// private
MediaPlayer* BrowserMediaPlayerManager::CreateMediaPlayer(
    const OxideHostMsg_MediaPlayer_Initialize_Params& params) {
  WebView* web_view = WebView::FromRenderFrameHost(render_frame_host_);
  if (!web_view) {
    return nullptr;
  }

  switch (params.type) {
    case MEDIA_PLAYER_TYPE_URL: {
      const std::string user_agent = web_view->GetBrowserContext()->GetUserAgent();

      MediaPlayerMediaHub* media_player_bridge =
        new MediaPlayerMediaHub(
                params.player_id,
                params.url,
                params.first_party_for_cookies,
                user_agent,
                this);

      OnMediaMetadataChanged(params.player_id, base::TimeDelta(), 0, 0, false);

      media_player_bridge->Initialize();

      return media_player_bridge;
    }

    case MEDIA_PLAYER_TYPE_MEDIA_SOURCE: {
      NOTIMPLEMENTED();
      return nullptr;
    }
  }

  NOTREACHED();
  return nullptr;
}

MediaPlayer* BrowserMediaPlayerManager::GetPlayer(int player_id) {
  for (ScopedVector<MediaPlayer>::iterator it = players_.begin();
      it != players_.end(); ++it) {
    if ((*it)->player_id() == player_id) {
      return *it;
    }
  }
  return nullptr;
}

void BrowserMediaPlayerManager::AddPlayer(MediaPlayer* player) {
  DCHECK(!GetPlayer(player->player_id()));
  players_.push_back(player);
}

void BrowserMediaPlayerManager::RemovePlayer(int player_id) {
  for (ScopedVector<MediaPlayer>::iterator it = players_.begin();
      it != players_.end(); ++it) {
    MediaPlayer* player = *it;
    if (player->player_id() == player_id) {
      players_.erase(it);
      break;
    }
  }
}

int BrowserMediaPlayerManager::GetRoutingID() {
  return render_frame_host_->GetRoutingID();
}

bool BrowserMediaPlayerManager::Send(IPC::Message* msg) {
  return render_frame_host_->Send(msg);
}

}
