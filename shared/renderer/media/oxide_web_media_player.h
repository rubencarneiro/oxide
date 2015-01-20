// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _OXIDE_RENDERER_MEDIA_WEBMEDIAPLAYER_OXIDE_H_
#define _OXIDE_RENDERER_MEDIA_WEBMEDIAPLAYER_OXIDE_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/time/time.h"
#include "base/threading/thread_checker.h"
#include "content/public/renderer/render_frame_observer.h"
#include "media/cdm/proxy_decryptor.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "media/base/demuxer_stream.h"
#include "media/base/media_keys.h"
#include "third_party/WebKit/public/platform/WebGraphicsContext3D.h"
#include "third_party/WebKit/public/platform/WebMediaPlayer.h"
#include "third_party/WebKit/public/platform/WebSize.h"
#include "third_party/WebKit/public/platform/WebURL.h"
#include "ui/gfx/geometry/rect_f.h"
#include "shared/common/oxide_message_enums.h"

#include "oxide_media_info_loader.h"

namespace base {
class MessageLoopProxy;
}

namespace blink {
class WebContentDecryptionModule;
class WebFrame;
class WebURL;
class WebMediaPlayerClient;
}

namespace gpu {
struct MailboxHolder;
}

namespace media {
class MediaLog;
class WebMediaPlayerDelegate;
}

