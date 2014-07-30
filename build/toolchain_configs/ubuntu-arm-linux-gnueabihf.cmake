set(CMAKE_SYSTEM_NAME Linux CACHE INTERNAL "")

find_program(_DPKG_ARCH_EXECUTABLE dpkg-architecture)
if(_DPKG_ARCH_EXECUTABLE STREQUAL "DPKG_ARCHITECTURE_EXECUTABLE-NOTFOUND")
  message(FATAL_ERROR "dpkg-architecture not found")
endif()
execute_process(COMMAND ${_DPKG_ARCH_EXECUTABLE} -qDEB_BUILD_GNU_TYPE
                RESULT_VARIABLE _RESULT
                OUTPUT_VARIABLE OXIDE_LIBRARY_HOST_ARCHITECTURE
                OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to determine host architecture")
endif()

set(GCC_VERSION_SUFFIX "")
if(USE_GCC_VERSION)
  set(GCC_VERSION_SUFFIX -${USE_GCC_VERSION})
endif()

set(CHROMIUM_C_HOST_COMPILER gcc${GCC_VERSION_SUFFIX} CACHE INTERNAL "")
set(CHROMIUM_CXX_HOST_COMPILER g++${GCC_VERSION_SUFFIX} CACHE INTERNAL "")
set(CHROMIUM_HOST_AR ar CACHE INTERNAL "")
set(CHROMIUM_TARGET_ARCH arm CACHE INTERNAL "")
set(CHROMIUM_PKG_CONFIG_PATH /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig CACHE INTERNAL "")

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc${GCC_VERSION_SUFFIX} CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++${GCC_VERSION_SUFFIX} CACHE INTERNAL "")
set(CMAKE_AR arm-linux-gnueabihf-ar CACHE INTERNAL "")
set(CMAKE_LIBRARY_ARCHITECTURE arm-linux-gnueabihf CACHE INTERNAL "")
