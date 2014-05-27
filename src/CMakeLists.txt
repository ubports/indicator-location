SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g ${CXX_WARNING_ARGS} ${GCOV_FLAGS}")

#set_property (DIRECTORY
#              APPEND PROPERTY INCLUDE_DIRECTORIES ${SERVICE_INCLUDE_DIRS})
#
#set_property (DIRECTORY
#              APPEND PROPERTY LINK_DIRECTORIES ${SERVICE_INCLUDE_DIRS})


###
###  The library.
###  this doesn't get installed anywhere;
###  it's built apart from main() as a convenience for tests/
###

add_library (${SERVICE_LIB} STATIC controller.cc phone.cc service.cc controller-ualc.cc)
include_directories (${CMAKE_SOURCE_DIR})
link_directories (${SERVICE_DEPS_LIBRARY_DIRS})


###
###  The service execuable.
###

add_executable (${SERVICE_EXEC} main.cc)
target_link_libraries (${SERVICE_EXEC} ${SERVICE_LIB} ${SERVICE_DEPS_LIBRARIES})
install (TARGETS ${SERVICE_EXEC}
         RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_PKGLIBEXECDIR})

set_property (SOURCE main.cc
              APPEND PROPERTY COMPILE_DEFINITIONS
              GETTEXT_PACKAGE="${GETTEXT_PACKAGE}"
              GNOMELOCALEDIR="@CMAKE_INSTALL_FULL_LOCALEDIR@")

