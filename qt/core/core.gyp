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
        'api/private/oxideqincomingmessage_p.cc',
        'api/private/oxideqincomingmessage_p.h',
        'api/private/oxideqloadevent_p.cc',
        'api/private/oxideqloadevent_p.h',
        'browser/oxide_qt_backing_store.cc',
        'browser/oxide_qt_backing_store.h',
        'browser/oxide_qt_content_browser_client.cc',
        'browser/oxide_qt_content_browser_client.h',
        'browser/oxide_qt_message_pump.cc',
        'browser/oxide_qt_message_pump.h',
        'browser/oxide_qt_render_widget_host_view.cc',
        'browser/oxide_qt_render_widget_host_view.h',
        'browser/oxide_qt_web_frame.cc',
        'browser/oxide_qt_web_frame.h',
        'browser/oxide_qt_web_frame_tree.cc',
        'browser/oxide_qt_web_frame_tree.h',
        'browser/oxide_qt_web_popup_menu_qquick.cc',
        'browser/oxide_qt_web_popup_menu_qquick.h',
        'common/oxide_qt_content_main_delegate.cc',
        'common/oxide_qt_content_main_delegate.h',
        'glue/private/oxide_qt_message_handler_adapter_p.cc',
        'glue/private/oxide_qt_message_handler_adapter_p.h',
        'glue/private/oxide_qt_outgoing_message_request_adapter_p.cc',
        'glue/private/oxide_qt_outgoing_message_request_adapter_p.h',
        'glue/private/oxide_qt_user_script_adapter_p.cc',
        'glue/private/oxide_qt_user_script_adapter_p.h',
        'glue/private/oxide_qt_web_context_adapter_p.cc',
        'glue/private/oxide_qt_web_context_adapter_p.h',
        'glue/private/oxide_qt_web_frame_adapter_p.cc',
        'glue/private/oxide_qt_web_frame_adapter_p.h',
        'glue/private/oxide_qt_web_view_adapter_p.cc',
        'glue/private/oxide_qt_web_view_adapter_p.h'
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
        '<(INTERMEDIATE_DIR)/moc_oxideqincomingmessage.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqloadevent.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqquickmessagehandler.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqquickoutgoingmessagerequest.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqquickuserscript.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqquickwebcontext.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqquickwebframe.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqquickwebview.cc',
        'api/oxideqincomingmessage.cc',
        'api/oxideqincomingmessage.h',
        'api/oxideqloadevent.cc',
        'api/oxideqloadevent.h',
        'glue/oxide_qt_adapter_base.h',
        'glue/oxide_qt_message_handler_adapter.cc',
        'glue/oxide_qt_message_handler_adapter.h',
        'glue/oxide_qt_outgoing_message_request_adapter.cc',
        'glue/oxide_qt_outgoing_message_request_adapter.h',
        'glue/oxide_qt_render_widget_host_view_delegate.cc',
        'glue/oxide_qt_render_widget_host_view_delegate.h',
        'glue/oxide_qt_user_script_adapter.cc',
        'glue/oxide_qt_user_script_adapter.h',
        'glue/oxide_qt_web_context_adapter.cc',
        'glue/oxide_qt_web_context_adapter.h',
        'glue/oxide_qt_web_frame_adapter.cc',
        'glue/oxide_qt_web_frame_adapter.h',
        'glue/oxide_qt_web_frame_tree_delegate.h',
        'glue/oxide_qt_web_view_adapter.cc',
        'glue/oxide_qt_web_view_adapter.h',
        '../quick/api/oxideqquickmessagehandler.cc',
        '../quick/api/oxideqquickmessagehandler_p.h',
        '../quick/api/oxideqquickmessagehandler_p_p.h',
        '../quick/api/oxideqquickoutgoingmessagerequest.cc',
        '../quick/api/oxideqquickoutgoingmessagerequest_p.h',
        '../quick/api/oxideqquickoutgoingmessagerequest_p_p.h',
        '../quick/api/oxideqquickuserscript.cc',
        '../quick/api/oxideqquickuserscript_p.h',
        '../quick/api/oxideqquickuserscript_p_p.h',
        '../quick/api/oxideqquickwebcontext.cc',
        '../quick/api/oxideqquickwebcontext_p.h',
        '../quick/api/oxideqquickwebcontext_p_p.h',
        '../quick/api/oxideqquickwebframe.cc',
        '../quick/api/oxideqquickwebframe_p.h',
        '../quick/api/oxideqquickwebframe_p_p.h',
        '../quick/api/oxideqquickwebview.cc',
        '../quick/api/oxideqquickwebview_p.h',
        '../quick/api/oxideqquickwebview_p_p.h',
        '../quick/oxide_qquick_render_widget_host_view_delegate.cc',
        '../quick/oxide_qquick_render_widget_host_view_delegate.h',
        '../quick/oxide_qquick_web_frame_tree_delegate.cc',
        '../quick/oxide_qquick_web_frame_tree_delegate.h',
      ],
      'actions': [
        {
          'action_name': 'moc_oxideqincomingmessage.cc',
          'moc_input': 'api/oxideqincomingmessage.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqloadevent.cc',
          'moc_input': 'api/oxideqloadevent.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqquickmessagehandler.cc',
          'moc_input': '../quick/api/oxideqquickmessagehandler_p.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqquickoutgoingmessagerequest.cc',
          'moc_input': '../quick/api/oxideqquickoutgoingmessagerequest_p.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqquickuserscript.cc',
          'moc_input': '../quick/api/oxideqquickuserscript_p.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqquickwebcontext.cc',
          'moc_input': '../quick/api/oxideqquickwebcontext_p.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqquickwebframe.cc',
          'moc_input': '../quick/api/oxideqquickwebframe_p.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqquickwebview.cc',
          'moc_input': '../quick/api/oxideqquickwebview_p.h',
          'includes': [ 'moc.gypi' ]
        },
      ]
    }
  ]
}
