#ifndef MEDIA_MEDIAHUB_PLAYER_SHIM_H
#define MEDIA_MEDIAHUB_PLAYER_SHIM_H

#include <string>

typedef void*
MediaHubClientHandle;

typedef int
MediaHubPlayerId;

class MediaHubDelegate {
public:
  enum Status { null, ready, playing, paused, stopped };

  virtual void seeked_to(int64_t pos) = 0;
  virtual void end_of_stream() = 0;
  virtual void playback_status_changed(Status status) = 0;
};

#if defined(__cplusplus)
extern "C" {
#endif


enum Lifetime { normal, resumable };

/**
 * Creates a media hub player instance
 */
MediaHubClientHandle
mediahub_create_player(int player_id, MediaHubDelegate* delegate = 0);

/**
 * Create a media hub for use with this domain
 */
MediaHubClientHandle
mediahub_create_fixed_player(int player_id, const std::string& domain, MediaHubDelegate* delegate = nullptr);

/**
 * Resume previously created player session
 */
MediaHubClientHandle
mediahub_resume_player(int player_id, int player_key);


bool
mediahub_open_uri(MediaHubClientHandle handle,
                  const std::string& uri,
                  const std::string& cookies = std::string(),
                  const std::string& user_agent = std::string());


/**
 * 
 */
void
mediahub_play(MediaHubClientHandle);

/**
 * 
 */
void
mediahub_pause(MediaHubClientHandle);

/**
 * 
 */
void
mediahub_stop(MediaHubClientHandle);

/**
 * 
 */
unsigned long long
mediahub_get_duration(MediaHubClientHandle);

/**
 * 
 */
unsigned long long
mediahub_get_position(MediaHubClientHandle);

/**
 * 
 */
void
mediahub_release(MediaHubClientHandle);

/**
 * 
 */
int
mediahub_is_playing(MediaHubClientHandle);

/**
 * 
 */
int
mediahub_can_seek_forward(MediaHubClientHandle);

/**
 * 
 */
int
mediahub_can_seek_backward(MediaHubClientHandle);

/**
 * 
 */
void
mediahub_seek_to(MediaHubClientHandle handle, int64_t offset);

/**
 * 
 */
int
mediahub_can_pause(MediaHubClientHandle);

/**
 * 
 */
int
mediahub_is_player_ready(MediaHubClientHandle);

/**
 * 
 */
void
mediahub_set_volume(MediaHubClientHandle, double volume);

/**
 * Set lifetime of player session.
 */
void
mediahub_set_player_lifetime(MediaHubClientHandle handle, Lifetime lifetime);

#if defined(__cplusplus)
}
#endif

#endif // MEDIA_MEDIAHUB_PLAYER_SHIM_H

