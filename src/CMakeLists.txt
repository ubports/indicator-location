set (SERVICE_EXEC "indicator-location-service")

set_property (SOURCE main.cpp
              APPEND PROPERTY COMPILE_DEFINITIONS
              GETTEXT_PACKAGE="${GETTEXT_PACKAGE}"
              GNOMELOCALEDIR="@CMAKE_INSTALL_FULL_LOCALEDIR@")

include_directories (${SERVICE_INCLUDE_DIRS})

link_directories (${SERVICE_LIBRARY_DIRS})

add_executable (${SERVICE_EXEC} main.c)

target_link_libraries (${SERVICE_EXEC} ${SERVICE_LIBRARIES} ${GCOV_LIBS})

install (TARGETS ${SERVICE_EXEC} RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_PKGLIBEXECDIR})

