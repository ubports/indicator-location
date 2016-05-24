project (indicator-location)
cmake_minimum_required (VERSION 2.8.9)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set (PROJECT_VERSION "13.10.0")
set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE indicator-location)

set (SERVICE_LIB ${PACKAGE})
set (SERVICE_EXEC "${PACKAGE}-service")

option (enable_tests "Build the package's automatic tests." ON)
option (enable_lcov "Generate lcov code coverage reports." ON)

##
##  GNU standard paths
## 
include (GNUInstallDirs)
if (EXISTS "/etc/debian_version") # Workaround for libexecdir on debian
  set (CMAKE_INSTALL_LIBEXECDIR "${CMAKE_INSTALL_LIBDIR}")
  set (CMAKE_INSTALL_FULL_LIBEXECDIR "${CMAKE_INSTALL_FULL_LIBDIR}")
endif ()
set (CMAKE_INSTALL_PKGLIBEXECDIR "${CMAKE_INSTALL_LIBEXECDIR}/${CMAKE_PROJECT_NAME}")
set (CMAKE_INSTALL_FULL_PKGLIBEXECDIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}/${CMAKE_PROJECT_NAME}")

##
##  Check for prerequisites
##

find_package (PkgConfig REQUIRED)

include (FindPkgConfig)
pkg_check_modules (SERVICE_DEPS REQUIRED
                   ubuntu-app-launch-2
                   ubuntu-location-service
                   url-dispatcher-1
                   gio-unix-2.0>=2.36
                   glib-2.0>=2.36
                   properties-cpp>=0.0.1)
include_directories (SYSTEM ${SERVICE_DEPS_INCLUDE_DIRS})

##
##  Code Style
##

# Code style fixer from unity-scopes-api.
#
# We put the code through astyle first because it makes some fixes that clang-format
# won't do (such as "char *p" -> "char* p"). But astyle messes up other things
# (particularly lambdas and assembly-style comments), which clang-format does right.
# So, we run clang-format after running astyle, which undoes the damage done by astyle
# without reverting desirable astyle fixes.

find_program(ASTYLE_COMMAND NAMES astyle)
if (NOT ASTYLE_COMMAND)
    message(WARNING "Cannot find astyle: formatcode target will not be available")
else()
    # astyle 2.03 creates DOS line endings, so we need to fix its output
    find_program(DOS2UNIX_COMMAND NAMES dos2unix)
    if (NOT DOS2UNIX_COMMAND)
        message(WARNING "Cannot find dos2unix: formatcode target will not be available")
    else()
        find_program(CLANG_FORMAT_COMMAND NAMES clang-format-3.6 clang-format-3.5)
        if (NOT CLANG_FORMAT_COMMAND)
            message(WARNING "Cannot find clang-format >= clang-format-3.5: formatcode target will not be available")
        endif()
    endif()
endif()

if (ASTYLE_COMMAND AND DOS2UNIX_COMMAND AND CLANG_FORMAT_COMMAND)
    add_custom_target(formatcode
                      ${PROJECT_SOURCE_DIR}/tools/format-files.sh ${PROJECT_SOURCE_DIR} ${ASTYLE_COMMAND} ${CLANG_FORMAT_COMMAND})
endif()

##
##
##

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CC_WARNING_ARGS " -Wall -Wextra -Wno-missing-field-initializers")

# testing & coverage
if (${enable_tests})
  set (GTEST_SOURCE_DIR /usr/src/gtest/src)
  set (GTEST_INCLUDE_DIR ${GTEST_SOURCE_DIR}/..)
  set (GTEST_LIBS -lpthread)
  enable_testing ()
  if (${enable_lcov})
    include(GCov)
  endif ()
endif ()

add_subdirectory (src)
add_subdirectory (data)
add_subdirectory (po)
add_subdirectory (tools)
if (${enable_tests})
  add_subdirectory (tests)
endif ()
