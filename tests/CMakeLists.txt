###
###  Google Test Framework
###

add_library (gtest STATIC 
             ${GTEST_SOURCE_DIR}/gtest-all.cc 
             ${GTEST_SOURCE_DIR}/gtest_main.cc)

###
###  test-service-class
###

set (TEST_SERVICE "test-service-class")
add_executable (${TEST_SERVICE} hello-world.cc)
add_test (${TEST_SERVICE} ${TEST_SERVICE})


###
###  globals
###

set (ALL_TESTS ${TEST_SERVICE})

set_property (TARGET ${ALL_TESTS}
              APPEND_STRING PROPERTY COMPILE_FLAGS "${CC_WARNING_ARGS} -g --std=c++11")

set_property (TARGET ${ALL_TESTS}
              APPEND PROPERTY LINK_LIBRARIES libindicator-location-service gtest ${GTEST_LIBS} ${GCOV_LIBS})

set_property (TARGET ${ALL_TESTS} gtest
              APPEND PROPERTY INCLUDE_DIRECTORIES ${SERVICE_INCLUDE_DIRS} ${GTEST_INCLUDE_DIR})

