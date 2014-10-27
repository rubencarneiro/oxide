{
  'variables': {
    'ozone_platform': 'oxide',
    'ozone_auto_platforms': 0,
    'external_ozone_platforms': [
      'oxide',
    ],
    'external_ozone_platform_files': [
      '<(DEPTH)/../../../shared/port/ozone/oxide_media_ozone_platform.cc',
      '<(DEPTH)/../../../shared/port/ozone/oxide_ozone_platform.cc',
      '<(DEPTH)/../../../shared/port/ozone/oxide_ozone_surface_factory.cc',
      '<(DEPTH)/../../../shared/port/ozone/oxide_ozone_surface_factory.h',
    ],
  },
}
