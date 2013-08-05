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
  'variables': {
    'oxide_subprocess%': 'oxide-renderer'
  },
  'targets': [
    {
      'target_name': 'oxide_shared_generated',
      'type': 'none',
      'all_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)'
        ]
      },
      'actions': [
        {
          'action_name': 'chrome_version_header',
          'inputs': [
            'common/chrome_version.h.in'
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/shared/common/chrome_version.h'
          ],
          'action': [
            'python',
            '<(DEPTH)/chrome/tools/build/version.py',
            '-f', '<(DEPTH)/chrome/VERSION',
            '-i', '<@(_inputs)',
            '-o', '<@(_outputs)'
          ]
        }
      ]
    },
    {
      'target_name': 'oxide_shared',
      'type': '<(component)',
      'all_dependent_settings': {
        'include_dirs': [
          '..',
          '<(DEPTH)'
        ],
        'variables': {
          'chromium_code': 1
        }
      },
      'defines': [
        'OXIDE_RESOURCE_SUBPATH=\"<(oxide_port_resource_subpath)\"',
        'OXIDE_SUBPROCESS=\"<(oxide_subprocess)\"'
      ],
      'dependencies': [
        'oxide_shared_generated',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/content/content.gyp:content_app',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_common',
        '<(DEPTH)/content/content.gyp:content_ppapi_plugin',
        '<(DEPTH)/content/content.gyp:content_plugin',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/content/content.gyp:content_worker',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/webkit/common/user_agent/webkit_user_agent.gyp:user_agent'
      ],
      'include_dirs': [
        '..',
        '<(DEPTH)'
      ],
      'sources': [
        'app/oxide_main.cc',
        'app/oxide_main.h',
        'browser/oxide_browser_context.cc',
        'browser/oxide_browser_context.h',
        'browser/oxide_browser_context_impl.cc',
        'browser/oxide_browser_context_impl.h',
        'browser/oxide_browser_main_parts.cc',
        'browser/oxide_browser_main_parts.h',
        'browser/oxide_browser_process_handle.cc',
        'browser/oxide_browser_process_handle.h',
        'browser/oxide_browser_process_main.cc',
        'browser/oxide_browser_process_main.h',
        'browser/oxide_content_browser_client.cc',
        'browser/oxide_content_browser_client.h',
        'browser/oxide_global_settings.cc',
        'browser/oxide_global_settings.h',
        'browser/oxide_http_user_agent_settings.cc',
        'browser/oxide_http_user_agent_settings.h',
        'browser/oxide_io_thread_delegate.cc',
        'browser/oxide_io_thread_delegate.h',
        'browser/oxide_message_pump.cc',
        'browser/oxide_message_pump.h',
        'browser/oxide_network_delegate.cc',
        'browser/oxide_network_delegate.h',
        'browser/oxide_off_the_record_browser_context_impl.cc',
        'browser/oxide_off_the_record_browser_context_impl.h',
        'browser/oxide_render_widget_host_view.cc',
        'browser/oxide_render_widget_host_view.h',
        'browser/oxide_script_executor_host.cc',
        'browser/oxide_script_executor_host.h',
        'browser/oxide_ssl_config_service.cc',
        'browser/oxide_ssl_config_service.h',
        'browser/oxide_url_request_context.cc',
        'browser/oxide_url_request_context.h',
        'browser/oxide_web_contents_view.cc',
        'browser/oxide_web_contents_view.h',
        'browser/oxide_web_contents_view_delegate.h',
        'browser/oxide_web_popup_menu.cc',
        'browser/oxide_web_popup_menu.h',
        'browser/oxide_web_view.cc',
        'browser/oxide_web_view.h',
        'common/oxide_constants.cc',
        'common/oxide_constants.h',
        'common/oxide_content_client.cc',
        'common/oxide_content_client.h',
        'common/oxide_content_main_delegate.cc',
        'common/oxide_content_main_delegate.h',
        'common/oxide_core_export.h',
        'common/oxide_export.h',
        'common/oxide_message_generator.cc',
        'common/oxide_message_generator.h',
        'common/oxide_messages.h',
        'common/oxide_user_script.h',
        'renderer/oxide_content_renderer_client.cc',
        'renderer/oxide_content_renderer_client.h',
        'renderer/oxide_script_executor.cc',
        'renderer/oxide_script_executor.h'
      ],
      'variables': {
        'chromium_code': 1
      }
    }
  ]
}
