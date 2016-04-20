
#include "shared/browser/media/oxide_media_web_contents_observer.h"

#include "base/memory/ptr_util.h"
#include "base/stl_util.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "shared/browser/media/oxide_browser_media_player_manager.h"
#include "shared/common/oxide_messages.h"

namespace oxide {

MediaWebContentsObserver::MediaWebContentsObserver(
    content::WebContents* contents)
    : content::WebContentsObserver(contents) {}

MediaWebContentsObserver::~MediaWebContentsObserver() {}

void MediaWebContentsObserver::RenderFrameDeleted(
    content::RenderFrameHost* render_frame_host) {
  uintptr_t key = reinterpret_cast<uintptr_t>(render_frame_host);
  media_player_managers_.erase(key);
}

bool MediaWebContentsObserver::OnMessageReceived(
    const IPC::Message& msg,
    content::RenderFrameHost* render_frame_host) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(MediaWebContentsObserver, msg)
    IPC_MESSAGE_FORWARD(OxideHostMsg_MediaPlayer_Initialize,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnInitialize)
    IPC_MESSAGE_FORWARD(OxideHostMsg_MediaPlayer_Start,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnStart)
    IPC_MESSAGE_FORWARD(OxideHostMsg_MediaPlayer_Seek,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnSeek)
    IPC_MESSAGE_FORWARD(OxideHostMsg_MediaPlayer_Pause,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnPause)
    IPC_MESSAGE_FORWARD(OxideHostMsg_MediaPlayer_SetVolume,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnSetVolume)
    IPC_MESSAGE_FORWARD(OxideHostMsg_MediaPlayer_SetRate,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnSetRate)
    IPC_MESSAGE_FORWARD(OxideHostMsg_MediaPlayer_SetPoster,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnSetPoster)
    IPC_MESSAGE_FORWARD(OxideHostMsg_MediaPlayer_Release,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnReleaseResources)
    IPC_MESSAGE_FORWARD(OxideHostMsg_MediaPlayer_DestroyMediaPlayer,
                        GetMediaPlayerManager(render_frame_host),
                        BrowserMediaPlayerManager::OnDestroyPlayer)
      IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void MediaWebContentsObserver::WebContentsDestroyed() {
  delete this;  
}

BrowserMediaPlayerManager* MediaWebContentsObserver::GetMediaPlayerManager(
    content::RenderFrameHost* render_frame_host) {
  uintptr_t key = reinterpret_cast<uintptr_t>(render_frame_host);
  if (!media_player_managers_.contains(key)) {
    media_player_managers_.set(
        key,
        base::WrapUnique(new BrowserMediaPlayerManager(render_frame_host)));
  }
  return media_player_managers_.get(key);
}

}  // namespace content
