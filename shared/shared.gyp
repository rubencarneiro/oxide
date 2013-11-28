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
    'oxide_subprocess%': 'oxide-renderer',
    'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/oxide'
  },
  'targets': [
    {
      'target_name': 'oxide_shared_generated',
      'type': 'none',
      'all_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)/oxide'
        ]
      },
      'actions': [
        {
          'action_name': 'chrome_version_header',
          'inputs': [
            'common/chrome_version.h.in',
            '<(DEPTH)/chrome/VERSION'
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/oxide/shared/common/chrome_version.h'
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
      'target_name': 'oxide_shared_resources',
      'type': 'none',
      'actions': [
        {
          'action_name': 'oxide_shared_resources_resources',
          'variables': {
            'grit_grd_file': 'oxide_resources.grd'
          },
          'includes': [ '../chromium/src/build/grit_action.gypi' ]
        }
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)/oxide'
        ]
      },
    },
    {
      'target_name': 'oxide_packed_resources',
      'type': 'none',
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py'
      },
      'dependencies': [
        'oxide_shared_resources',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/ui/ui.gyp:ui_resources'
      ],
      'actions': [
        {
          'action_name': 'repack_oxide',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/oxide/oxide_resources.pak'
            ]
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)'
          ],
          'outputs': [
            '<(PRODUCT_DIR)/oxide.pak'
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)']
        },
        {
          'action_name': 'repack_oxide_100_percent',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_100_percent.pak'
            ]
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)'
          ],
          'outputs': [
            '<(PRODUCT_DIR)/oxide_100_percent.pak'
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)']
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
        'oxide_packed_resources',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/content/content.gyp:content_app_both',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_common',
        '<(DEPTH)/content/content.gyp:content_gpu',
        '<(DEPTH)/content/content.gyp:content_ppapi_plugin',
        '<(DEPTH)/content/content.gyp:content_plugin',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/content/content.gyp:content_utility',
        '<(DEPTH)/content/content.gyp:content_worker',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/WebKit/public/blink.gyp:blink',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/url/url.gyp:url_lib',
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
        'browser/oxide_browser_process_handle.cc',
        'browser/oxide_browser_process_handle.h',
        'browser/oxide_browser_process_main.cc',
        'browser/oxide_browser_process_main.h',
        'browser/oxide_content_browser_client.cc',
        'browser/oxide_content_browser_client.h',
        'browser/oxide_http_user_agent_settings.cc',
        'browser/oxide_http_user_agent_settings.h',
        'browser/oxide_incoming_message.cc',
        'browser/oxide_incoming_message.h',
        'browser/oxide_io_thread_delegate.cc',
        'browser/oxide_io_thread_delegate.h',
        'browser/oxide_message_handler.cc',
        'browser/oxide_message_handler.h',
        'browser/oxide_message_pump.cc',
        'browser/oxide_message_pump.h',
        'browser/oxide_message_target.h',
        'browser/oxide_network_delegate.cc',
        'browser/oxide_network_delegate.h',
        'browser/oxide_off_the_record_browser_context_impl.cc',
        'browser/oxide_off_the_record_browser_context_impl.h',
        'browser/oxide_outgoing_message_request.cc',
        'browser/oxide_outgoing_message_request.h',
        'browser/oxide_render_widget_host_view.cc',
        'browser/oxide_render_widget_host_view.h',
        'browser/oxide_shared_gl_context.cc',
        'browser/oxide_shared_gl_context.h',
        'browser/oxide_ssl_config_service.cc',
        'browser/oxide_ssl_config_service.h',
        'browser/oxide_url_request_context.cc',
        'browser/oxide_url_request_context.h',
        'browser/oxide_user_script_master.cc',
        'browser/oxide_user_script_master.h',
        'browser/oxide_web_contents_view.cc',
        'browser/oxide_web_contents_view.h',
        'browser/oxide_web_frame.cc',
        'browser/oxide_web_frame.h',
        'browser/oxide_web_popup_menu.cc',
        'browser/oxide_web_popup_menu.h',
        'browser/oxide_web_view.cc',
        'browser/oxide_web_view.h',
        'chromium_support/oxide_compositor_util.cc',
        'common/oxide_constants.cc',
        'common/oxide_constants.h',
        'common/oxide_content_client.cc',
        'common/oxide_content_client.h',
        'common/oxide_content_main_delegate.cc',
        'common/oxide_content_main_delegate.h',
        'common/oxide_core_export.h',
        'common/oxide_export.h',
        'common/oxide_file_utils.cc',
        'common/oxide_file_utils.h',
        'common/oxide_message_enums.h',
        'common/oxide_message_generator.cc',
        'common/oxide_message_generator.h',
        'common/oxide_messages.h',
        'common/oxide_user_script.cc',
        'common/oxide_user_script.h',
        'renderer/oxide_content_renderer_client.cc',
        'renderer/oxide_content_renderer_client.h',
        'renderer/oxide_isolated_world_map.cc',
        'renderer/oxide_isolated_world_map.h',
        'renderer/oxide_message_dispatcher_renderer.cc',
        'renderer/oxide_message_dispatcher_renderer.h',
        'renderer/oxide_process_observer.cc',
        'renderer/oxide_process_observer.h',
        'renderer/oxide_user_script_scheduler.cc',
        'renderer/oxide_user_script_scheduler.h',
        'renderer/oxide_user_script_slave.cc',
        'renderer/oxide_user_script_slave.h',
        'renderer/oxide_v8_message_manager.cc',
        'renderer/oxide_v8_message_manager.h',
        'renderer/oxide_v8_scoped_persistent.h',
        'renderer/oxide_web_frame_observer.cc',
        'renderer/oxide_web_frame_observer.h',
        '<(DEPTH)/extensions/common/constants.cc',
        '<(DEPTH)/extensions/common/constants.h',
        '<(DEPTH)/extensions/common/error_utils.cc',
        '<(DEPTH)/extensions/common/error_utils.h',
        '<(DEPTH)/extensions/common/url_pattern.cc',
        '<(DEPTH)/extensions/common/url_pattern.h',
        '<(DEPTH)/extensions/common/url_pattern_set.cc',
        '<(DEPTH)/extensions/common/url_pattern_set.h'
      ],
      'variables': {
        'chromium_code': 1
      }
    }
  ]
}
