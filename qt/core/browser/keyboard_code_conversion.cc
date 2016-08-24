// vim:expandtab:shiftwidth=2:tabstop=2:
// Copyright (C) 2014-2016 Canonical Ltd.

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

#include "keyboard_code_conversion.h"

#include <QKeyEvent>
#include <Qt>

namespace oxide {
namespace qt {

ui::DomKey GetDomKeyFromQKeyEvent(QKeyEvent* event) {
  if (!event->text().isEmpty()) {
    return ui::DomKey::FromCharacter(event->text().utf16()[0]);
  }

  int keycode = event->key();

  // =============================
  // General purpose function keys
  // =============================
  if (keycode >= Qt::Key_F1 && keycode <= Qt::Key_F24) {
    return (keycode - Qt::Key_F1) + ui::DomKey::Key::F1;
  }

  switch (keycode) {
    case Qt::Key_Backspace:
      return ui::DomKey::Key::BACKSPACE;
    case Qt::Key_Tab:
      return ui::DomKey::Key::TAB;
    case Qt::Key_Return:
    case Qt::Key_Enter:
      return ui::DomKey::Key::ENTER;
    case Qt::Key_Escape:
      return ui::DomKey::Key::ESCAPE;
    case Qt::Key_Delete:
      return ui::DomKey::Key::DEL;

    // =============
    // Modifier keys
    // =============
    // ACCEL
    case Qt::Key_Alt:
      return ui::DomKey::Key::ALT;
    case Qt::Key_AltGr:
      return ui::DomKey::Key::ALT_GRAPH;
    case Qt::Key_CapsLock:
      return ui::DomKey::Key::CAPS_LOCK;
    case Qt::Key_Control:
      return ui::DomKey::Key::CONTROL;
    // FN
    // FN_LOCK
    case Qt::Key_Hyper_L:
    case Qt::Key_Hyper_R:
      return ui::DomKey::Key::HYPER;
    case Qt::Key_Meta:
      return ui::DomKey::Key::META;
    case Qt::Key_NumLock:
      return ui::DomKey::Key::NUM_LOCK;
    case Qt::Key_ScrollLock:
      return ui::DomKey::Key::SCROLL_LOCK;
    case Qt::Key_Shift:
      return ui::DomKey::Key::SHIFT;
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
      return ui::DomKey::Key::SUPER;
    // SYMBOL
    // SYMBOL_LOCK
    // SHIFT_LEVEL5
    // ALT_GRAPH_LATCH

    // ===============
    // Navigation keys
    // ===============
    case Qt::Key_Down:
      return ui::DomKey::Key::ARROW_DOWN;
    case Qt::Key_Left:
      return ui::DomKey::Key::ARROW_LEFT;
    case Qt::Key_Right:
      return ui::DomKey::Key::ARROW_RIGHT;
    case Qt::Key_Up:
      return ui::DomKey::Key::ARROW_UP;
    case Qt::Key_End:
      return ui::DomKey::Key::END;
    case Qt::Key_Home:
      return ui::DomKey::Key::HOME;
    case Qt::Key_PageDown:
      return ui::DomKey::Key::PAGE_DOWN;
    case Qt::Key_PageUp:
      return ui::DomKey::Key::PAGE_UP;

    // ============
    // Editing keys
    // ============
    case Qt::Key_Clear:
      return ui::DomKey::Key::CLEAR;
    case Qt::Key_Copy:
      return ui::DomKey::Key::COPY;
    // CR_SEL
    case Qt::Key_Cut:
      return ui::DomKey::Key::CUT;
    // ERASE_EOF
    // EX_SEL
    case Qt::Key_Insert:
      return ui::DomKey::Key::INSERT;
    case Qt::Key_Paste:
      return ui::DomKey::Key::PASTE;
    case Qt::Key_Redo:
      return ui::DomKey::Key::REDO;
    case Qt::Key_Undo:
      return ui::DomKey::Key::UNDO;

    // =======
    // UI keys
    // =======
    // ACCEPT
    // AGAIN
    // ATTN
    case Qt::Key_Cancel:
      return ui::DomKey::Key::CANCEL;
    case Qt::Key_Menu:
      return ui::DomKey::Key::CONTEXT_MENU;
    case Qt::Key_Execute:
      return ui::DomKey::Key::EXECUTE;
    case Qt::Key_Find:
      return ui::DomKey::Key::FIND;
    case Qt::Key_Help:
      return ui::DomKey::Key::HELP;
    case Qt::Key_Pause:
      return ui::DomKey::Key::PAUSE;
    case Qt::Key_Play:
      return ui::DomKey::Key::PLAY;
    // PROPS
    case Qt::Key_Select:
      return ui::DomKey::Key::SELECT;
    case Qt::Key_ZoomIn:
      return ui::DomKey::Key::ZOOM_IN;
    case Qt::Key_ZoomOut:
      return ui::DomKey::Key::ZOOM_OUT;

    // ===========
    // Device keys
    // ===========
    case Qt::Key_MonBrightnessDown:
      return ui::DomKey::Key::BRIGHTNESS_DOWN;
    case Qt::Key_MonBrightnessUp:
      return ui::DomKey::Key::BRIGHTNESS_UP;
    case Qt::Key_Eject:
      return ui::DomKey::Key::EJECT;
    case Qt::Key_LogOff:
      return ui::DomKey::Key::LOG_OFF;
    // POWER
    case Qt::Key_PowerOff:
      return ui::DomKey::Key::POWER_OFF;
    // PRINT_SCREEN
    case Qt::Key_Hibernate:
      return ui::DomKey::Key::HIBERNATE;
    case Qt::Key_Standby:
      return ui::DomKey::Key::STANDBY;
    case Qt::Key_WakeUp:
      return ui::DomKey::Key::WAKE_UP;

    // ========================
    // IME and Composition keys
    // ========================
    // ALL_CANDIDATES
    // ALPHANUMERIC
    case Qt::Key_Codeinput:
      return ui::DomKey::Key::CODE_INPUT;
    // COMPOSE
    // CONVERT
    // FINAL_MODE
    // GROUP_FIRST
    // GROUP_LAST
    // GROUP_NEXT
    // GROUP_PREVIOUS
    case Qt::Key_Mode_switch:
      return ui::DomKey::Key::MODE_CHANGE;
    // NEXT_CANDIDATE
    // NON_CONVERT
    case Qt::Key_PreviousCandidate:
      return ui::DomKey::Key::PREVIOUS_CANDIDATE;
    // PROCESS
    case Qt::Key_SingleCandidate:
      return ui::DomKey::Key::SINGLE_CANDIDATE;
    case Qt::Key_Hangul:
      return ui::DomKey::Key::HANGUL_MODE;
    case Qt::Key_Hangul_Hanja:
      return ui::DomKey::Key::HANJA_MODE;
    // JUNJA_MODE
    case Qt::Key_Eisu_Shift:
    case Qt::Key_Eisu_toggle:
      return ui::DomKey::Key::EISU;
    case Qt::Key_Hankaku:
      return ui::DomKey::Key::HANKAKU;
    case Qt::Key_Hiragana:
      return ui::DomKey::Key::HIRAGANA;
    case Qt::Key_Hiragana_Katakana:
      return ui::DomKey::Key::HIRAGANA_KATAKANA;
    case Qt::Key_Kana_Lock:
    case Qt::Key_Kana_Shift:
      return ui::DomKey::Key::KANA_MODE;
    case Qt::Key_Kanji:
      return ui::DomKey::Key::KANJI_MODE;
    case Qt::Key_Katakana:
      return ui::DomKey::Key::KATAKANA;
    case Qt::Key_Romaji:
      return ui::DomKey::Key::ROMAJI;
    case Qt::Key_Zenkaku:
      return ui::DomKey::Key::ZENKAKU;
    case Qt::Key_Zenkaku_Hankaku:
      return ui::DomKey::Key::ZENKAKU_HANKAKU;

    // ===============
    // Multimedia keys
    // ===============
    case Qt::Key_ChannelDown:
      return ui::DomKey::Key::CHANNEL_DOWN;
    case Qt::Key_ChannelUp:
      return ui::DomKey::Key::CHANNEL_UP;
    case Qt::Key_Close:
      return ui::DomKey::Key::CLOSE;
    case Qt::Key_MailForward:
      return ui::DomKey::Key::MAIL_FORWARD;
    // MAIL_REPLY
    // MAIL_SEND
    // MEDIA_FAST_FORWARD
    case Qt::Key_MediaPause:
      return ui::DomKey::Key::MEDIA_PAUSE;
    case Qt::Key_MediaPlay:
      return ui::DomKey::Key::MEDIA_PLAY;
    case Qt::Key_MediaTogglePlayPause:
      return ui::DomKey::Key::MEDIA_PLAY_PAUSE;
    case Qt::Key_MediaRecord:
      return ui::DomKey::Key::MEDIA_RECORD;
    // MEDIA_REWIND
    case Qt::Key_MediaStop:
      return ui::DomKey::Key::MEDIA_STOP;
    case Qt::Key_MediaNext:
      return ui::DomKey::Key::MEDIA_TRACK_NEXT;
    case Qt::Key_MediaPrevious:
      return ui::DomKey::Key::MEDIA_TRACK_PREVIOUS;
    case Qt::Key_New:
      return ui::DomKey::Key::NEW;
    case Qt::Key_Open:
      return ui::DomKey::Key::OPEN;
    case Qt::Key_Print:
      return ui::DomKey::Key::PRINT;
    case Qt::Key_Save:
      return ui::DomKey::Key::SAVE;
    case Qt::Key_Spell:
      return ui::DomKey::Key::SPELL_CHECK;

    // ======================
    // Multimedia numpad keys
    // ======================
    // KEY11
    // KEY12

    // ==========
    // Audio keys
    // ==========
    // AUDIO_BALANCE_LEFT
    // AUDIO_BALANCE_RIGHT
    case Qt::Key_BassDown:
      return ui::DomKey::Key::AUDIO_BASS_DOWN;
    // AUDIO_BASS_BOOST_DOWN
    case Qt::Key_BassBoost:
      return ui::DomKey::Key::AUDIO_BASS_BOOST_TOGGLE;
    // AUDIO_BASS_BOOST_UP
    case Qt::Key_BassUp:
      return ui::DomKey::Key::AUDIO_BASS_UP;
    // AUDIO_FADER_FRONT
    // AUDIO_FADER_REAR
    // AUDIO_SURROUND_MODE_NEXT
    case Qt::Key_TrebleDown:
      return ui::DomKey::Key::AUDIO_TREBLE_DOWN;
    case Qt::Key_TrebleUp:
      return ui::DomKey::Key::AUDIO_TREBLE_UP;
    case Qt::Key_VolumeDown:
      return ui::DomKey::Key::AUDIO_VOLUME_DOWN;
    case Qt::Key_VolumeUp:
      return ui::DomKey::Key::AUDIO_VOLUME_UP;
    case Qt::Key_VolumeMute:
      return ui::DomKey::Key::AUDIO_VOLUME_MUTE;
    // MICROPHONE_TOGGLE
    case Qt::Key_MicVolumeDown:
      return ui::DomKey::Key::MICROPHONE_VOLUME_DOWN;
    case Qt::Key_MicVolumeUp:
      return ui::DomKey::Key::MICROPHONE_VOLUME_UP;
    case Qt::Key_MicMute:
      return ui::DomKey::Key::MICROPHONE_VOLUME_MUTE;

    // ===========
    // Speech keys
    // ===========
    // SPEECH_CORRECTION_LIST
    // SPEECH_INPUT_TOGGLE

    // ================
    // Application keys
    // ================
    case Qt::Key_Calculator:
      return ui::DomKey::Key::LAUNCH_CALCULATOR;
    case Qt::Key_Calendar:
      return ui::DomKey::Key::LAUNCH_CALENDAR;
    case Qt::Key_LaunchMail:
      return ui::DomKey::Key::LAUNCH_MAIL;
    case Qt::Key_LaunchMedia:
      return ui::DomKey::Key::LAUNCH_MEDIA_PLAYER;
    case Qt::Key_Music:
      return ui::DomKey::Key::LAUNCH_MUSIC_PLAYER;
    // LAUNCH_MY_COMPUTER
    case Qt::Key_Phone:
      return ui::DomKey::Key::LAUNCH_PHONE;
    case Qt::Key_ScreenSaver:
      return ui::DomKey::Key::LAUNCH_SCREEN_SAVER;
    // LAUNCH_SPREADSHEET
    // LAUNCH_WEB_BROWSER
    case Qt::Key_WebCam:
      return ui::DomKey::Key::LAUNCH_WEB_CAM;
    case Qt::Key_Word:
      return ui::DomKey::Key::LAUNCH_WORD_PROCESSOR;

    // ============
    // Browser keys
    // ============
    case Qt::Key_Back:
      return ui::DomKey::Key::BROWSER_BACK;
    case Qt::Key_Favorites:
      return ui::DomKey::Key::BROWSER_FAVORITES;
    case Qt::Key_Forward:
      return ui::DomKey::Key::BROWSER_FORWARD;
    case Qt::Key_HomePage:
      return ui::DomKey::Key::BROWSER_HOME;
    case Qt::Key_Refresh:
      return ui::DomKey::Key::BROWSER_REFRESH;
    case Qt::Key_Search:
      return ui::DomKey::Key::BROWSER_SEARCH;
    case Qt::Key_Stop:
      return ui::DomKey::Key::BROWSER_STOP;

    // =================
    // Mobile phone keys
    // =================
    // APP_SWITCH
    case Qt::Key_Call:
      return ui::DomKey::Key::CALL;
    case Qt::Key_Camera:
      return ui::DomKey::Key::CAMERA;
    case Qt::Key_CameraFocus:
      return ui::DomKey::Key::CAMERA_FOCUS;
    case Qt::Key_Hangup:
      return ui::DomKey::Key::END_CALL;
    // GO_BACK
    // GO_HOME
    // HEADSET_HOOK
    case Qt::Key_LastNumberRedial:
      return ui::DomKey::Key::LAST_NUMBER_REDIAL;
    // NOTIFICATION
    // MANNER_MODE
    case Qt::Key_VoiceDial:
      return ui::DomKey::Key::VOICE_DIAL;

    // =======
    // TV keys
    // =======
    // TODO

    // =====================
    // Media Controller keys
    // =====================
    // TODO

    default:
      return ui::DomKey::Key::UNIDENTIFIED;
  }
}

} // namespace qt
} // namespace oxide
