# - Find ACE 
# Find the native ACE includes and library
#
#  ACE_INCLUDE_DIRS - where to find re2.h, etc.
#  ACE_LIBRARIES    - List of libraries when using ACE.
#  ACE_COMPILER     - where the ACE compiler is.
#  ACE_FOUND        - True if ACE found.

find_path(ACE_INCLUDE_DIR
  NAMES ace/model/Helper.h
  NO_DEFAULT_PATH
  PATHS
  /usr/local
  /usr
  PATH_SUFFIXES include
  HINTS
  ${ACE_ROOT}
  $ENV{ACE_ROOT})

find_library(ACE_LIBRARY
  NAMES ace
  NO_DEFAULT_PATH
  PATHS
  /usr/local
  /usr
  PATH_SUFFIXES lib
  HINTS
  ${ACE_ROOT}
  $ENV{ACE_ROOT})

find_program(ACE_COMPILER
  NAMES ace-compile
  NO_DEFAULT_PATH
  PATHS
  /usr/local
  /usr
  PATH_SUFFIXES bin
  HINTS
  ${ACE_ROOT}
  $ENV{ACE_ROOT})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ACE DEFAULT_MSG ACE_LIBRARY ACE_INCLUDE_DIR ACE_COMPILER)

if(ACE_FOUND)
  set(ACE_LIBRARIES ${ACE_LIBRARY})
  set(ACE_INCLUDE_DIRS ${ACE_INCLUDE_DIR})
endif()
