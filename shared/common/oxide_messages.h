// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2013 Canonical Ltd.

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

#include <string>

#include "base/memory/shared_memory.h"
#include "cc/input/top_controls_state.h"
#include "content/public/common/common_param_traits.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_message_start.h"
#include "url/gurl.h"

#include "shared/common/oxide_message_enums.h"
#include "shared/common/oxide_script_message_request.h"

IPC_ENUM_TRAITS(OxideMsg_SendMessage_Type::Value)
IPC_ENUM_TRAITS(oxide::ScriptMessageRequest::Error)
IPC_ENUM_TRAITS(cc::TopControlsState)

IPC_STRUCT_BEGIN(OxideMsg_SendMessage_Params)
  IPC_STRUCT_MEMBER(std::string, context)
  IPC_STRUCT_MEMBER(int, serial)
  IPC_STRUCT_MEMBER(OxideMsg_SendMessage_Type::Value, type)
  IPC_STRUCT_MEMBER(oxide::ScriptMessageRequest::Error, error)
  IPC_STRUCT_MEMBER(std::string, msg_id)
  IPC_STRUCT_MEMBER(std::string, payload)
IPC_STRUCT_END()

#define IPC_MESSAGE_START OxideMsgStart

IPC_MESSAGE_CONTROL1(OxideMsg_UpdateUserScripts,
                     base::SharedMemoryHandle)

IPC_MESSAGE_CONTROL1(OxideMsg_SetUserAgent,
                     std::string)

IPC_MESSAGE_ROUTED1(OxideHostMsg_SendMessage,
                    OxideMsg_SendMessage_Params)

IPC_MESSAGE_ROUTED1(OxideMsg_SendMessage,
                    OxideMsg_SendMessage_Params)

IPC_MESSAGE_ROUTED1(OxideMsg_SetAllowDisplayingInsecureContent,
                    bool)
IPC_MESSAGE_ROUTED1(OxideMsg_SetAllowRunningInsecureContent,
                    bool)
IPC_MESSAGE_ROUTED0(OxideMsg_ReloadFrame)

IPC_MESSAGE_ROUTED3(OxideMsg_UpdateTopControlsState,
                    cc::TopControlsState,
                    cc::TopControlsState,
                    bool)

IPC_MESSAGE_ROUTED0(OxideHostMsg_DidBlockDisplayingInsecureContent)
IPC_MESSAGE_ROUTED0(OxideHostMsg_DidBlockRunningInsecureContent)

IPC_SYNC_MESSAGE_CONTROL1_2(OxideHostMsg_GetUserAgentOverride,
                            GURL,
                            std::string,
                            bool)

// Media
IPC_ENUM_TRAITS(OxideHostMsg_MediaPlayer_Initialize_Type)

// Parameters to describe a media player
IPC_STRUCT_BEGIN(OxideHostMsg_MediaPlayer_Initialize_Params)
  IPC_STRUCT_MEMBER(OxideHostMsg_MediaPlayer_Initialize_Type, type)
  IPC_STRUCT_MEMBER(int, player_id)
  IPC_STRUCT_MEMBER(GURL, url)
  IPC_STRUCT_MEMBER(GURL, first_party_for_cookies)
  IPC_STRUCT_MEMBER(GURL, frame_url)
IPC_STRUCT_END()

// Media buffering has updated.
IPC_MESSAGE_ROUTED2(OxideMsg_MediaPlayer_MediaBufferingUpdate,
                    int /* player_id */,
                    int /* percent */)

// A media playback error has occurred.
IPC_MESSAGE_ROUTED2(OxideMsg_MediaPlayer_MediaError,
                    int /* player_id */,
                    int /* error */)

// Playback is completed.
IPC_MESSAGE_ROUTED1(OxideMsg_MediaPlayer_MediaPlaybackCompleted,
                    int /* player_id */)

// Media metadata has changed.
IPC_MESSAGE_ROUTED5(OxideMsg_MediaPlayer_MediaMetadataChanged,
                    int /* player_id */,
                    base::TimeDelta /* duration */,
                    int /* width */,
                    int /* height */,
                    bool /* success */)

