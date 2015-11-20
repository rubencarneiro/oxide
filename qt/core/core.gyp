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
      'product_extension': 'so.<(oxide_core_so_version)',
      'defines': [
        'QT_NO_SIGNALS_SLOTS_KEYWORDS',
      ],
      'dependencies': [
        '../build/system.gyp:Qt5Core',
        '../build/system.gyp:Qt5Core-private',
        '../build/system.gyp:Qt5Gui',
        '../build/system.gyp:Qt5Gui-private',
        '../build/system.gyp:Qt5Positioning',
        '../build/system.gyp:Qt5Network',
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
        '<(DEPTH)/ui/gfx/gfx.gyp:gfx_geometry',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/surface/surface.gyp:surface',
        '<(DEPTH)/url/url.gyp:url_lib',
        'OxideQtCore_public',
      ],
      'variables': {
        'chromium_code': 1,
      },
      'include_dirs': [
        '../..',
        '<(INTERMEDIATE_DIR)',
        '<(DEPTH)'
      ],
      'ldflags': [
        '-Wl,-rpath=\$$ORIGIN/<(oxide_subprocess_dir)',
      ],
      'sources': [
        '<(INTERMEDIATE_DIR)/moc_oxide_qt_browser_platform_integration.cc',
        '<(INTERMEDIATE_DIR)/moc_oxide_qt_input_method_context.cc',
        'api/internal/oxideqmediacapturedevices_p.cc',
        'api/internal/oxideqwebpreferences_p.cc',
        'app/oxide_qt_main.cc',
        'app/oxide_qt_main.h',
        'app/oxide_qt_platform_delegate.cc',
        'app/oxide_qt_platform_delegate.h',
        'browser/input/oxide_qt_input_method_context.cc',
        'browser/input/oxide_qt_input_method_context.h',
        'browser/input/oxide_qt_input_method_context_client.h',
        'browser/oxide_qt_browser_platform_integration.cc',
        'browser/oxide_qt_browser_platform_integration.h',
        'browser/oxide_qt_browser_startup.cc',
        'browser/oxide_qt_browser_startup.h',
        'browser/oxide_qt_browser_thread_q_event_dispatcher.cc',
        'browser/oxide_qt_browser_thread_q_event_dispatcher.h',
        'browser/oxide_qt_clipboard.cc',
        'browser/oxide_qt_clipboard.h',
        'browser/oxide_qt_event_utils.cc',
        'browser/oxide_qt_event_utils.h',
        'browser/oxide_qt_file_picker.cc',
        'browser/oxide_qt_file_picker.h',
        'browser/oxide_qt_find_controller.cc',
        'browser/oxide_qt_find_controller.h',
        'browser/oxide_qt_javascript_dialog.cc',
        'browser/oxide_qt_javascript_dialog.h',
        'browser/oxide_qt_location_provider.cc',
        'browser/oxide_qt_location_provider.h',
        'browser/oxide_qt_message_pump.cc',
        'browser/oxide_qt_message_pump.h',
        'browser/oxide_qt_screen_utils.cc',
        'browser/oxide_qt_screen_utils.h',
        'browser/oxide_qt_script_message.cc',
        'browser/oxide_qt_script_message.h',
        'browser/oxide_qt_script_message_handler.cc',
        'browser/oxide_qt_script_message_handler.h',
        'browser/oxide_qt_script_message_request.cc',
        'browser/oxide_qt_script_message_request.h',
        'browser/oxide_qt_skutils.cc',
        'browser/oxide_qt_skutils.h',
        'browser/oxide_qt_url_request_delegated_job.cc',
        'browser/oxide_qt_url_request_delegated_job.h',
        'browser/oxide_qt_user_script.cc',
        'browser/oxide_qt_user_script.h',
        'browser/oxide_qt_variant_value_converter.cc',
        'browser/oxide_qt_variant_value_converter.h',
        'browser/oxide_qt_web_context.cc',
        'browser/oxide_qt_web_context.h',
        'browser/oxide_qt_web_context_menu.cc',
        'browser/oxide_qt_web_context_menu.h',
        'browser/oxide_qt_web_frame.cc',
        'browser/oxide_qt_web_frame.h',
        'browser/oxide_qt_web_popup_menu.cc',
        'browser/oxide_qt_web_popup_menu.h',
        'browser/oxide_qt_web_preferences.cc',
        'browser/oxide_qt_web_preferences.h',
        'browser/oxide_qt_web_view.cc',
        'browser/oxide_qt_web_view.h',
        'glue/oxide_qt_file_picker_proxy.h',
        'glue/oxide_qt_file_picker_proxy_client.h',
        'glue/oxide_qt_init.cc',
        'glue/oxide_qt_init.h',
        'glue/oxide_qt_javascript_dialog_proxy.h',
        'glue/oxide_qt_javascript_dialog_proxy_client.h',
        'glue/oxide_qt_proxy_handle.h',
        'glue/oxide_qt_script_message_handler_proxy.cc',
        'glue/oxide_qt_script_message_handler_proxy.h',
        'glue/oxide_qt_script_message_handler_proxy_client.h',
        'glue/oxide_qt_script_message_proxy.h',
        'glue/oxide_qt_script_message_request_proxy.cc',
        'glue/oxide_qt_script_message_request_proxy.h',
        'glue/oxide_qt_script_message_request_proxy_client.h',
        'glue/oxide_qt_user_script_proxy.cc',
        'glue/oxide_qt_user_script_proxy.h',
        'glue/oxide_qt_user_script_proxy_client.h',
        'glue/oxide_qt_web_context_menu_proxy.h',
        'glue/oxide_qt_web_context_menu_proxy_client.h',
        'glue/oxide_qt_web_context_proxy.cc',
        'glue/oxide_qt_web_context_proxy.h',
        'glue/oxide_qt_web_context_proxy_client.h',
        'glue/oxide_qt_web_frame_proxy.h',
        'glue/oxide_qt_web_frame_proxy_client.h',
        'glue/oxide_qt_web_popup_menu_proxy.h',
        'glue/oxide_qt_web_popup_menu_proxy_client.h',
        'glue/oxide_qt_web_view_proxy.cc',
        'glue/oxide_qt_web_view_proxy.h',
        'glue/oxide_qt_web_view_proxy_client.h',
        'gpu/oxide_qt_gl_context_dependent.cc',
        'gpu/oxide_qt_gl_context_dependent.h',
      ],
      'actions': [
        {
          'action_name': 'oxide_qt_clipboard.moc',
          'moc_input': 'browser/oxide_qt_clipboard.cc',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'oxide_qt_location_provider.moc',
          'moc_input': 'browser/oxide_qt_location_provider.cc',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'oxide_qt_url_request_delegated_job.moc',
          'moc_input': 'browser/oxide_qt_url_request_delegated_job.cc',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_qt_browser_platform_integration.cc',
          'moc_input': 'browser/oxide_qt_browser_platform_integration.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxide_qt_input_method_context.cc',
          'moc_input': 'browser/input/oxide_qt_input_method_context.h',
          'includes': [ 'moc.gypi' ]
        },
      ],
      'conditions': [
        ['component=="shared_library"', {
          'ldflags': [
            '-Wl,-rpath=\$$ORIGIN/<(oxide_subprocess_dir)/lib',
          ],
        }],
      ],
    },
    {
      'target_name': 'OxideQtCore_public',
      'type': 'static_library',
      'cflags_cc!': [ '-fno-rtti' ],
      'defines': [
        'QT_NO_SIGNALS_SLOTS_KEYWORDS',
      ],
      'dependencies': [
        '../build/system.gyp:Qt5Core',
        '../build/system.gyp:Qt5Gui',
        '../build/system.gyp:Qt5Gui-private',
        '../build/system.gyp:Qt5Network',
        '../../shared/shared.gyp:oxide_shared',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_common',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/ui/gfx/gfx.gyp:gfx',
        '<(DEPTH)/ui/gfx/gfx.gyp:gfx_geometry',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
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
        '<(INTERMEDIATE_DIR)/moc_oxideqcertificateerror.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqdownloadrequest.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqhttpauthenticationrequest.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqloadevent.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqmediacapturedevices.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqnetworkcallbackevents.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqnavigationrequest.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqnewviewrequest.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqpermissionrequest.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqfindcontroller.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqsecuritystatus.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqsslcertificate.cc',
        '<(INTERMEDIATE_DIR)/moc_oxideqwebpreferences.cc',
        'api/oxideqcertificateerror.cc',
        'api/oxideqcertificateerror.h',
        'api/oxideqcertificateerror_p.h',
        'api/oxideqdownloadrequest.cc',
        'api/oxideqdownloadrequest.h',
        'api/oxideqglobal.cc',
        'api/oxideqglobal.h',
        'api/oxideqhttpauthenticationrequest.cc',
        'api/oxideqhttpauthenticationrequest.h',
        'api/oxideqhttpauthenticationrequest_p.h',
        'api/oxideqloadevent.cc',
        'api/oxideqloadevent.h',
        'api/oxideqloadevent_p.h',
        'api/oxideqmediacapturedevices.cc',
        'api/oxideqmediacapturedevices.h',
        'api/oxideqmediacapturedevices_p.h',
        'api/oxideqnetworkcallbackevents.cc',
        'api/oxideqnetworkcallbackevents.h',
        'api/oxideqnetworkcallbackevents_p.h',
        'api/oxideqnavigationrequest.cc',
        'api/oxideqnavigationrequest.h',
        'api/oxideqnewviewrequest.cc',
        'api/oxideqnewviewrequest.h',
        'api/oxideqnewviewrequest_p.h',
        'api/oxideqpermissionrequest.cc',
        'api/oxideqpermissionrequest.h',
        'api/oxideqpermissionrequest_p.h',
        'api/oxideqfindcontroller.cc',
        'api/oxideqfindcontroller.h',
        'api/oxideqfindcontroller_p.h',
        'api/oxideqsecuritystatus.cc',
        'api/oxideqsecuritystatus.h',
        'api/oxideqsecuritystatus_p.h',
        'api/oxideqsslcertificate.cc',
        'api/oxideqsslcertificate.h',
        'api/oxideqsslcertificate_p.h',
        'api/oxideqwebpreferences.cc',
        'api/oxideqwebpreferences.h',
        'api/oxideqwebpreferences_p.h',
      ],
      'actions': [
        {
          'action_name': 'moc_oxideqcertificateerror.cc',
          'moc_input': 'api/oxideqcertificateerror.h',
          'includes': [ 'moc.gypi' ],
        },
        {
          'action_name': 'moc_oxideqdownloadrequest.cc',
          'moc_input': 'api/oxideqdownloadrequest.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqhttpauthenticationrequest.cc',
          'moc_input': 'api/oxideqhttpauthenticationrequest.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqloadevent.cc',
          'moc_input': 'api/oxideqloadevent.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqmediacapturedevices.cc',
          'moc_input': 'api/oxideqmediacapturedevices.h',
          'includes': [ 'moc.gypi' ]
        },
        {
          'action_name': 'moc_oxideqnetworkcallbackevents.cc',
          'moc_input': 'api/oxideqnetworkcallbackevents.h',
          'includes': [ 'moc.gypi' ],
        },
        {
          'action_name': 'moc_oxideqnavigationrequest.cc',
          'moc_input': 'api/oxideqnavigationrequest.h',
          'includes': [ 'moc.gypi' ],
        },
        {
          'action_name': 'moc_oxideqnewviewrequest.cc',
          'moc_input': 'api/oxideqnewviewrequest.h',
          'includes': [ 'moc.gypi' ],
        },
        {
          'action_name': 'moc_oxideqpermissionrequest.cc',
          'moc_input': 'api/oxideqpermissionrequest.h',
          'includes': [ 'moc.gypi' ],
        },
        {
          'action_name': 'moc_oxideqfindcontroller.cc',
          'moc_input': 'api/oxideqfindcontroller.h',
          'includes': [ 'moc.gypi' ],
        },
        {
          'action_name': 'moc_oxideqsecuritystatus.cc',
          'moc_input': 'api/oxideqsecuritystatus.h',
          'includes': [ 'moc.gypi' ],
        },
        {
          'action_name': 'moc_oxideqsslcertificate.cc',
          'moc_input': 'api/oxideqsslcertificate.h',
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
