set (SERVICE_EXEC "indicator-location-service")

set_property (SOURCE main.cpp
              APPEND PROPERTY COMPILE_DEFINITIONS
              GETTEXT_PACKAGE="${GETTEXT_PACKAGE}"
              GNOMELOCALEDIR="@CMAKE_INSTALL_FULL_LOCALEDIR@")

include_directories (${SERVICE_INCLUDE_DIRS})

link_directories (${SERVICE_LIBRARY_DIRS})

set_property (SOURCE main.cc
              APPEND PROPERTY COMPILE_DEFINITIONS
              GETTEXT_PACKAGE="${GETTEXT_PACKAGE}"
              GNOMELOCALEDIR="@CMAKE_INSTALL_FULL_LOCALEDIR@")

qt5_generate_moc (app.cc app.moc)
qt5_generate_moc (service.cc service.moc)

add_executable (${SERVICE_EXEC}
                main.cc
                app.cc app.moc
                service.cc service.moc)

set_property (TARGET ${SERVICE_EXEC}
              APPEND PROPERTY COMPILE_FLAGS ${Qt5Core_EXECUTABLE_COMPILE_FLAGS})

target_link_libraries (${SERVICE_EXEC} ${SERVICE_LIBRARIES} ${GCOV_LIBS})

install (TARGETS ${SERVICE_EXEC} RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_PKGLIBEXECDIR})

