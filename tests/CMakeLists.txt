
###
###  Google Test Framework
###

add_library (gtest STATIC 
             ${GTEST_SOURCE_DIR}/gtest-all.cc 
             ${GTEST_SOURCE_DIR}/gtest_main.cc)
set_target_properties (gtest PROPERTIES INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} ${GTEST_INCLUDE_DIR})
set_target_properties (gtest PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS} -w)

###
###  General setup
###

include_directories (${CMAKE_SOURCE_DIR})
#include_directories (${CMAKE_CURRENT_BINARY_DIR})

set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -g ${CC_WARNING_ARGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ${CC_WARNING_ARGS} -std=c++11")

add_test(NAME formatcode 
         COMMAND "${CMAKE_SOURCE_DIR}/tools/format-test.sh" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/tools/formatcode")

###
###  phone-test
###

set (TEST_NAME phone-test)
add_executable (${TEST_NAME} ${TEST_NAME}.cc)
add_test (${TEST_NAME} ${TEST_NAME})
add_dependencies (${TEST_NAME} ${SERVICE_LIB})
target_link_libraries (${TEST_NAME} ${SERVICE_LIB} gtest ${SERVICE_DEPS_LIBRARIES} ${GTEST_LIBS})

###
###  globals
###

#set_property (TARGET ${ALL_TESTS}
#              APPEND PROPERTY LINK_LIBRARIES libindicator-location-service gtest ${GTEST_LIBS} ${GCOV_LIBS})
#
#set_property (TARGET ${ALL_TESTS} gtest
#              APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR} ${SERVICE_INCLUDE_DIRS} ${GTEST_INCLUDE_DIR})

