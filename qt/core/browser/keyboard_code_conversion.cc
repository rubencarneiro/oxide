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

ui::KeyboardCode GetKeyboardCodeFromQKeyEvent(QKeyEvent* event) {
  int qkeycode = event->key();

  // 1) Keypad
  if (event->modifiers() & Qt::KeypadModifier) {
    if (qkeycode >= Qt::Key_0 && qkeycode <= Qt::Key_9) {
      return static_cast<ui::KeyboardCode>(
          (qkeycode - Qt::Key_0) + ui::VKEY_NUMPAD0);
    }

    switch (qkeycode) {
      case Qt::Key_Enter:
      case Qt::Key_Return:
        return ui::VKEY_RETURN;
      case Qt::Key_PageUp:
        return ui::VKEY_PRIOR;
      case Qt::Key_PageDown:
        return ui::VKEY_NEXT;
      case Qt::Key_End:
        return ui::VKEY_END;
      case Qt::Key_Home:
        return ui::VKEY_HOME;
      case Qt::Key_Left:
        return ui::VKEY_LEFT;
      case Qt::Key_Up:
        return ui::VKEY_UP;
      case Qt::Key_Right:
        return ui::VKEY_RIGHT;
      case Qt::Key_Down:
        return ui::VKEY_DOWN;
      case Qt::Key_Asterisk:
        return ui::VKEY_MULTIPLY;
      case Qt::Key_Plus:
        return ui::VKEY_ADD;
      case Qt::Key_Minus:
        return ui::VKEY_SUBTRACT;
      case Qt::Key_Period:
        return ui::VKEY_DECIMAL;
      case Qt::Key_Slash:
        return ui::VKEY_DIVIDE;
      case Qt::Key_Insert:
        return ui::VKEY_INSERT;
      case Qt::Key_Delete:
        return ui::VKEY_DELETE;
      default:
        return ui::VKEY_UNKNOWN;
    }
  }

  // 2) VKEY_A - VKEY_Z
  if (qkeycode >= Qt::Key_A && qkeycode <= Qt::Key_Z) {
    return static_cast<ui::KeyboardCode>((qkeycode - Qt::Key_A) + ui::VKEY_A);
  }

  // 3) VKEY_0 - VKEY_9
  if (qkeycode >= Qt::Key_0 && qkeycode <= Qt::Key_9) {
    return static_cast<ui::KeyboardCode>((qkeycode - Qt::Key_0) + ui::VKEY_0);
  }

  switch (qkeycode) {
    case Qt::Key_ParenRight:
      return ui::VKEY_0;
    case Qt::Key_Exclam:
      return ui::VKEY_1;
    case Qt::Key_At:
      return ui::VKEY_2;
    case Qt::Key_NumberSign:
      return ui::VKEY_3;
    case Qt::Key_Dollar:
      return ui::VKEY_4;
    case Qt::Key_Percent:
      return ui::VKEY_5;
    case Qt::Key_AsciiCircum:
      return ui::VKEY_6;
    case Qt::Key_Ampersand:
      return ui::VKEY_7;
    case Qt::Key_Asterisk:
      return ui::VKEY_8;
    case Qt::Key_ParenLeft:
      return ui::VKEY_9;
    default:
      break;
  }

  // 4) VKEY_F1 - VKEY_F24
  if (qkeycode >= Qt::Key_F1 && qkeycode <= Qt::Key_F24) {
    // We miss Qt::Key_F25 - Qt::Key_F35
    return static_cast<ui::KeyboardCode>((qkeycode - Qt::Key_F1) + ui::VKEY_F1);
  }

  switch (qkeycode) {
    case Qt::Key_Cancel:
      return ui::VKEY_CANCEL;
    case Qt::Key_Backspace:
      return ui::VKEY_BACK;
    case Qt::Key_Tab:
      return ui::VKEY_TAB;
    case Qt::Key_Backtab:
      return ui::VKEY_BACKTAB;
    case Qt::Key_Clear:
      return ui::VKEY_CLEAR;
    case Qt::Key_Return:
    case Qt::Key_Enter:
      return ui::VKEY_RETURN;
    case Qt::Key_Shift:
      return ui::VKEY_SHIFT;
    case Qt::Key_Control:
      return ui::VKEY_CONTROL;
    case Qt::Key_Alt:
      return ui::VKEY_MENU;
    case Qt::Key_Pause:
      return ui::VKEY_PAUSE;
    case Qt::Key_CapsLock:
      return ui::VKEY_CAPITAL;
    case Qt::Key_Kana_Lock:
    case Qt::Key_Kana_Shift:
      return ui::VKEY_KANA;
    case Qt::Key_Hangul:
      return ui::VKEY_HANGUL;
    // VKEY_JUNJA
    // VKEY_FINAL
    case Qt::Key_Hangul_Hanja:
      return ui::VKEY_HANJA;
    case Qt::Key_Kanji:
      return ui::VKEY_KANJI;
    case Qt::Key_Escape:
      return ui::VKEY_ESCAPE;
    case Qt::Key_Henkan:
      return ui::VKEY_CONVERT;
    case Qt::Key_Muhenkan:
      return ui::VKEY_NONCONVERT;
    // VKEY_ACCEPT
    // VKEY_MODECHANGE
    case Qt::Key_Space:
      return ui::VKEY_SPACE;
    case Qt::Key_PageUp:
      return ui::VKEY_PRIOR;
    case Qt::Key_PageDown:
      return ui::VKEY_NEXT;
    case Qt::Key_End:
      return ui::VKEY_END;
    case Qt::Key_Home:
      return ui::VKEY_HOME;
    case Qt::Key_Left:
      return ui::VKEY_LEFT;
    case Qt::Key_Up:
      return ui::VKEY_UP;
    case Qt::Key_Right:
      return ui::VKEY_RIGHT;
    case Qt::Key_Down:
      return ui::VKEY_DOWN;
    case Qt::Key_Select:
      return ui::VKEY_SELECT;
    case Qt::Key_Print:
      return ui::VKEY_PRINT;
    case Qt::Key_Execute:
      return ui::VKEY_EXECUTE;
    // VKEY_SNAPSHOT
    case Qt::Key_Insert:
      return ui::VKEY_INSERT;
    case Qt::Key_Delete:
      return ui::VKEY_DELETE;
    case Qt::Key_Help:
      return ui::VKEY_HELP;
    // VKEY_0 - VKEY_Z handled above
    case Qt::Key_Super_L:
      return ui::VKEY_LWIN;
    case Qt::Key_Super_R:
      return ui::VKEY_RWIN;
    case Qt::Key_Menu:
      return ui::VKEY_APPS;
    case Qt::Key_Sleep:
      return ui::VKEY_SLEEP;
    // VKEY_NUMPAD0 - VKEY_DIVIDE handled in keypad section
    // VKEY_F1 - VKEY_F24 handled above
    case Qt::Key_NumLock:
      return ui::VKEY_NUMLOCK;
    case Qt::Key_ScrollLock:
      return ui::VKEY_SCROLL;
    // VKEY_LSHIFT
    // VKEY_RSHIFT
    // VKEY_LCONTROL
    // VKEY_RCONTROL
    // VKEY_LMENU;
    // VKEY_RMENU
    case Qt::Key_Back:
      return ui::VKEY_BROWSER_BACK;
    case Qt::Key_Forward:
      return ui::VKEY_BROWSER_FORWARD;
    case Qt::Key_Refresh:
      return ui::VKEY_BROWSER_REFRESH;
    case Qt::Key_Stop:
      return ui::VKEY_BROWSER_STOP;
    case Qt::Key_Search:
      return ui::VKEY_BROWSER_SEARCH;
    case Qt::Key_Favorites:
      return ui::VKEY_BROWSER_FAVORITES;
    case Qt::Key_HomePage:
      return ui::VKEY_BROWSER_HOME;
    case Qt::Key_VolumeMute:
      return ui::VKEY_VOLUME_MUTE;
    case Qt::Key_VolumeDown:
      return ui::VKEY_VOLUME_DOWN;
    case Qt::Key_VolumeUp:
      return ui::VKEY_VOLUME_UP;
    case Qt::Key_MediaNext:
      return ui::VKEY_MEDIA_NEXT_TRACK;
    case Qt::Key_MediaPrevious:
      return ui::VKEY_MEDIA_PREV_TRACK;
    case Qt::Key_MediaStop:
      return ui::VKEY_MEDIA_STOP;
    case Qt::Key_MediaPlay:
    case Qt::Key_MediaPause:
    case Qt::Key_MediaTogglePlayPause:
      return ui::VKEY_MEDIA_PLAY_PAUSE;
    case Qt::Key_LaunchMail:
      return ui::VKEY_MEDIA_LAUNCH_MAIL;
    case Qt::Key_LaunchMedia:
      return ui::VKEY_MEDIA_LAUNCH_MEDIA_SELECT;
    case Qt::Key_Launch0:
      return ui::VKEY_MEDIA_LAUNCH_APP1;
    case Qt::Key_Launch1:
      return ui::VKEY_MEDIA_LAUNCH_APP2;
    case Qt::Key_Colon:
    case Qt::Key_Semicolon:
      return ui::VKEY_OEM_1;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
      return ui::VKEY_OEM_PLUS;
    case Qt::Key_Comma:
    case Qt::Key_Less:
      return ui::VKEY_OEM_COMMA;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
      return ui::VKEY_OEM_MINUS;
    case Qt::Key_Period:
    case Qt::Key_Greater:
      return ui::VKEY_OEM_PERIOD;
    case Qt::Key_Slash:
    case Qt::Key_Question:
      return ui::VKEY_OEM_2;
    case Qt::Key_QuoteLeft:
    case Qt::Key_AsciiTilde:
      return ui::VKEY_OEM_3;
    case Qt::Key_BracketLeft:
    case Qt::Key_BraceLeft:
      return ui::VKEY_OEM_4;
    case Qt::Key_Backslash:
    case Qt::Key_Bar:
      return ui::VKEY_OEM_5;
    case Qt::Key_BracketRight:
    case Qt::Key_BraceRight:
      return ui::VKEY_OEM_6;
    case Qt::Key_QuoteDbl:
    case Qt::Key_Apostrophe:
      return ui::VKEY_OEM_7;
    case Qt::Key_Exclam:
    case Qt::Key_section:
      return ui::VKEY_OEM_8;
    case Qt::Key_Ugrave:
    case Qt::Key_brokenbar:
      return ui::VKEY_OEM_102;
    // VKEY_OEM_103
    // VKEY_OEM_104
    // VKEY_PROCESSKEY
    // VKEY_PACKET
    // VKEY_OEM_ATTN
    // VKEY_OEM_FINISH
    // VKEY_OEM_COPY
    // VKEY_DBE_SBCSCHAR
    case Qt::Key_Zenkaku_Hankaku:
      return ui::VKEY_DBE_DBCSCHAR;
    // VKEY_OEM_BACKTAB
    // VKEY_ATTN
    // VKEY_CRSEL
    // VKEY_EXSEL
    // VKEY_EREOF
    // VKEY_PLAY
    // VKEY_ZOOM
    // VKEY_NONAME
    // VKEY_PA1
    // VKEY_OEM_CLEAR
    case Qt::Key_WLAN:
      return ui::VKEY_WLAN;
    case Qt::Key_PowerOff:
      return ui::VKEY_POWER;
    case Qt::Key_MonBrightnessDown:
      return ui::VKEY_BRIGHTNESS_DOWN;
    case Qt::Key_MonBrightnessUp:
      return ui::VKEY_BRIGHTNESS_UP;
    case Qt::Key_KeyboardBrightnessDown:
      return ui::VKEY_KBD_BRIGHTNESS_DOWN;
    case Qt::Key_KeyboardBrightnessUp:
      return ui::VKEY_KBD_BRIGHTNESS_UP;
    case Qt::Key_AltGr:
      return ui::VKEY_ALTGR;
    default:
      break;
  }

  return ui::VKEY_UNKNOWN;
}

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
