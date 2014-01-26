set(CMAKE_SYSTEM_NAME Linux)

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

set(CHROMIUM_C_HOST_COMPILER ${CMAKE_C_COMPILER})
set(CHROMIUM_CXX_HOST_COMPILER ${CMAKE_CXX_COMPILER})
set(CHROMIUM_TARGET_ARCH arm)
set(CHROMIUM_PKG_CONFIG_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig")
set(CHROMIUM_LINKER arm-linux-gnueabihf-ld)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_LIBRARY_ARCHITECTURE arm-linux-gnueabihf)
