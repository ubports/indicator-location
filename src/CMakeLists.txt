
set_property (DIRECTORY
              APPEND PROPERTY INCLUDE_DIRECTORIES ${SERVICE_INCLUDE_DIRS})

set_property (DIRECTORY
              APPEND PROPERTY LINK_DIRECTORIES ${SERVICE_INCLUDE_DIRS})


###
###  The library.
###  this doesn't get installed anywhere;
###  it's built apart from main() as a convenience for tests/
###

set (INDICATOR_LIB "libindicator-location-service")

add_library (${INDICATOR_LIB} STATIC controller.cc main.cc phone.cc service.cc)

set_property (TARGET ${INDICATOR_LIB}
              APPEND PROPERTY LINK_LIBRARIES ${INDICATOR_LIB} ${SERVICE_LIBRARIES} ${GCOV_LIBS})


###
###  The service execuable.
###

set (INDICATOR_EXEC "indicator-location-service")

add_executable (${INDICATOR_EXEC} main.cc)

set_property (SOURCE main.cc
              APPEND PROPERTY COMPILE_DEFINITIONS
              GETTEXT_PACKAGE="${GETTEXT_PACKAGE}"
              GNOMELOCALEDIR="@CMAKE_INSTALL_FULL_LOCALEDIR@")

target_link_libraries (${INDICATOR_EXEC} libindicator-location-service)

install (TARGETS ${INDICATOR_EXEC}
         RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_PKGLIBEXECDIR})



set_property (TARGET ${INDICATOR_LIB} ${INDICATOR_EXEC}
              APPEND_STRING PROPERTY COMPILE_FLAGS " ${CC_WARNING_ARGS} --std=c++11")

