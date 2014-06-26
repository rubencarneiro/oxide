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
    'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/oxide',
  },
  'targets': [
    {
      'target_name': 'oxide_extra_resources',
      'type': 'none',
      'actions': [
        {
          'action_name': 'oxide_resources',
          'variables': {
            'grit_grd_file': 'oxide_resources.grd'
          },
          'includes': [ '../third_party/chromium/src/build/grit_action.gypi' ]
        }
      ],
      'includes': [ '../third_party/chromium/src/build/grit_target.gypi' ],
    },
    {
      'target_name': 'oxide_packed_resources',
      'type': 'none',
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py'
      },
      'dependencies': [
        'oxide_extra_resources',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
        '<(DEPTH)/net/net.gyp:net_resources',
        '<(DEPTH)/ui/resources/ui_resources.gyp:ui_resources',
        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_resources',
        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_strings',
      ],
      'actions': [
        {
          'action_name': 'repack_oxide',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/oxide/oxide_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/blink_resources.pak',
            ]
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)'
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/repack/oxide.pak'
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)']
        },
        {
          'action_name': 'repack_oxide_100_percent',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources_100_percent.pak',
            ]
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)'
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/repack/oxide_100_percent.pak'
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)']
        },
        {
          'action_name': 'repack_oxide_200_percent',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_200_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources_200_percent.pak',
            ]
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)'
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/repack/oxide_200_percent.pak'
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)']
        },
        {
          'action_name': 'repack_locales',
          'variables': {
            'pak_locales': '<(locales)',
            'repack_locales_path': '../build/scripts/repack_locales.py',
          },
          'inputs': [
            '<(repack_locales_path)',
            '<!@pymod_do_main(repack_locales -i -p <(OS) -b chromium -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(pak_locales))'
          ],
          'outputs': [
            '<!@pymod_do_main(repack_locales -o -p <(OS) -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(pak_locales))'
          ],
          'action': [
            'python',
            '<(repack_locales_path)',
            '-b', 'chromium',
            '-p', '<(OS)',
            '-g', '<(grit_out_dir)',
            '-s', '<(SHARED_INTERMEDIATE_DIR)',
            '-x', '<(SHARED_INTERMEDIATE_DIR)/.',
            '<@(pak_locales)',
          ],
        },
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/repack/oxide.pak'
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/repack/oxide_100_percent.pak'
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/repack/oxide_200_percent.pak'
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/locales',
          'files': [
            '<!@pymod_do_main(repack_locales -o -p <(OS) -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(locales))'
          ],
        },
      ],
    },
    {
      'target_name': 'oxide_shared',
      'type': 'static_library',
      'hard_dependency': 1,
      'variables': {
        'chromium_code': 1,
        'oxide_subprocess_path%': 'oxide-renderer',
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)/oxide',
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/ui/accessibility/accessibility.gyp:accessibility',
      ],
      'defines': [
        'OXIDE_SUBPROCESS_PATH=\"<(oxide_subprocess_path)\"'
      ],
      'dependencies': [
        'oxide_packed_resources',
        'oxide_extra_resources',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/build/linux/system.gyp:dbus',
        '<(DEPTH)/build/linux/system.gyp:fontconfig',
        '<(DEPTH)/cc/cc.gyp:cc',
        '<(DEPTH)/content/content.gyp:content_app_both',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_child',
        '<(DEPTH)/content/content.gyp:content_common',
        '<(DEPTH)/content/content.gyp:content_gpu',
        '<(DEPTH)/content/content.gyp:content_ppapi_plugin',
        '<(DEPTH)/content/content.gyp:content_plugin',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/content/content.gyp:content_utility',
        '<(DEPTH)/content/content.gyp:content_worker',
        '<(DEPTH)/gpu/gpu.gyp:command_buffer_common',
        '<(DEPTH)/gpu/gpu.gyp:gpu_config',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/WebKit/public/blink.gyp:blink',
        # Not used directly. Should content_browser have this in export_dependent_settings?
        '<(DEPTH)/ui/accessibility/accessibility.gyp:accessibility',
        '<(DEPTH)/ui/base/ui_base.gyp:ui_base',
        '<(DEPTH)/ui/events/events.gyp:events',
        '<(DEPTH)/ui/gfx/gfx.gyp:gfx',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/native_theme/native_theme.gyp:native_theme',
        '<(DEPTH)/ui/ozone/ozone.gyp:ozone',
        '<(DEPTH)/url/url.gyp:url_lib',
      ],
      'include_dirs': [
        '..',
        '<(DEPTH)',
        '<(SHARED_INTERMEDIATE_DIR)/oxide',
      ],
      'sources': [
        'app/oxide_content_main_delegate.cc',
        'app/oxide_content_main_delegate.h',
        'browser/compositor/oxide_compositor.cc',
        'browser/compositor/oxide_compositor.h',
        'browser/compositor/oxide_compositor_client.h',
        'browser/compositor/oxide_compositor_frame_handle.cc',
        'browser/compositor/oxide_compositor_frame_handle.h',
        'browser/compositor/oxide_compositor_output_surface.cc',
        'browser/compositor/oxide_compositor_output_surface.h',
        'browser/compositor/oxide_compositor_output_surface_gl.cc',
        'browser/compositor/oxide_compositor_output_surface_gl.h',
        'browser/compositor/oxide_compositor_output_surface_software.cc',
        'browser/compositor/oxide_compositor_output_surface_software.h',
        'browser/compositor/oxide_compositor_software_output_device.cc',
        'browser/compositor/oxide_compositor_software_output_device.h',
        'browser/compositor/oxide_compositor_thread_proxy.cc',
        'browser/compositor/oxide_compositor_thread_proxy.h',
        'browser/compositor/oxide_compositor_utils.cc',
        'browser/compositor/oxide_compositor_utils.h',
        'browser/oxide_access_token_store.cc',
        'browser/oxide_access_token_store.h',
        'browser/oxide_browser_context.cc',
        'browser/oxide_browser_context.h',
        'browser/oxide_browser_context_delegate.h',
        'browser/oxide_browser_context_impl.cc',
        'browser/oxide_browser_context_impl.h',
        'browser/oxide_browser_context_observer.cc',
        'browser/oxide_browser_context_observer.h',
        'browser/oxide_browser_process_main.cc',
        'browser/oxide_browser_process_main.h',
        'browser/oxide_content_browser_client.cc',
        'browser/oxide_content_browser_client.h',
        'browser/oxide_default_screen_info.h',
        'browser/oxide_devtools_http_handler_delegate.cc',
        'browser/oxide_devtools_http_handler_delegate.h',
        'browser/oxide_devtools_target.cc',
        'browser/oxide_devtools_target.h',
        'browser/oxide_file_picker.cc',
        'browser/oxide_file_picker.h',
        'browser/oxide_form_factor.h',
        'browser/oxide_form_factor_linux.cc',
        'browser/oxide_http_user_agent_settings.cc',
        'browser/oxide_http_user_agent_settings.h',
        'browser/oxide_io_thread.cc',
        'browser/oxide_io_thread.h',
        'browser/oxide_javascript_dialog.cc',
        'browser/oxide_javascript_dialog.h',
        'browser/oxide_javascript_dialog_manager.cc',
        'browser/oxide_javascript_dialog_manager.h',
        'browser/oxide_message_pump.cc',
        'browser/oxide_message_pump.h',
        'browser/oxide_network_delegate.cc',
        'browser/oxide_network_delegate.h',
        'browser/oxide_off_the_record_browser_context_impl.cc',
        'browser/oxide_off_the_record_browser_context_impl.h',
        'browser/oxide_permission_request.cc',
        'browser/oxide_permission_request.h',
        'browser/oxide_power_save_blocker.cc',
        'browser/oxide_renderer_frame_evictor.cc',
        'browser/oxide_renderer_frame_evictor.h',
        'browser/oxide_render_widget_host_view.cc',
        'browser/oxide_render_widget_host_view.h',
        'browser/oxide_render_widget_host_view_factory.cc',
        'browser/oxide_render_widget_host_view_factory.h',
        'browser/oxide_script_message_dispatcher_browser.cc',
        'browser/oxide_script_message_dispatcher_browser.h',
        'browser/oxide_script_message_impl_browser.cc',
        'browser/oxide_script_message_impl_browser.h',
        'browser/oxide_script_message_request_impl_browser.cc',
        'browser/oxide_script_message_request_impl_browser.h',
        'browser/oxide_script_message_target.h',
        'browser/oxide_ssl_config_service.cc',
        'browser/oxide_ssl_config_service.h',
        'browser/oxide_url_request_context.cc',
        'browser/oxide_url_request_context.h',
        'browser/oxide_user_agent_override_provider.cc',
        'browser/oxide_user_agent_override_provider.h',
        'browser/oxide_user_script_master.cc',
        'browser/oxide_user_script_master.h',
        'browser/oxide_web_contents_view.cc',
        'browser/oxide_web_contents_view.h',
        'browser/oxide_web_frame.cc',
        'browser/oxide_web_frame.h',
        'browser/oxide_web_popup_menu.cc',
        'browser/oxide_web_popup_menu.h',
        'browser/oxide_web_preferences.cc',
        'browser/oxide_web_preferences.h',
        'browser/oxide_web_preferences_observer.cc',
        'browser/oxide_web_preferences_observer.h',
        'browser/oxide_web_view.cc',
        'browser/oxide_web_view.h',
        'browser/oxide_web_view_contents_helper.cc',
        'browser/oxide_web_view_contents_helper.h',
        'browser/oxide_web_view_contents_helper_delegate.h',
        'common/oxide_constants.cc',
        'common/oxide_constants.h',
        'common/oxide_content_client.cc',
        'common/oxide_content_client.h',
        'common/oxide_core_export.h',
        'common/oxide_export.h',
        'common/oxide_file_utils.cc',
        'common/oxide_file_utils.h',
        'common/oxide_message_enums.h',
        'common/oxide_message_generator.cc',
        'common/oxide_message_generator.h',
        'common/oxide_messages.h',
        'common/oxide_net_resource_provider.cc',
        'common/oxide_net_resource_provider.h',
        'common/oxide_paths.cc',
        'common/oxide_paths.h',
        'common/oxide_script_message.cc',
        'common/oxide_script_message.h',
        'common/oxide_script_message_handler.cc',
        'common/oxide_script_message_handler.h',
        'common/oxide_script_message_request.cc',
        'common/oxide_script_message_request.h',
        'common/oxide_user_script.cc',
        'common/oxide_user_script.h',
        'gl/gl_context_linux.cc',
        'gl/gl_image_linux.cc',
        'gl/gl_implementation_linux.cc',
        'gl/gl_surface_linux.cc',
        'gl/oxide_gl_implementation.h',
        'gl/oxide_shared_gl_context.cc',
        'gl/oxide_shared_gl_context.h',
        'ozone/oxide_media_ozone_platform.cc',
        'ozone/oxide_ozone_platform.cc',
        'ozone/oxide_ozone_surface_factory.cc',
        'ozone/oxide_ozone_surface_factory.h',
        'renderer/oxide_content_renderer_client.cc',
        'renderer/oxide_content_renderer_client.h',
        'renderer/oxide_isolated_world_map.cc',
        'renderer/oxide_isolated_world_map.h',
        'renderer/oxide_object_backed_native_handler.cc',
        'renderer/oxide_object_backed_native_handler.h',
        'renderer/oxide_render_process_observer.cc',
        'renderer/oxide_render_process_observer.h',
        'renderer/oxide_script_message_dispatcher_renderer.cc',
        'renderer/oxide_script_message_dispatcher_renderer.h',
        'renderer/oxide_script_message_handler_renderer.cc',
        'renderer/oxide_script_message_handler_renderer.h',
        'renderer/oxide_script_message_impl_renderer.cc',
        'renderer/oxide_script_message_impl_renderer.h',
        'renderer/oxide_script_message_manager.cc',
        'renderer/oxide_script_message_manager.h',
        'renderer/oxide_script_message_object_handler.cc',
        'renderer/oxide_script_message_object_handler.h',
        'renderer/oxide_script_message_request_impl_renderer.cc',
        'renderer/oxide_script_message_request_impl_renderer.h',
        'renderer/oxide_script_message_request_object_handler.cc',
        'renderer/oxide_script_message_request_object_handler.h',
        'renderer/oxide_script_referenced_object.cc',
        'renderer/oxide_script_referenced_object.h',
        'renderer/oxide_user_script_scheduler.cc',
        'renderer/oxide_user_script_scheduler.h',
        'renderer/oxide_user_script_slave.cc',
        'renderer/oxide_user_script_slave.h',
        'renderer/oxide_v8_scoped_persistent.h',
        '<(DEPTH)/extensions/common/constants.cc',
        '<(DEPTH)/extensions/common/constants.h',
        '<(DEPTH)/extensions/common/error_utils.cc',
        '<(DEPTH)/extensions/common/error_utils.h',
        '<(DEPTH)/extensions/common/url_pattern.cc',
        '<(DEPTH)/extensions/common/url_pattern.h',
        '<(DEPTH)/extensions/common/url_pattern_set.cc',
        '<(DEPTH)/extensions/common/url_pattern_set.h'
      ],
      'conditions': [
        ['enable_plugins==1', {
          'sources': [
            'browser/oxide_pepper_host_factory_browser.cc',
            'browser/oxide_pepper_host_factory_browser.h',
            'browser/oxide_pepper_talk_host.cc',
            'browser/oxide_pepper_talk_host.h',
          ],
          'dependencies': [
            '<(DEPTH)/ppapi/ppapi_internal.gyp:ppapi_host',
            '<(DEPTH)/ppapi/ppapi_internal.gyp:ppapi_proxy',
            '<(DEPTH)/ppapi/ppapi_internal.gyp:ppapi_shared',
          ],
        }],
      ],
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
            '<(DEPTH)/build/util/version.py',
            '-f', '<(DEPTH)/chrome/VERSION',
            '-i', '<@(_inputs)',
            '-o', '<@(_outputs)'
          ]
        },
      ],
    }
  ]
}
