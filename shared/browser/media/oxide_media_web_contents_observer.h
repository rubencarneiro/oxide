#ifndef _OXIDE_MEDIA_WEB_CONTENTS_OBSERVER_H_
#define _OXIDE_MEDIA_WEB_CONTENTS_OBSERVER_H_

#include "base/compiler_specific.h"
#include "base/containers/scoped_ptr_hash_map.h"
#include "content/common/content_export.h"
#include "content/public/browser/web_contents_observer.h"

namespace content {
class WebContents;
class RenderFrameHost;
}

namespace oxide {

class WebView;
class BrowserMediaPlayerManager;

class MediaWebContentsObserver : public content::WebContentsObserver {
 public:
  explicit MediaWebContentsObserver(WebView* webView, content::WebContents* contents);
  virtual ~MediaWebContentsObserver();

  virtual void RenderFrameDeleted(content::RenderFrameHost* render_frame_host) override;
  virtual bool OnMessageReceived(const IPC::Message& message,
                                 content::RenderFrameHost* render_frame_host) override;

  virtual void WebContentsDestroyed() override;

  // Gets the media player manager associated with |render_frame_host|. Creates
  // a new one if it doesn't exist. The caller doesn't own the returned pointer.
  BrowserMediaPlayerManager* GetMediaPlayerManager(
      content::RenderFrameHost* render_frame_host);

 private:
  typedef base::ScopedPtrHashMap<uintptr_t, BrowserMediaPlayerManager>
      MediaPlayerManagerMap;
  MediaPlayerManagerMap media_player_managers_;

  WebView* webView_;
  
  DISALLOW_COPY_AND_ASSIGN(MediaWebContentsObserver);
};

}  // namespace oxide

#endif  // _OXIDE_MEDIA_WEB_CONTENTS_OBSERVER_H_
