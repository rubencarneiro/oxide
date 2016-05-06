{
  'variables': {
    'ozone_platform': 'oxide',
    'ozone_auto_platforms': 0,
    'external_ozone_platforms': [
      'oxide',
    ],
    'external_ozone_platform_deps': [
      '<(DEPTH)/third_party/khronos/khronos.gyp:khronos_headers',
    ],
    'external_ozone_platform_files': [
      '../../../../ui/ozone/platform/oxide/ozone_platform_oxide.cc',
      '../../../../ui/ozone/platform/oxide/surface_factory_ozone_oxide.cc',
      '../../../../ui/ozone/platform/oxide/surface_factory_ozone_oxide.h',
    ],
  },
}
