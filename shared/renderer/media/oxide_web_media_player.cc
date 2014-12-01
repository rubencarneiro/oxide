// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "oxide_web_media_player.h"
#include "oxide_renderer_media_player_manager.h"

#include <limits>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/metrics/histogram.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "cc/layers/video_layer.h"
#include "content/public/common/content_client.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_frame.h"
#include "content/renderer/media/crypto/key_systems.h"
#include "content/renderer/media/webcontentdecryptionmodule_impl.h"
#include "media/blink/webmediaplayer_delegate.h"
#include "media/blink/webmediaplayer_util.h"
#include "content/renderer/render_frame_impl.h"
#include "content/renderer/render_thread_impl.h"
#include "gpu/GLES2/gl2extchromium.h"
#include "gpu/command_buffer/client/gles2_interface.h"
#include "gpu/command_buffer/common/mailbox_holder.h"
#include "media/base/bind_to_current_loop.h"
#include "media/base/media_keys.h"
#include "media/base/media_switches.h"
#include "media/base/video_frame.h"
#include "net/base/mime_util.h"
#include "third_party/WebKit/public/platform/WebMediaPlayerClient.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebURL.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebRuntimeFeatures.h"
#include "third_party/WebKit/public/web/WebSecurityOrigin.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "ui/gfx/image/image.h"
// #include "webkit/renderer/compositor_bindings/web_layer_impl.h"

static const uint32 kGLTextureExternalOES = 0x8D65;

using blink::WebMediaPlayer;
using blink::WebSize;
using blink::WebString;
using blink::WebTimeRanges;
using blink::WebURL;
using media::VideoFrame;

namespace {

// Prefix for histograms related to Encrypted Media Extensions.
const char* kMediaEme = "Media.EME.";

}  // namespace