namespace oxide {
class RendererMediaPlayerManager;
class WebContentDecryptionModuleImpl;

class WebMediaPlayer : public blink::WebMediaPlayer,
                       public content::RenderFrameObserver {
 public:
  WebMediaPlayer(blink::WebFrame* frame,
                        blink::WebMediaPlayerClient* client,
                        base::WeakPtr<media::WebMediaPlayerDelegate> delegate,
                        RendererMediaPlayerManager* player_manager,
                        media::MediaLog* media_log);
  ~WebMediaPlayer();

  // blink::WebMediaPlayer implementation.
  void enterFullscreen();
  void exitFullscreen();
  bool canEnterFullscreen() const;

  // Resource loading.
  void load(LoadType load_type,
                    const blink::WebURL& url,
                    CORSMode cors_mode);

  // Playback controls.
  void play();
  void pause();
  void seek(double seconds);
  bool supportsSave() const;
  void setRate(double rate);
  void setVolume(double volume);
  blink::WebTimeRanges buffered() const;
  blink::WebTimeRanges seekable() const;
  double maxTimeSeekable() const;

  // Poster image, as defined in the <video> element.
  void setPoster(const blink::WebURL& poster) override;

  // Methods for painting.
  void paint(blink::WebCanvas* canvas,
                     const blink::WebRect& rect,
                     unsigned char alpha);

  bool copyVideoTextureToPlatformTexture(
      blink::WebGraphicsContext3D* web_graphics_context,
      unsigned int texture,
      unsigned int level,
      unsigned int internal_format,
      unsigned int type,
      bool premultiply_alpha,
      bool flip_y);

  // True if the loaded media has a playable video/audio track.
  bool hasVideo() const;
  bool hasAudio() const;

  // Dimensions of the video.
  blink::WebSize naturalSize() const;

  // Getters of playback state.
  bool paused() const;
  bool seeking() const;
  double duration() const;
  double timelineOffset() const;
  double currentTime() const;

  bool didLoadingProgress();

  // Internal states of loading and network.
  blink::WebMediaPlayer::NetworkState networkState() const;
  blink::WebMediaPlayer::ReadyState readyState() const;

  bool hasSingleSecurityOrigin() const;
  bool didPassCORSAccessCheck() const;

  double mediaTimeForTimeValue(double timeValue) const;

  // Provide statistics.
  unsigned decodedFrameCount() const;
  unsigned droppedFrameCount() const;
  unsigned audioDecodedByteCount() const;
  unsigned videoDecodedByteCount() const;

  void paint(blink::WebCanvas*, const blink::WebRect&, unsigned char alpha, SkXfermode::Mode);
    //
  // Media player callback handlers.
  void OnMediaMetadataChanged(const base::TimeDelta& duration, int width,
                              int height, bool success);
  void OnPlaybackComplete();
  void OnBufferingUpdate(int percentage);
  void OnSeekRequest(const base::TimeDelta& time_to_seek);
  void OnSeekComplete(const base::TimeDelta& current_time);
  void OnMediaError(int error_type);
  void OnVideoSizeChanged(int width, int height);
  void OnDurationChanged(const base::TimeDelta& duration);

  // Called to update the current time.
  void OnTimeUpdate(const base::TimeDelta& current_time);

  // Functions called when media player status changes.
  void OnDidEnterFullscreen();
  void OnDidExitFullscreen();
  void OnMediaPlayerPlay();
  void OnMediaPlayerPause();
  void OnRequestFullscreen();

  // Called when the player is released.
  void OnPlayerReleased();

  // This function is called by the RendererMediaPlayerManager to pause the
  // video and release the media player and surface texture when we switch tabs.
  // However, the actual GlTexture is not released to keep the video screenshot.
  void ReleaseMediaResources();

  // RenderFrameObserver implementation.
  void OnDestruct() override;

  // Detach the player from its manager.
  void Detach();

  MediaKeyException generateKeyRequest(
      const blink::WebString& key_system,
      const unsigned char* init_data,
      unsigned init_data_length);
  MediaKeyException addKey(
      const blink::WebString& key_system,
      const unsigned char* key,
      unsigned key_length,
      const unsigned char* init_data,
      unsigned init_data_length,
      const blink::WebString& session_id);
  MediaKeyException cancelKeyRequest(
      const blink::WebString& key_system,
      const blink::WebString& session_id);
  void setContentDecryptionModule(
      blink::WebContentDecryptionModule* cdm);

  void OnKeyAdded(const std::string& session_id);
  void OnKeyError(const std::string& session_id,
                  media::MediaKeys::KeyError error_code,
                  uint32 system_code);
  void OnKeyMessage(const std::string& session_id,
                    const std::vector<uint8>& message,
                    const GURL& destination_url);

  void OnMediaSourceOpened(blink::WebMediaSource* web_media_source);

  void OnNeedKey(const std::string& type,
                 const std::vector<uint8>& init_data);

 protected:
  // Helper method to update the playing state.
  void UpdatePlayingState(bool is_playing_);

  // Helper methods for posting task for setting states and update WebKit.
  void UpdateNetworkState(blink::WebMediaPlayer::NetworkState state);
  void UpdateReadyState(blink::WebMediaPlayer::ReadyState state);

 private:
  void Pause(bool is_media_related_action);
  void DidLoadMediaInfo(MediaInfoLoader::Status status);
  bool IsKeySystemSupported(const std::string& key_system);

  // Actually do the work for generateKeyRequest/addKey so they can easily
  // report results to UMA.
  MediaKeyException GenerateKeyRequestInternal(const std::string& key_system,
                                               const unsigned char* init_data,
                                               unsigned init_data_length);
  MediaKeyException AddKeyInternal(const std::string& key_system,
                                   const unsigned char* key,
                                   unsigned key_length,
                                   const unsigned char* init_data,
                                   unsigned init_data_length,
                                   const std::string& session_id);
  MediaKeyException CancelKeyRequestInternal(const std::string& key_system,
                                             const std::string& session_id);

  blink::WebFrame* const frame_;

  blink::WebMediaPlayerClient* const client_;

  base::WeakPtr<media::WebMediaPlayerDelegate> delegate_;

  // Save the list of buffered time ranges.
  blink::WebTimeRanges buffered_;

  // Size of the video.
  blink::WebSize natural_size_;

  base::ThreadChecker main_thread_checker_;

  // Message loop for media thread.
//  const scoped_refptr<base::MessageLoopProxy> media_loop_;

  // URL of the media file to be fetched.
  GURL url_;

  // Media duration.
  base::TimeDelta duration_;

  // Flag to remember if we have a trusted duration_ value provided by
  // MediaSourceDelegate notifying OnDurationChanged(). In this case, ignore
  // any subsequent duration value passed to OnMediaMetadataChange().
  bool ignore_metadata_duration_change_;

  // Seek gets pending if another seek is in progress. Only last pending seek
  // will have effect.
  bool pending_seek_;
  base::TimeDelta pending_seek_time_;

  // Internal seek state.
  bool seeking_;
  base::TimeDelta seek_time_;

  // Whether loading has progressed since the last call to didLoadingProgress.
  bool did_loading_progress_;

  // Manages this object and delegates player calls to the browser process.
  // Owned by RenderFrameImpl.
  RendererMediaPlayerManager* player_manager_;

  // Player ID assigned by the |player_manager_|.
  int player_id_;

  // Current player states.
  blink::WebMediaPlayer::NetworkState network_state_;
  blink::WebMediaPlayer::ReadyState ready_state_;

  // Whether the mediaplayer is playing.
  bool is_playing_;

  // Whether the mediaplayer has already started playing.
  bool playing_started_;

  // Whether the video size info is available.
  bool has_size_info_;

  // Whether the video metadata and info are available.
  bool has_media_metadata_;
  bool has_media_info_;

  // Internal pending playback state.
  // Store a playback request that cannot be started immediately.
  bool pending_playback_;

  OxideHostMsg_MediaPlayer_Initialize_Type player_type_;

  // The current playing time. Because the media player is in the browser
  // process, it will regularly update the |current_time_| by calling
  // OnTimeUpdate().
  double current_time_;

  // Whether the browser is currently connected to a remote media player.
  bool is_remote_;

  media::MediaLog* media_log_;

  scoped_ptr<MediaInfoLoader> info_loader_;

  // The currently selected key system. Empty string means that no key system
  // has been selected.
  std::string current_key_system_;

  // Temporary for EME v0.1. In the future the init data type should be passed
  // through GenerateKeyRequest() directly from WebKit.
  std::string init_data_type_;

  // NOTE: Weak pointers must be invalidated before all other member variables.
  base::WeakPtrFactory<WebMediaPlayer> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebMediaPlayer);
};

}  // namespace content

#endif  // _OXIDE_RENDERER_MEDIA_UTOUCH_WEBMEDIAPLAYER_OXIDE_H_
