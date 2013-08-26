##
##  DBus Service File
##

# where to install
set (DBUS_SERVICE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/dbus-1/services")
message (STATUS "${DBUS_SERVICE_DIR} is the DBus Service File install dir")

set (SERVICE_NAME "${CMAKE_PROJECT_NAME}.service")
set (SERVICE_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SERVICE_NAME}")
set (SERVICE_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/${SERVICE_NAME}.in")

# build it
set (pkglibexecdir "${CMAKE_INSTALL_FULL_PKGLIBEXECDIR}")
configure_file ("${SERVICE_FILE_IN}" "${SERVICE_FILE}")

# install it
install (FILES "${SERVICE_FILE}"
         DESTINATION "${DBUS_SERVICE_DIR}")

##
##  Unity Indicator File
##

# where to install
set (UNITY_INDICATOR_DIR "${CMAKE_INSTALL_FULL_DATAROOTDIR}/unity/indicators")
message (STATUS "${UNITY_INDICATOR_DIR} is the Unity Indicator install dir")

set (UNITY_INDICATOR_NAME "com.canonical.indicator.location")
set (UNITY_INDICATOR_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${UNITY_INDICATOR_NAME}")

install (FILES "${UNITY_INDICATOR_FILE}"
         DESTINATION "${UNITY_INDICATOR_DIR}")
