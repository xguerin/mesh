#
# Project definition
#

project("Mesh" CXX)
cmake_minimum_required(VERSION 3.3)

include(cmake/GetGitRevisionDescription.cmake)

#
# Uninstall target
#

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake)

#
# Get the current GIT version
#

git_describe(VERSION)

string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")

set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
message(STATUS "Build version: " ${VERSION_SHORT})

#
# Global definitions
#

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release ... FORCE)
endif()

add_definitions(-DMESH_VERSION="${VERSION_SHORT}")

#
# Flags preferences
#

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unused-parameter")
else()
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unused-parameter -pthread")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3")

#
# Dependencies checks
#

find_package(Threads REQUIRED)
find_package(TCLAP REQUIRED)
find_package(ACE REQUIRED)

include_directories(${TCLAP_INCLUDE_DIRS})
include_directories(${ACE_INCLUDE_DIRS})

#
# Subdirectories
#

add_subdirectory(tools)
