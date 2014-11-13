#include "oxide_media_player_oxide.h"
#include "oxide_player_mediahub.h"
#include "oxide_browser_media_player_manager.h"
#include "shared/common/oxide_content_client.h"
#include "shared/browser/oxide_web_view.h"
#include "shared/browser/oxide_browser_context.h"
#include "shared/common/oxide_messages.h"
#include "content/public/browser/render_frame_host.h"
#include "net/cookies/cookie_store.h"

namespace oxide {

BrowserMediaPlayerManager* BrowserMediaPlayerManager::Create(WebView* webView, content::RenderFrameHost* rfh)
{
  return new BrowserMediaPlayerManager(webView, rfh);
}

BrowserMediaPlayerManager::BrowserMediaPlayerManager(WebView* webView,
    content::RenderFrameHost* rfh):
    web_view_(webView)
    , render_frame_host_(rfh)
//    , web_contents_(WebContents::FromRenderFrameHost(render_frame_host))
//    , weak_ptr_factory_(this)
{
}

BrowserMediaPlayerManager::~BrowserMediaPlayerManager()
{
}

void BrowserMediaPlayerManager::OnInitialize(const MediaPlayerHostMsg_Initialize_Params& media_player_params) {

  RemovePlayer(media_player_params.player_id);

  MediaPlayerOxide* player = CreateMediaPlayer(media_player_params);

  if (!player)
    return;

  AddPlayer(player);
}

void BrowserMediaPlayerManager::OnStart(int player_id) {
  MediaPlayerOxide* player = GetPlayer(player_id);
  if (!player)
    return;

  player->Start();
}

void BrowserMediaPlayerManager::OnSeek(
    int player_id,
    const base::TimeDelta& time) {
  MediaPlayerOxide* player = GetPlayer(player_id);

  if (player)
    player->SeekTo(time);
}

void BrowserMediaPlayerManager::OnPause(
    int player_id,
    bool is_media_related_action) {

  MediaPlayerOxide* player = GetPlayer(player_id);

  if (player)
    player->Pause(is_media_related_action);
}

void BrowserMediaPlayerManager::OnSetVolume(int player_id, double volume) {
  MediaPlayerOxide* player = GetPlayer(player_id);

  if (player)
    player->SetVolume(volume);
}

void BrowserMediaPlayerManager::OnSetPoster(int player_id, const GURL& url) {
  // To be overridden by subclasses.
}

void BrowserMediaPlayerManager::OnReleaseResources(int player_id) {
  MediaPlayerOxide* player = GetPlayer(player_id);

  if (player)
    player->Release();
}

void BrowserMediaPlayerManager::OnDestroyPlayer(int player_id) {
  RemovePlayer(player_id);
}

void BrowserMediaPlayerManager::OnMediaMetadataChanged(
    int player_id, base::TimeDelta duration, int width, int height,
    bool success) {

  Send(new MediaPlayerMsg_MediaMetadataChanged(
      RoutingID(), player_id, duration, width, height, success));
}

void BrowserMediaPlayerManager::OnPlaybackComplete(int player_id) {
  Send(new MediaPlayerMsg_MediaPlaybackCompleted(RoutingID(), player_id));
}

void BrowserMediaPlayerManager::OnPlayerPlay(int player_id) {
  Send(new MediaPlayerMsg_DidMediaPlayerPlay(RoutingID(), player_id));
}

void BrowserMediaPlayerManager::OnPlayerPause(int player_id) {
  Send(new MediaPlayerMsg_DidMediaPlayerPause(RoutingID(), player_id));
}

void BrowserMediaPlayerManager::GetCookies(
      const GURL& url,
      const GURL& first_party_for_cookies,
      const BrowserMediaPlayerManager::GetCookieCB& callback) {

  scoped_refptr<net::CookieStore> cookie_store = web_view_->GetBrowserContext()->GetCookieStore();

  if (cookie_store.get() != 0) {
    net::CookieOptions cookie_options;
    cookie_options.set_include_httponly();

    cookie_store->GetCookiesWithOptionsAsync(
                    url,
                    cookie_options,
                    callback);
    } else {
      callback.Run("");
    }
}

// private
MediaPlayerOxide* BrowserMediaPlayerManager::CreateMediaPlayer(
    const MediaPlayerHostMsg_Initialize_Params& params) {

  switch (params.type) {
    case MEDIA_PLAYER_TYPE_URL: {
      const std::string user_agent = web_view_->GetBrowserContext()->GetUserAgent();

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
      return NULL;
    }
  }

  NOTREACHED();
  return NULL;
}

MediaPlayerOxide* BrowserMediaPlayerManager::GetPlayer(int player_id) {
  for (ScopedVector<MediaPlayerOxide>::iterator it = players_.begin();
      it != players_.end(); ++it) {
    if ((*it)->player_id() == player_id)
      return *it;
  }
  return NULL;
}

void BrowserMediaPlayerManager::AddPlayer(MediaPlayerOxide* player) {
  DCHECK(!GetPlayer(player->player_id()));
  players_.push_back(player);
}

void BrowserMediaPlayerManager::RemovePlayer(int player_id) {
  for (ScopedVector<MediaPlayerOxide>::iterator it = players_.begin();
      it != players_.end(); ++it) {
    MediaPlayerOxide* player = *it;
    if (player->player_id() == player_id) {
      players_.erase(it);
      break;
    }
  }
}

int BrowserMediaPlayerManager::RoutingID() {
  return render_frame_host_->GetRoutingID();
}

bool BrowserMediaPlayerManager::Send(IPC::Message* msg) {
  return render_frame_host_->Send(msg);
}

}
