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
      'target_name': '<(oxide_core_name)',
      'type': 'shared_library',
      'shared_library_version': '<(oxide_core_so_version)',
      'dependencies': [
        'OxideQtCore_private',
        'OxideQtCore_public',
      ],
    },
    {
      'target_name': 'OxideQtCore_private',
      'type': 'static_library',
      'dependencies': [
        'system.gyp:Qt5Core',
        'system.gyp:Qt5Gui',
        'system.gyp:Qt5Gui-private',
        '../../shared/shared.gyp:oxide_shared',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_common',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/WebKit/public/blink.gyp:blink',
        '<(DEPTH)/ui/base/ui_base.gyp:ui_base',
        '<(DEPTH)/ui/events/events.gyp:events',
        '<(DEPTH)/ui/gfx/gfx.gyp:gfx',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/surface/surface.gyp:surface',
        '<(DEPTH)/url/url.gyp:url_lib'
      ],
      'variables': {
        'chromium_code': 1,
      },
      'include_dirs': [
        '../..',
        '<(DEPTH)'
      ],
      'sources': [
        '<(INTERMEDIATE_DIR)/oxide_qt_web_popup_menu_qquick.moc',
        'api/internal/oxideqwebpreferences_p.cc',
        'app/oxide_qt_content_main_delegate.cc',
        'app/oxide_qt_content_main_delegate.h',
        'browser/oxide_default_screen_info.cc',
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
        'browser/oxide_qt_web_popup_menu.cc',
        'browser/oxide_qt_web_popup_menu.h',
        'browser/oxide_qt_web_preferences.cc',
        'browser/oxide_qt_web_preferences.h',
        'browser/oxide_qt_web_view.cc',
        'browser/oxide_qt_web_view.h',
        'common/oxide_qt_content_client.cc',
        'common/oxide_qt_content_client.h',
        'gl/oxide_gl_implementation.cc',
        'gl/oxide_qt_shared_gl_context.cc',
        'gl/oxide_qt_shared_gl_context.h',
        'glue/private/oxide_qt_web_context_adapter_p.cc',
      ],
    },
    {
      'target_name': 'OxideQtCore_public',
      'type': 'static_library',
      'cflags_cc!': [ '-fno-rtti' ],
      'dependencies': [
        'system.gyp:Qt5Core',
        'system.gyp:Qt5Gui',
        '../../shared/shared.gyp:oxide_shared',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/ui/gfx/gfx.gyp:gfx',
        '<(DEPTH)/url/url.gyp:url_lib'
      ],
      'variables': {
        'chromium_code': 1,
      },
      'include_dirs': [
        '../..',
        '<(INTERMEDIATE_DIR)',
        '<(DEPTH)'
      ],
      'sources': [
        '<(INTERMEDIATE_DIR)/moc_oxideqloadevent.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqnetworkcallbackevents.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqstoragepermissionrequest.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqwebpreferences.cc',
        'api/oxideqloadevent.cc',
        'api/oxideqloadevent.h',
        'api/oxideqloadevent_p.h',
        'api/oxideqnetworkcallbackevents.cc',
        'api/oxideqnetworkcallbackevents.h',
        'api/oxideqnetworkcallbackevents_p.h',
        'api/oxideqstoragepermissionrequest.cc',
        'api/oxideqstoragepermissionrequest.h',
        'api/oxideqstoragepermissionrequest_p.h',
        'api/oxideqwebpreferences.cc',
        'api/oxideqwebpreferences.h',
        'api/oxideqwebpreferences_p.h',
        'glue/oxide_qt_adapter_base.h',
        'glue/oxide_qt_render_widget_host_view_delegate.cc',
        'glue/oxide_qt_render_widget_host_view_delegate.h',
        'glue/oxide_qt_render_widget_host_view_delegate_p.h',
        'glue/oxide_qt_script_message_adapter.cc',
        'glue/oxide_qt_script_message_adapter.h',
        'glue/oxide_qt_script_message_adapter_p.h',
        'glue/oxide_qt_script_message_handler_adapter.cc',
        'glue/oxide_qt_script_message_handler_adapter.h',
        'glue/oxide_qt_script_message_handler_adapter_p.h',
        'glue/oxide_qt_script_message_request_adapter.cc',
        'glue/oxide_qt_script_message_request_adapter.h',
        'glue/oxide_qt_script_message_request_adapter_p.h',
        'glue/oxide_qt_user_script_adapter.cc',
        'glue/oxide_qt_user_script_adapter.h',
        'glue/oxide_qt_user_script_adapter_p.h',
        'glue/oxide_qt_web_context_adapter.cc',
        'glue/oxide_qt_web_context_adapter.h',
        'glue/oxide_qt_web_context_adapter_p.h',
        'glue/oxide_qt_web_frame_adapter.cc',
        'glue/oxide_qt_web_frame_adapter.h',
        'glue/oxide_qt_web_frame_adapter_p.h',
        'glue/oxide_qt_web_popup_menu_delegate.cc',
        'glue/oxide_qt_web_popup_menu_delegate.h',
        'glue/oxide_qt_web_view_adapter.cc',
        'glue/oxide_qt_web_view_adapter.h',
      ],
      'actions': [
        {
          'action_name': 'moc_oxideqloadevent.cc',
          'moc_input': 'api/oxideqloadevent.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqnetworkcallbackevents.cc',
          'moc_input': 'api/oxideqnetworkcallbackevents.h',
          'includes': [ 'moc.gypi' ],
        },
        {
          'action_name': 'moc_oxideqstoragepermissionrequest.cc',
          'moc_input': 'api/oxideqstoragepermissionrequest.h',
          'includes': [ 'moc.gypi' ],
        },
        {
          'action_name': 'moc_oxideqwebpreferences.cc',
          'moc_input': 'api/oxideqwebpreferences.h',
          'includes': [ 'moc.gypi' ]
        },
      ]
    }
  ]
}