namespace oxide {

WebMediaPlayer::WebMediaPlayer(
    blink::WebFrame* frame,
    blink::WebMediaPlayerClient* client,
    base::WeakPtr<media::WebMediaPlayerDelegate> delegate,
    RendererMediaPlayerManager* player_manager,
    media::MediaLog* media_log)
    : content::RenderFrameObserver(content::RenderFrame::FromWebFrame(frame)),
      frame_(frame),
      client_(client),
      delegate_(delegate),
      buffered_(static_cast<size_t>(1)),
      ignore_metadata_duration_change_(false),
      pending_seek_(false),
      seeking_(false),
      did_loading_progress_(false),
      player_manager_(player_manager),
      network_state_(WebMediaPlayer::NetworkStateEmpty),
      ready_state_(WebMediaPlayer::ReadyStateHaveNothing),
      is_playing_(false),
      playing_started_(false),
      has_size_info_(false),
      has_media_metadata_(false),
      has_media_info_(false),
      pending_playback_(false),
      player_type_(MEDIA_PLAYER_TYPE_URL),
      current_time_(0),
      is_remote_(false),
      media_log_(media_log),
      weak_factory_(this) {
  DCHECK(player_manager_);
  DCHECK(main_thread_checker_.CalledOnValidThread());

  player_id_ = player_manager_->RegisterMediaPlayer(this);
}

WebMediaPlayer::~WebMediaPlayer() {
  client_->setWebLayer(NULL);

  if (player_manager_) {
    player_manager_->DestroyPlayer(player_id_);
    player_manager_->UnregisterMediaPlayer(player_id_);
  }

  if (player_type_ == MEDIA_PLAYER_TYPE_MEDIA_SOURCE && delegate_)
    delegate_->PlayerGone(this);
}

void WebMediaPlayer::load(LoadType load_type,
                               const blink::WebURL& url,
                               CORSMode cors_mode) {
  switch (load_type) {
    case LoadTypeURL:
      player_type_ = MEDIA_PLAYER_TYPE_URL;
      break;

    case LoadTypeMediaSource:
      CHECK(false) << "WebMediaPlayer doesn't support MediaSource on "
                      "this platform";
      return;

    case LoadTypeMediaStream:
      CHECK(false) << "WebMediaPlayer doesn't support MediaStream on "
                      "this platform";
      return;
  }

  has_media_metadata_ = false;
  has_media_info_ = false;

  info_loader_.reset(
      new MediaInfoLoader(
          url,
          cors_mode,
          base::Bind(&WebMediaPlayer::DidLoadMediaInfo,
                     weak_factory_.GetWeakPtr())));

  // The url might be redirected when android media player
  // requests the stream. As a result, we cannot guarantee there is only
  // a single origin. Remove the following line when b/12573548 is fixed.
  // Check http://crbug.com/334204.

  info_loader_->set_single_origin(false);
  info_loader_->Start(frame_);

  url_ = url;

  if (player_manager_) {
    GURL first_party_url = frame_->document().firstPartyForCookies();
    player_manager_->Initialize(
        player_type_, player_id_, url, first_party_url);
  }

  UpdateNetworkState(WebMediaPlayer::NetworkStateLoading);
  UpdateReadyState(WebMediaPlayer::ReadyStateHaveNothing);
}

void WebMediaPlayer::DidLoadMediaInfo(MediaInfoLoader::Status status) {
  if (status == MediaInfoLoader::kFailed) {
    info_loader_.reset();
    UpdateNetworkState(WebMediaPlayer::NetworkStateNetworkError);
    return;
  }

  has_media_info_ = true;
  if (has_media_metadata_ &&
      ready_state_ != WebMediaPlayer::ReadyStateHaveEnoughData) {
    UpdateReadyState(WebMediaPlayer::ReadyStateHaveMetadata);
    UpdateReadyState(WebMediaPlayer::ReadyStateHaveEnoughData);
  }
  // Android doesn't start fetching resources until an implementation-defined
  // event (e.g. playback request) occurs. Sets the network state to IDLE
  // if play is not requested yet.
  if (!playing_started_)
    UpdateNetworkState(WebMediaPlayer::NetworkStateIdle);
}

void WebMediaPlayer::play() {
  if (paused())
    player_manager_->Start(player_id_);

  UpdatePlayingState(true);
  UpdateNetworkState(WebMediaPlayer::NetworkStateLoading);

  playing_started_ = true;
}

void WebMediaPlayer::pause() {
  Pause(true);
}

void WebMediaPlayer::seek(double seconds) {
  NOTIMPLEMENTED();
}

bool WebMediaPlayer::supportsSave() const {
  return false;
}

void WebMediaPlayer::setRate(double rate) {
  NOTIMPLEMENTED();
}

void WebMediaPlayer::setVolume(double volume) {
  player_manager_->SetVolume(player_id_, volume);
}

bool WebMediaPlayer::hasVideo() const {
  if (has_size_info_)
    return !natural_size_.isEmpty();

  // We don't know whether the current media content has video unless
  // the player is prepared. If the player is not prepared, we fall back
  // to the mime-type. There may be no mime-type on a redirect URL.
  // In that case, we conservatively assume it contains video so that
  // enterfullscreen call will not fail.

  if (!url_.has_path())
    return false;

  std::string mime;
  if (!net::GetMimeTypeFromFile(base::FilePath(url_.path()), &mime))
    return true;

  return mime.find("audio/") == std::string::npos;
}

void WebMediaPlayer::setPoster(const blink::WebURL& poster) {
  //  player_manager_->SetPoster(player_id_, poster);
}

bool WebMediaPlayer::hasAudio() const {
  if (!url_.has_path())
    return false;

  std::string mime;
  if (!net::GetMimeTypeFromFile(base::FilePath(url_.path()), &mime))
    return true;

  if (mime.find("audio/") != std::string::npos ||
      mime.find("video/") != std::string::npos ||
      mime.find("application/ogg") != std::string::npos) {
    return true;
  }

  return false;
}

bool WebMediaPlayer::paused() const {
  return !is_playing_;
}

bool WebMediaPlayer::seeking() const {
  return seeking_;
}

double WebMediaPlayer::duration() const {
  // HTML5 spec requires duration to be NaN if readyState is HAVE_NOTHING
  if (ready_state_ == WebMediaPlayer::ReadyStateHaveNothing) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  if (duration_ == media::kInfiniteDuration()) {
    return std::numeric_limits<double>::infinity();
  }
  return duration_.InSecondsF();
}

double WebMediaPlayer::timelineOffset() const {
  base::Time timeline_offset;

  if (timeline_offset.is_null()) {

    return std::numeric_limits<double>::quiet_NaN();
  }

  return timeline_offset.ToJsTime();
}

double WebMediaPlayer::currentTime() const {
  // If the player is processing a seek, return the seek time.
  // Blink may still query us if updatePlaybackState() occurs while seeking.
  if (seeking()) {
    return pending_seek_ ?
        pending_seek_time_.InSecondsF() : seek_time_.InSecondsF();
  }

  return current_time_;
}

WebSize WebMediaPlayer::naturalSize() const {
  return natural_size_;
}

WebMediaPlayer::NetworkState WebMediaPlayer::networkState() const {
  return network_state_;
}

WebMediaPlayer::ReadyState WebMediaPlayer::readyState() const {
  return ready_state_;
}

WebTimeRanges WebMediaPlayer::buffered() const {
  return buffered_;
}

WebTimeRanges WebMediaPlayer::seekable() const {
  return buffered_;
}

double WebMediaPlayer::maxTimeSeekable() const {
  // If we haven't even gotten to ReadyStateHaveMetadata yet then just
  // return 0 so that the seekable range is empty.
  if (ready_state_ < WebMediaPlayer::ReadyStateHaveMetadata)
    return 0.0;

  return duration();
}

bool WebMediaPlayer::didLoadingProgress() {
  bool ret = did_loading_progress_;
  did_loading_progress_ = false;
  return ret;
}

void WebMediaPlayer::paint(blink::WebCanvas* canvas,
                                  const blink::WebRect& rect,
                                  unsigned char alpha) {
  NOTIMPLEMENTED();
}

bool WebMediaPlayer::copyVideoTextureToPlatformTexture(
    blink::WebGraphicsContext3D* web_graphics_context,
    unsigned int texture,
    unsigned int level,
    unsigned int internal_format,
    unsigned int type,
    bool premultiply_alpha,
    bool flip_y) {
  NOTIMPLEMENTED();
  return false;
}

bool WebMediaPlayer::hasSingleSecurityOrigin() const {
  if (info_loader_)
    return info_loader_->HasSingleOrigin();

  // The info loader may have failed.
  if (player_type_ == MEDIA_PLAYER_TYPE_URL)
    return false;

  return true;
}

bool WebMediaPlayer::didPassCORSAccessCheck() const {
  if (info_loader_)
    return info_loader_->DidPassCORSAccessCheck();

  return false;
}

double WebMediaPlayer::mediaTimeForTimeValue(double timeValue) const {
  return media::ConvertSecondsToTimestamp(timeValue).InSecondsF();
}

unsigned WebMediaPlayer::decodedFrameCount() const {
  NOTIMPLEMENTED();

  return 0;
}

unsigned WebMediaPlayer::droppedFrameCount() const {
  NOTIMPLEMENTED();

  return 0;
}

unsigned WebMediaPlayer::audioDecodedByteCount() const {
  NOTIMPLEMENTED();

  return 0;
}

unsigned WebMediaPlayer::videoDecodedByteCount() const {
  NOTIMPLEMENTED();

  return 0;
}

void WebMediaPlayer::paint(blink::WebCanvas*, const blink::WebRect&, unsigned char alpha, SkXfermode::Mode) {
  NOTIMPLEMENTED();
}

void WebMediaPlayer::OnMediaMetadataChanged(
    const base::TimeDelta& duration, int width, int height, bool success) {
  bool need_to_signal_duration_changed = false;

  if (url_.SchemeIs("file"))
    UpdateNetworkState(WebMediaPlayer::NetworkStateLoaded);

  // Update duration, if necessary, prior to ready state updates that may
  // cause duration() query.
  if (!ignore_metadata_duration_change_ && duration_ != duration) {
    duration_ = duration;

    // Client readyState transition from HAVE_NOTHING to HAVE_METADATA
    // already triggers a durationchanged event. If this is a different
    // transition, remember to signal durationchanged.
    // Do not ever signal durationchanged on metadata change in MSE case
    // because OnDurationChanged() handles this.
    if (ready_state_ > WebMediaPlayer::ReadyStateHaveNothing &&
        player_type_ != MEDIA_PLAYER_TYPE_MEDIA_SOURCE) {
      need_to_signal_duration_changed = true;
    }
  }

  has_media_metadata_ = true;
  if (has_media_info_ &&
      ready_state_ != WebMediaPlayer::ReadyStateHaveEnoughData) {
    UpdateReadyState(WebMediaPlayer::ReadyStateHaveMetadata);
    UpdateReadyState(WebMediaPlayer::ReadyStateHaveEnoughData);
  }

  // TODO(wolenetz): Should we just abort early and set network state to an
  // error if success == false? See http://crbug.com/248399
  if (success)
    OnVideoSizeChanged(width, height);

  if (need_to_signal_duration_changed)
    client_->durationChanged();
}

void WebMediaPlayer::OnPlaybackComplete() {
  // When playback is about to finish, android media player often stops
  // at a time which is smaller than the duration. This makes webkit never
  // know that the playback has finished. To solve this, we set the
  // current time to media duration when OnPlaybackComplete() get called.
  OnTimeUpdate(duration_);
  client_->timeChanged();

  // if the loop attribute is set, timeChanged() will update the current time
  // to 0. It will perform a seek to 0. As the requests to the renderer
  // process are sequential, the OnSeekComplete() will only occur
  // once OnPlaybackComplete() is done. As the playback can only be executed
  // upon completion of OnSeekComplete(), the request needs to be saved.
  is_playing_ = false;
  if (seeking_ && seek_time_ == base::TimeDelta())
    pending_playback_ = true;
}

void WebMediaPlayer::OnBufferingUpdate(int percentage) {
  buffered_[0].end = duration() * percentage / 100;
  did_loading_progress_ = true;
}

void WebMediaPlayer::OnSeekRequest(const base::TimeDelta& time_to_seek) {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  client_->requestSeek(time_to_seek.InSecondsF());
}

void WebMediaPlayer::OnSeekComplete(
    const base::TimeDelta& current_time) {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  seeking_ = false;
  if (pending_seek_) {
    pending_seek_ = false;
    seek(pending_seek_time_.InSecondsF());
    return;
  }

  OnTimeUpdate(current_time);

  UpdateReadyState(WebMediaPlayer::ReadyStateHaveEnoughData);

  client_->timeChanged();

  if (pending_playback_) {
    play();
    pending_playback_ = false;
  }
}

void WebMediaPlayer::OnMediaError(int error_type) {
  client_->repaint();
}

void WebMediaPlayer::OnVideoSizeChanged(int width, int height) {
  has_size_info_ = true;
  if (natural_size_.width == width && natural_size_.height == height)
    return;

  natural_size_.width = width;
  natural_size_.height = height;

  // TODO(qinmin): This is a hack. We need the media element to stop showing the
  // poster image by forcing it to call setDisplayMode(video). Should move the
  // logic into HTMLMediaElement.cpp.
  client_->timeChanged();
}

void WebMediaPlayer::OnTimeUpdate(const base::TimeDelta& current_time) {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  current_time_ = current_time.InSecondsF();
}

void WebMediaPlayer::OnDidEnterFullscreen() {
  NOTIMPLEMENTED();
}

void WebMediaPlayer::OnDidExitFullscreen() {
  NOTIMPLEMENTED();
}

void WebMediaPlayer::OnMediaPlayerPlay() {
  UpdateNetworkState(WebMediaPlayer::NetworkStateLoaded);
  UpdatePlayingState(true);
  client_->playbackStateChanged();
}

void WebMediaPlayer::OnMediaPlayerPause() {
  UpdatePlayingState(false);
  client_->playbackStateChanged();
}

void WebMediaPlayer::OnRequestFullscreen() {
  NOTIMPLEMENTED();
}

void WebMediaPlayer::OnDurationChanged(const base::TimeDelta& duration) {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  // Only MSE |player_type_| registers this callback.
  DCHECK_EQ(player_type_, MEDIA_PLAYER_TYPE_MEDIA_SOURCE);

  // Cache the new duration value and trust it over any subsequent duration
  // values received in OnMediaMetadataChanged().
  duration_ = duration;
  ignore_metadata_duration_change_ = true;

  // Notify MediaPlayerClient that duration has changed, if > HAVE_NOTHING.
  if (ready_state_ > WebMediaPlayer::ReadyStateHaveNothing)
    client_->durationChanged();
}

void WebMediaPlayer::UpdateNetworkState(
    WebMediaPlayer::NetworkState state) {
  DCHECK(main_thread_checker_.CalledOnValidThread());
  if (ready_state_ == WebMediaPlayer::ReadyStateHaveNothing &&
      (state == WebMediaPlayer::NetworkStateNetworkError ||
       state == WebMediaPlayer::NetworkStateDecodeError)) {
    // Any error that occurs before reaching ReadyStateHaveMetadata should
    // be considered a format error.
    network_state_ = WebMediaPlayer::NetworkStateFormatError;
  } else {
    network_state_ = state;
  }
  client_->networkStateChanged();
}

void WebMediaPlayer::UpdateReadyState(
    WebMediaPlayer::ReadyState state) {
  ready_state_ = state;
  client_->readyStateChanged();
}

void WebMediaPlayer::OnPlayerReleased() {
  if (is_playing_)
    OnMediaPlayerPause();
}

void WebMediaPlayer::ReleaseMediaResources() {
  switch (network_state_) {
    // Pause the media player and inform WebKit if the player is in a good
    // shape.
    case WebMediaPlayer::NetworkStateIdle:
    case WebMediaPlayer::NetworkStateLoading:
    case WebMediaPlayer::NetworkStateLoaded:
      Pause(false);
      client_->playbackStateChanged();
      break;
    // If a WebMediaPlayer instance has entered into one of these states,
    // the internal network state in HTMLMediaElement could be set to empty.
    // And calling playbackStateChanged() could get this object deleted.
    case WebMediaPlayer::NetworkStateEmpty:
    case WebMediaPlayer::NetworkStateFormatError:
    case WebMediaPlayer::NetworkStateNetworkError:
    case WebMediaPlayer::NetworkStateDecodeError:
      break;
  }
  player_manager_->ReleaseResources(player_id_);
  OnPlayerReleased();
}

void WebMediaPlayer::OnDestruct() {
  if (player_manager_)
    player_manager_->UnregisterMediaPlayer(player_id_);
  Detach();
}

void WebMediaPlayer::Detach() {
  is_remote_ = false;
  player_manager_ = NULL;
}

void WebMediaPlayer::Pause(bool is_media_related_action) {
  if (player_manager_)
    player_manager_->Pause(player_id_, is_media_related_action);
  UpdatePlayingState(false);
}

void WebMediaPlayer::UpdatePlayingState(bool is_playing) {
  is_playing_ = is_playing;
  if (!delegate_)
    return;
  if (is_playing)
    delegate_->DidPlay(this);
  else
    delegate_->DidPause(this);
}

// The following EME related code is copied from WebMediaPlayerImpl.
// TODO(xhwang): Remove duplicate code between WebMediaPlayer and
// WebMediaPlayerImpl.

// Convert a WebString to ASCII, falling back on an empty string in the case
// of a non-ASCII string.
static std::string ToASCIIOrEmpty(const blink::WebString& string) {
  return base::IsStringASCII(string) ? base::UTF16ToASCII(string)
                                     : std::string();
}

// Helper functions to report media EME related stats to UMA. They follow the
// convention of more commonly used macros UMA_HISTOGRAM_ENUMERATION and
// UMA_HISTOGRAM_COUNTS. The reason that we cannot use those macros directly is
// that UMA_* macros require the names to be constant throughout the process'
// lifetime.

static void EmeUMAHistogramEnumeration(const std::string& key_system,
                                       const std::string& method,
                                       int sample,
                                       int boundary_value) {
  base::LinearHistogram::FactoryGet(
      kMediaEme + content::KeySystemNameForUMA(key_system) + "." + method,
      1, boundary_value, boundary_value + 1,
      base::Histogram::kUmaTargetedHistogramFlag)->Add(sample);
}

static void EmeUMAHistogramCounts(const std::string& key_system,
                                  const std::string& method,
                                  int sample) {
  // Use the same parameters as UMA_HISTOGRAM_COUNTS.
  base::Histogram::FactoryGet(
      kMediaEme + content::KeySystemNameForUMA(key_system) + "." + method,
      1, 1000000, 50, base::Histogram::kUmaTargetedHistogramFlag)->Add(sample);
}

// Helper enum for reporting generateKeyRequest/addKey histograms.
enum MediaKeyException {
  kUnknownResultId,
  kSuccess,
  kKeySystemNotSupported,
  kInvalidPlayerState,
  kMaxMediaKeyException
};

static MediaKeyException MediaKeyExceptionForUMA(
    WebMediaPlayer::MediaKeyException e) {
  switch (e) {
    case WebMediaPlayer::MediaKeyExceptionKeySystemNotSupported:
      return kKeySystemNotSupported;
    case WebMediaPlayer::MediaKeyExceptionInvalidPlayerState:
      return kInvalidPlayerState;
    case WebMediaPlayer::MediaKeyExceptionNoError:
      return kSuccess;
    default:
      return kUnknownResultId;
  }
}

// Helper for converting |key_system| name and exception |e| to a pair of enum
// values from above, for reporting to UMA.
static void ReportMediaKeyExceptionToUMA(const std::string& method,
                                         const std::string& key_system,
                                         WebMediaPlayer::MediaKeyException e) {
  MediaKeyException result_id = MediaKeyExceptionForUMA(e);
  DCHECK_NE(result_id, kUnknownResultId) << e;
  EmeUMAHistogramEnumeration(
      key_system, method, result_id, kMaxMediaKeyException);
}

bool WebMediaPlayer::IsKeySystemSupported(
    const std::string& key_system) {
  // TODO
  return player_type_ == MEDIA_PLAYER_TYPE_MEDIA_SOURCE &&
         content::IsConcreteSupportedKeySystem(key_system);
}

WebMediaPlayer::MediaKeyException WebMediaPlayer::generateKeyRequest(
    const WebString& key_system,
    const unsigned char* init_data,
    unsigned init_data_length) {
  DVLOG(1) << "generateKeyRequest: " << base::string16(key_system) << ": "
           << std::string(reinterpret_cast<const char*>(init_data),
                          static_cast<size_t>(init_data_length));

  std::string ascii_key_system =
      content::GetUnprefixedKeySystemName(ToASCIIOrEmpty(key_system));

  WebMediaPlayer::MediaKeyException e =
      GenerateKeyRequestInternal(ascii_key_system, init_data, init_data_length);
  ReportMediaKeyExceptionToUMA("generateKeyRequest", ascii_key_system, e);
  return e;
}

// Guess the type of |init_data|. This is only used to handle some corner cases
// so we keep it as simple as possible without breaking major use cases.
/*
static std::string GuessInitDataType(const unsigned char* init_data,
                                     unsigned init_data_length) {
  // Most WebM files use KeyId of 16 bytes. MP4 init data are always >16 bytes.
  if (init_data_length == 16)
    return "video/webm";

  return "video/mp4";
}
*/

// TODO(xhwang): Report an error when there is encrypted stream but EME is
// not enabled. Currently the player just doesn't start and waits for
// ever.
WebMediaPlayer::MediaKeyException
WebMediaPlayer::GenerateKeyRequestInternal(
    const std::string& key_system,
    const unsigned char* init_data,
    unsigned init_data_length) {
  NOTIMPLEMENTED();
  return WebMediaPlayer::MediaKeyExceptionKeySystemNotSupported;
}

WebMediaPlayer::MediaKeyException WebMediaPlayer::addKey(
    const WebString& key_system,
    const unsigned char* key,
    unsigned key_length,
    const unsigned char* init_data,
    unsigned init_data_length,
    const WebString& session_id) {
  DVLOG(1) << "addKey: " << base::string16(key_system) << ": "
           << std::string(reinterpret_cast<const char*>(key),
                          static_cast<size_t>(key_length)) << ", "
           << std::string(reinterpret_cast<const char*>(init_data),
                          static_cast<size_t>(init_data_length)) << " ["
           << base::string16(session_id) << "]";

  std::string ascii_key_system =
      content::GetUnprefixedKeySystemName(ToASCIIOrEmpty(key_system));
  std::string ascii_session_id = ToASCIIOrEmpty(session_id);

  WebMediaPlayer::MediaKeyException e = AddKeyInternal(ascii_key_system,
                                                       key,
                                                       key_length,
                                                       init_data,
                                                       init_data_length,
                                                       ascii_session_id);
  ReportMediaKeyExceptionToUMA("addKey", ascii_key_system, e);
  return e;
}

WebMediaPlayer::MediaKeyException WebMediaPlayer::AddKeyInternal(
    const std::string& key_system,
    const unsigned char* key,
    unsigned key_length,
    const unsigned char* init_data,
    unsigned init_data_length,
    const std::string& session_id) {
  DCHECK(key);
  DCHECK_GT(key_length, 0u);

  if (!IsKeySystemSupported(key_system))
    return WebMediaPlayer::MediaKeyExceptionKeySystemNotSupported;

  if (current_key_system_.empty() || key_system != current_key_system_)
    return WebMediaPlayer::MediaKeyExceptionInvalidPlayerState;

  return WebMediaPlayer::MediaKeyExceptionNoError;
}

WebMediaPlayer::MediaKeyException WebMediaPlayer::cancelKeyRequest(
    const WebString& key_system,
    const WebString& session_id) {
  DVLOG(1) << "cancelKeyRequest: " << base::string16(key_system) << ": "
           << " [" << base::string16(session_id) << "]";

  std::string ascii_key_system =
      content::GetUnprefixedKeySystemName(ToASCIIOrEmpty(key_system));
  std::string ascii_session_id = ToASCIIOrEmpty(session_id);

  WebMediaPlayer::MediaKeyException e =
      CancelKeyRequestInternal(ascii_key_system, ascii_session_id);
  ReportMediaKeyExceptionToUMA("cancelKeyRequest", ascii_key_system, e);
  return e;
}

WebMediaPlayer::MediaKeyException
WebMediaPlayer::CancelKeyRequestInternal(const std::string& key_system,
                                                const std::string& session_id) {
  if (!IsKeySystemSupported(key_system))
    return WebMediaPlayer::MediaKeyExceptionKeySystemNotSupported;

  if (current_key_system_.empty() || key_system != current_key_system_)
    return WebMediaPlayer::MediaKeyExceptionInvalidPlayerState;

  return WebMediaPlayer::MediaKeyExceptionNoError;
}

void WebMediaPlayer::setContentDecryptionModule(
    blink::WebContentDecryptionModule* cdm) {
  NOTIMPLEMENTED();
}

void WebMediaPlayer::OnKeyAdded(const std::string& session_id) {
  EmeUMAHistogramCounts(current_key_system_, "KeyAdded", 1);

  client_->keyAdded(
      WebString::fromUTF8(content::GetPrefixedKeySystemName(current_key_system_)),
      WebString::fromUTF8(session_id));
}

void WebMediaPlayer::OnKeyError(const std::string& session_id,
                                       media::MediaKeys::KeyError error_code,
                                       uint32 system_code) {
  EmeUMAHistogramEnumeration(current_key_system_, "KeyError",
                             error_code, media::MediaKeys::kMaxKeyError);

  unsigned short short_system_code = 0;
  if (system_code > std::numeric_limits<unsigned short>::max()) {
    LOG(WARNING) << "system_code exceeds unsigned short limit.";
    short_system_code = std::numeric_limits<unsigned short>::max();
  } else {
    short_system_code = static_cast<unsigned short>(system_code);
  }

  client_->keyError(
      WebString::fromUTF8(content::GetPrefixedKeySystemName(current_key_system_)),
      WebString::fromUTF8(session_id),
      static_cast<blink::WebMediaPlayerClient::MediaKeyErrorCode>(error_code),
      short_system_code);
}

void WebMediaPlayer::OnKeyMessage(const std::string& session_id,
                                         const std::vector<uint8>& message,
                                         const GURL& destination_url) {
  DCHECK(destination_url.is_empty() || destination_url.is_valid());

  client_->keyMessage(
      WebString::fromUTF8(content::GetPrefixedKeySystemName(current_key_system_)),
      WebString::fromUTF8(session_id),
      message.empty() ? NULL : &message[0],
      message.size(),
      destination_url);
}

void WebMediaPlayer::OnMediaSourceOpened(
    blink::WebMediaSource* web_media_source) {
  client_->mediaSourceOpened(web_media_source);
}

void WebMediaPlayer::OnNeedKey(const std::string& type,
                                      const std::vector<uint8>& init_data) {
  DCHECK(main_thread_checker_.CalledOnValidThread());

  // Do not fire NeedKey event if encrypted media is not enabled.
  if (!blink::WebRuntimeFeatures::isPrefixedEncryptedMediaEnabled() &&
      !blink::WebRuntimeFeatures::isEncryptedMediaEnabled()) {
    return;
  }

  UMA_HISTOGRAM_COUNTS(kMediaEme + std::string("NeedKey"), 1);

  DCHECK(init_data_type_.empty() || type.empty() || type == init_data_type_);
  if (init_data_type_.empty())
    init_data_type_ = type;

  const uint8* init_data_ptr = init_data.empty() ? NULL : &init_data[0];
  client_->encrypted(
      WebString::fromUTF8(type), init_data_ptr, init_data.size());
}

void WebMediaPlayer::enterFullscreen() {
  NOTIMPLEMENTED();
}

void WebMediaPlayer::exitFullscreen() {
  NOTIMPLEMENTED();
}

bool WebMediaPlayer::canEnterFullscreen() const {
  return false;
}

}  // namespace content
