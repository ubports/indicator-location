set (SERVICE_EXEC "indicator-location-service")

add_executable (${SERVICE_EXEC} main.cc app.cc service.cc)

qt5_use_modules (${SERVICE_EXEC} Core)

set_property (TARGET ${SERVICE_EXEC}
              APPEND PROPERTY INCLUDE_DIRECTORIES
              ${SERVICE_INCLUDE_DIRS})

set_property (TARGET ${SERVICE_EXEC}
              APPEND PROPERTY LINK_LIBRARIES
              ${SERVICE_LIBRARIES} ${GCOV_LIBS})

set_property (SOURCE main.cc
              APPEND PROPERTY COMPILE_DEFINITIONS
              GETTEXT_PACKAGE="${GETTEXT_PACKAGE}"
              GNOMELOCALEDIR="@CMAKE_INSTALL_FULL_LOCALEDIR@")

install (TARGETS ${SERVICE_EXEC} RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_PKGLIBEXECDIR})

