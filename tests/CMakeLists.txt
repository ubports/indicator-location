###
###  Google Test Framework
###

add_library (gtest STATIC 
             ${GTEST_SOURCE_DIR}/gtest-all.cc 
             ${GTEST_SOURCE_DIR}/gtest_main.cc)

###
###  phone-test
###

set (PHONE_TEST "phone-test")
add_executable (${PHONE_TEST} phone-test.cc)
add_test (${PHONE_TEST} ${PHONE_TEST})


###
###  globals
###

set (ALL_TESTS ${PHONE_TEST})

set_property (TARGET ${ALL_TESTS}
              APPEND_STRING PROPERTY COMPILE_FLAGS "${CC_WARNING_ARGS} -g --std=c++11")

set_property (TARGET ${ALL_TESTS}
              APPEND PROPERTY LINK_LIBRARIES libindicator-location-service gtest ${GTEST_LIBS} ${GCOV_LIBS})

set_property (TARGET ${ALL_TESTS} gtest
              APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR} ${SERVICE_INCLUDE_DIRS} ${GTEST_INCLUDE_DIR})

