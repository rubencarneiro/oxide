# Copyright (C) 2013 Canonical Ltd.

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

{
  'targets': [
    {
      'target_name': 'oxide-qt',
      'type': 'shared_library',
      'shared_library_version': '<(oxide_qt_libversion)',
      'dependencies': [
        'system.gyp:Qt5Core',
        'system.gyp:Qt5Gui',
        'system.gyp:Qt5Quick',
        '../../shared/shared.gyp:oxide_shared',
        'oxide-qt_public',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/url/url.gyp:url_lib'
      ],
      'include_dirs': [
        '../..',
        '<(INTERMEDIATE_DIR)',
        '<(DEPTH)'
      ],
      'sources': [
        '<(INTERMEDIATE_DIR)/oxide_qt_web_popup_menu_qquick.moc',
        'browser/oxide_qt_backing_store.cc',
        'browser/oxide_qt_backing_store.h',
        'browser/oxide_qt_content_browser_client.cc',
        'browser/oxide_qt_content_browser_client.h',
        'browser/oxide_qt_message_pump.cc',
        'browser/oxide_qt_message_pump.h',
        'browser/oxide_qt_render_widget_host_view_qquick.cc',
        'browser/oxide_qt_render_widget_host_view_qquick.h',
        'browser/oxide_qt_web_frame.cc',
        'browser/oxide_qt_web_frame.h',
        'browser/oxide_qt_web_popup_menu_qquick.cc',
        'browser/oxide_qt_web_popup_menu_qquick.h',
        'common/oxide_qt_content_main_delegate.cc',
        'common/oxide_qt_content_main_delegate.h',
        'api/oxide_q_incoming_message_p.cc',
        'api/oxide_q_incoming_message_p.h',
        'api/oxide_q_load_event_p.cc',
        'api/oxide_q_load_event_p.h',
        'api/oxide_qquick_web_view_p.cc',
        'api/oxide_qquick_web_view_p_p.h',
        'api/oxide_qt_qmessage_handler_p.cc',
        'api/oxide_qt_qmessage_handler_p.h',
        'api/oxide_qt_qoutgoing_message_request_p.cc',
        'api/oxide_qt_qoutgoing_message_request_p.h',
        'api/oxide_qt_qweb_frame_p.cc',
        'api/oxide_qt_qweb_frame_p.h',
        'api/oxide_qt_qweb_view_context_p.cc',
        'api/oxide_qt_qweb_view_context_p.h',
        'api/oxide_q_user_script_p.cc',
        'api/oxide_q_user_script_p.h'
      ],
      'actions': [
        {
          'action_name': 'oxide_qt_web_popup_menu_qquick.moc',
          'moc_input': 'browser/oxide_qt_web_popup_menu_qquick.cc',
          'includes': [ 'moc.gypi' ]
        },
      ],
      'variables': {
        'chromium_code': 1
      }
    },
    {
      'target_name': 'oxide-qt_public',
      'type': 'static_library',
      'cflags_cc!': [ '-fno-rtti' ],
      'dependencies': [
        'system.gyp:Qt5Core',
        'system.gyp:Qt5Gui',
        'system.gyp:Qt5Quick',
        '../../shared/shared.gyp:oxide_shared',
        '<(DEPTH)/skia/skia.gyp:skia',
      ],
      'export_dependent_settings': [
        '<(DEPTH)/skia/skia.gyp:skia'
      ],
      'include_dirs': [
        '../..',
        '<(DEPTH)'
      ],
      'sources': [
        '<(INTERMEDIATE_DIR)/moc_oxide_q_incoming_message.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_q_load_event.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_q_message_handler_base.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_q_outgoing_message_request_base.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_q_user_script.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_q_web_frame_base.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_q_web_view_context_base.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_qquick_message_handler.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_qquick_outgoing_message_request.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_qquick_web_frame.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_qquick_web_view.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_qquick_web_view_context.cc',
        'api/public/oxide_q_incoming_message.cc',
        'api/public/oxide_q_incoming_message.h',
        'api/public/oxide_q_load_event.cc',
        'api/public/oxide_q_load_event.h',
        'api/public/oxide_q_message_handler_base.h',
        'api/public/oxide_q_outgoing_message_request_base.h',
        'api/public/oxide_q_user_script.cc',
        'api/public/oxide_q_user_script.h',
        'api/public/oxide_q_web_frame_base.h',
        'api/public/oxide_q_web_view_context_base.h',
        'api/public/oxide_qquick_message_handler_p.h',
        'api/public/oxide_qquick_outgoing_message_request_p.h',
        'api/public/oxide_qquick_web_frame_p.h',
        'api/public/oxide_qquick_web_view.cc',
        'api/public/oxide_qquick_web_view_p.h',
        'api/public/oxide_qquick_web_view_context_p.h',
        'api/public/oxide_qt_qmessage_handler.cc',
        'api/public/oxide_qt_qoutgoing_message_request.cc',
        'api/public/oxide_qt_qweb_frame.cc',
        'api/public/oxide_qt_qweb_view_context.cc',
      ],
      'actions': [
        {
          'action_name': 'moc_oxide_q_incoming_message.cc',
          'moc_input': 'api/public/oxide_q_incoming_message.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_q_load_event.cc',
          'moc_input': 'api/public/oxide_q_load_event.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_q_message_handler_base.cc',
          'moc_input': 'api/public/oxide_q_message_handler_base.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_q_outgoing_message_request_base.cc',
          'moc_input': 'api/public/oxide_q_outgoing_message_request_base.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_q_user_script.cc',
          'moc_input': 'api/public/oxide_q_user_script.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_q_web_frame_base.cc',
          'moc_input': 'api/public/oxide_q_web_frame_base.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_q_web_view_context_base.cc',
          'moc_input': 'api/public/oxide_q_web_view_context_base.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_qquick_message_handler.cc',
          'moc_input': 'api/public/oxide_qquick_message_handler_p.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_qquick_outgoing_message_request.cc',
          'moc_input': 'api/public/oxide_qquick_outgoing_message_request_p.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_qquick_web_frame.cc',
          'moc_input': 'api/public/oxide_qquick_web_frame_p.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_qquick_web_view.cc',
          'moc_input': 'api/public/oxide_qquick_web_view_p.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_qquick_web_view_context.cc',
          'moc_input': 'api/public/oxide_qquick_web_view_context_p.h',
          'includes': [ 'moc.gypi' ]
        },
      ]
    }
  ]
}