// Requests renderer player to ask its client (blink HTMLMediaElement) to seek.
IPC_MESSAGE_ROUTED2(OxideMsg_MediaPlayer_SeekRequest,
                    int /* player_id */,
                    base::TimeDelta /* time_to_seek_to */)

// Media seek is completed.
IPC_MESSAGE_ROUTED2(OxideMsg_MediaPlayer_SeekCompleted,
                    int /* player_id */,
                    base::TimeDelta /* current_time */)

// Video size has changed.
IPC_MESSAGE_ROUTED3(OxideMsg_MediaPlayer_MediaVideoSizeChanged,
                    int /* player_id */,
                    int /* width */,
                    int /* height */)

// The current play time has updated.
IPC_MESSAGE_ROUTED2(OxideMsg_MediaPlayer_MediaTimeUpdate,
                    int /* player_id */,
                    base::TimeDelta /* current_time */)

// The player has been released.
IPC_MESSAGE_ROUTED1(OxideMsg_MediaPlayer_MediaPlayerReleased,
                    int /* player_id */)

// The player has entered fullscreen mode.
IPC_MESSAGE_ROUTED1(OxideMsg_MediaPlayer_DidEnterFullscreen,
                    int /* player_id */)

// The player exited fullscreen.
IPC_MESSAGE_ROUTED1(OxideMsg_MediaPlayer_DidExitFullscreen,
                    int /* player_id */)

// The player started playing.
IPC_MESSAGE_ROUTED1(OxideMsg_MediaPlayer_DidMediaPlayerPlay,
                    int /* player_id */)

// The player was paused.
IPC_MESSAGE_ROUTED1(OxideMsg_MediaPlayer_DidMediaPlayerPause,
                    int /* player_id */)

// Instructs the video element to enter fullscreen.
IPC_MESSAGE_ROUTED1(OxideMsg_MediaPlayer_RequestFullscreen,
                    int /*player_id */)

// Pauses all video playback.
IPC_MESSAGE_ROUTED0(OxideMsg_MediaPlayer_PauseVideo)

// Messages for controlling the media playback in browser process ----------

// Destroy the media player object.
IPC_MESSAGE_ROUTED1(OxideHostMsg_MediaPlayer_DestroyMediaPlayer,
                    int /* player_id */)

// Initialize a media player object.
IPC_MESSAGE_ROUTED1(
    OxideHostMsg_MediaPlayer_Initialize,
    OxideHostMsg_MediaPlayer_Initialize_Params);

// Pause the player.
IPC_MESSAGE_ROUTED2(OxideHostMsg_MediaPlayer_Pause,
                    int /* player_id */,
                    bool /* is_media_related_action */)

// Release player resources, but keep the object for future usage.
IPC_MESSAGE_ROUTED1(OxideHostMsg_MediaPlayer_Release, int /* player_id */)

// Perform a seek.
IPC_MESSAGE_ROUTED2(OxideHostMsg_MediaPlayer_Seek,
                    int /* player_id */,
                    base::TimeDelta /* time */)

// Start the player for playback.
IPC_MESSAGE_ROUTED1(OxideHostMsg_MediaPlayer_Start, int /* player_id */)

// Set the volume.
IPC_MESSAGE_ROUTED2(OxideHostMsg_MediaPlayer_SetVolume,
                    int /* player_id */,
                    double /* volume */)

// Set the poster image.
IPC_MESSAGE_ROUTED2(OxideHostMsg_MediaPlayer_SetPoster,
                    int /* player_id */,
                    GURL /* poster url */)

// Requests the player to enter fullscreen.
IPC_MESSAGE_ROUTED1(OxideHostMsg_MediaPlayer_EnterFullscreen, int /* player_id */)

// Requests the player to exit fullscreen.
IPC_MESSAGE_ROUTED1(OxideHostMsg_MediaPlayer_ExitFullscreen, int /* player_id */)

// Inform the media source player of changed media duration from demuxer.
IPC_MESSAGE_CONTROL2(OxideHostMsg_MediaPlayer_DurationChanged,
                     int /* demuxer_client_id */,
                     base::TimeDelta /* duration */)
