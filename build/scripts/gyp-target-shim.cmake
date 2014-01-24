set(ENV{CFLAGS})
set(ENV{CXXFLAGS})
set(ENV{CPPFLAGS})
set(ENV{LDFLAGS})

if(NOT DEFINED MAKE OR NOT DEFINED GYP_DIR)
  message(FATAL_ERROR "Need to define MAKE and GYP_DIR")
endif()

if(DEFINED BUILDTYPE)
  set(BUILDTYPE_ARG BUILDTYPE=${BUILDTYPE})
else()
  set(BUILDTYPE_ARG)
endif()

execute_process(COMMAND ${MAKE} -C ${GYP_DIR} -f Makefile ${BUILDTYPE_ARG} all
                RESULT_VARIABLE _RESULT)
if(NOT ${_RESULT} EQUAL 0)
  message(FATAL_ERROR "Failed to build gyp target")
endif()
