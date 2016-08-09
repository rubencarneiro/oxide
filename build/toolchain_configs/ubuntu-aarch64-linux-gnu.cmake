set(CMAKE_SYSTEM_NAME Linux CACHE INTERNAL "")
set(CMAKE_SYSTEM_PROCESSOR aarch64 CACHE INTERNAL "")

find_program(_DPKG_ARCH_EXECUTABLE dpkg-architecture)
if(_DPKG_ARCH_EXECUTABLE STREQUAL "DPKG_ARCHITECTURE_EXECUTABLE-NOTFOUND")
  message(FATAL_ERROR "dpkg-architecture not found")
endif()
execute_process(COMMAND ${_DPKG_ARCH_EXECUTABLE} -qDEB_BUILD_MULTIARCH
                RESULT_VARIABLE _RESULT
                OUTPUT_VARIABLE OXIDE_LIBRARY_HOST_ARCHITECTURE
                OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to determine host architecture")
endif()

# Set the target variables used by CMake
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu CACHE INTERNAL "")
set(CMAKE_C_COMPILER /usr/bin/${CMAKE_LIBRARY_ARCHITECTURE}-gcc CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER /usr/bin/${CMAKE_LIBRARY_ARCHITECTURE}-g++ CACHE INTERNAL "")
set(CMAKE_AR /usr/bin/${CMAKE_LIBRARY_ARCHITECTURE}-ar CACHE INTERNAL "")

# Set various host variables required by Chromium
set(CHROMIUM_C_HOST_COMPILER /usr/bin/gcc CACHE INTERNAL "")
set(CHROMIUM_CXX_HOST_COMPILER /usr/bin/g++ CACHE INTERNAL "")
set(CHROMIUM_HOST_AR /usr/bin/ar CACHE INTERNAL "")
set(CHROMIUM_CXX_HOST_COMPILER_ID GNU CACHE INTERNAL "")

# Set various target variables required by Chromium
set(CHROMIUM_NM /usr/bin/${CMAKE_LIBRARY_ARCHITECTURE}-nm)
set(CHROMIUM_READELF /usr/bin/${CMAKE_LIBRARY_ARCHITECTURE}-readelf)
set(CHROMIUM_TARGET_ARCH arm64 CACHE INTERNAL "")
set(CHROMIUM_PKG_CONFIG /usr/bin/${CMAKE_LIBRARY_ARCHITECTURE}-pkg-config CACHE INTERNAL "")
