#include "mediahub_player_shim.h"

#include <map>
#include <ostream>
#include <core/media/service.h>
#include <core/media/player.h>

using namespace core::ubuntu::media;

namespace {

typedef std::map<int, std::shared_ptr<Player> > player_map;

player_map g_mediahub_players;

player_map::iterator find_player(Player *player)
{
  player_map::iterator it = g_mediahub_players.begin();
  for (; it != g_mediahub_players.end(); it++) {
    if (it->second.get() == player) {
      break;
    }
  }
  return it;
}

void setup_delegate(MediaHubDelegate *delegate, std::shared_ptr<Player>& player)
{
  player->seeked_to().connect([delegate](int64_t pos) {
          delegate->seeked_to(pos);
      });

  player->end_of_stream().connect([delegate]() {
          delegate->end_of_stream();
      });

  player->playback_status_changed().connect([delegate](Player::PlaybackStatus status) {
          delegate->playback_status_changed(
                      static_cast<MediaHubDelegate::Status>(status)
                    );
      });
}
}


MediaHubClientHandle
mediahub_create_player(int player_id, MediaHubDelegate *delegate)
{
  try {
    if (player_id < 0) {
      return MediaHubClientHandle();
    }

    if (g_mediahub_players.find(player_id) != g_mediahub_players.end()) {
      return MediaHubClientHandle();
    }

    std::shared_ptr<Player> player =
        Service::Client::instance()->create_session(
            Player::Client::default_configuration());

    if (delegate) {
      setup_delegate(delegate, player);
    }

    g_mediahub_players[player_id] = player;

    return MediaHubClientHandle(static_cast<void*>(player.get()));
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return MediaHubClientHandle();
}

MediaHubClientHandle
mediahub_create_fixed_player(int player_id, const std::string& domain, MediaHubDelegate *delegate)
{
  try {
    if (player_id < 0) {
      return MediaHubClientHandle();
    }

    if (g_mediahub_players.find(player_id) != g_mediahub_players.end()) {
      return MediaHubClientHandle();
    }

    std::shared_ptr<Player> player =
        Service::Client::instance()->create_fixed_session(domain,
            Player::Client::default_configuration());

    if (delegate) {
      setup_delegate(delegate, player);

      delegate->playback_status_changed(
                    MediaHubDelegate::Status(player->playback_status().get())
                  );
    }

    g_mediahub_players[player_id] = player;

    return MediaHubClientHandle(static_cast<void*>(player.get()));
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return MediaHubClientHandle();
}

MediaHubClientHandle
mediahub_resume_player(int player_id, int player_key)
{
  try {
    if (player_id < 0) {
      return MediaHubClientHandle();
    }

    if (g_mediahub_players.find(player_id) != g_mediahub_players.end()) {
      return MediaHubClientHandle();
    }

    std::shared_ptr<Player> player =
        Service::Client::instance()->resume_session(player_key);

    g_mediahub_players[player_id] = player;

    return MediaHubClientHandle(static_cast<void*>(player.get()));
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return MediaHubClientHandle();
}

bool
mediahub_open_uri(MediaHubClientHandle handle,
                  const std::string& uri,
                  const std::string& cookies,
                  const std::string& user_agent)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player != 0) {
      if (player->playback_status().get() == Player::playing) {
        player->stop();
      }
      core::ubuntu::media::Player::HeadersType headers;
      if (!cookies.empty()) {
        headers["Cookie"] = cookies;
      }
      if (!user_agent.empty()) {
        headers["User-Agent"] = user_agent;
      }
      return player->open_uri(uri, headers);
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return false;
}


void
mediahub_play(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      player->play();
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
}

void
mediahub_pause(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      player->pause();
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
}

void
mediahub_stop(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      player->stop();
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
}

unsigned long long
mediahub_get_duration(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      return player->duration().get();
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return 0;
}

unsigned long long
mediahub_get_position(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      return player->position().get();
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return 0;
}

void
mediahub_release(MediaHubClientHandle handle)
{
  try {
    auto it = find_player(static_cast<Player*>(handle));
    if (it != g_mediahub_players.end()) {
      g_mediahub_players.erase(it);
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
}

int
mediahub_is_playing(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      return player->playback_status().get() != Player::playing;
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return 0;
}

int
mediahub_can_seek_forward(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      return player->can_seek().get();
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return 0;
}

int
mediahub_can_seek_backward(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      return player->can_seek().get();
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return 0;
}

void
mediahub_seek_to(MediaHubClientHandle handle, int64_t offset)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      return player->seek_to(std::chrono::microseconds(offset));
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
}

int
mediahub_can_pause(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      return player->can_pause().get();
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return 0;
}

int
mediahub_is_player_ready(MediaHubClientHandle handle)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      return player->playback_status().get() != Player::null;
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
  return 0;
}

void
mediahub_set_volume(MediaHubClientHandle handle, double volume)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      player->volume().set(volume);
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
}

void
mediahub_set_rate(MediaHubClientHandle handle, double rate)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      player->playback_rate().set(rate);
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
}

void
mediahub_set_player_lifetime(MediaHubClientHandle handle, Lifetime lifetime)
{
  try {
    Player* player = static_cast<Player*>(handle);
    if (player) {
      player->lifetime().set(Player::Lifetime(lifetime));
    }
  } catch (std::runtime_error& error) {
    std::cerr << __PRETTY_FUNCTION__ << " " << error.what() << std::endl;
  }
}

