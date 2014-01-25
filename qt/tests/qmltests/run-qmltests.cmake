foreach(v CHROMIUM_PRODUCT_DIR CHROMIUM_LIB_DIR SOURCE_DIR SERVER_DIR BINARY_DIR TEST_DIR TEST_NAME)
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "Must specify ${v}")
  endif()
endforeach()

if(USE_DATA_DIR)
  set(_T -t)
endif()

set(ENV{OXIDE_NO_SANDBOX} 1)
set(ENV{OXIDE_RESOURCE_PATH} ${CHROMIUM_PRODUCT_DIR})
set(ENV{LD_LIBRARY_PATH} ${CHROMIUM_LIB_DIR})

set(_COMMAND python ${SOURCE_DIR}/qt/tests/runtests.py -s ${SERVER_DIR} -p 8080 ${_T} --
    ${BINARY_DIR}/qt/bin/qmltest -name ${TEST_NAME} -import ${BINARY_DIR}/qt/imports -input ${TEST_DIR})
message("Running ${_COMMAND}")
execute_process(COMMAND ${_COMMAND} RESULT_VARIABLE _RESULT)
if(NOT ${_RESULT} EQUAL 0)
  message(FATAL_ERROR "Tests ${TEST_NAME} failed!")
endif()
