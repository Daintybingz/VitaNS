# FindDynarmic.cmake
# Finds the Dynarmic library for ARM emulation
#
# This module defines:
#  DYNARMIC_FOUND - True if Dynarmic was found
#  DYNARMIC_INCLUDE_DIRS - Include directories for Dynarmic
#  DYNARMIC_LIBRARIES - Libraries to link against Dynarmic
#  DYNARMIC_DEFINITIONS - Compiler definitions for Dynarmic

# Try to find Dynarmic using pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_DYNARMIC QUIET dynarmic)
endif()

# Find include directory
find_path(DYNARMIC_INCLUDE_DIR
  NAMES dynarmic/interface/A32/config.h
  PATHS
    ${PC_DYNARMIC_INCLUDEDIR}
    ${PC_DYNARMIC_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/externals/dynarmic/include
    /usr/include
    /usr/local/include
  PATH_SUFFIXES dynarmic
)

# Find library
find_library(DYNARMIC_LIBRARY
  NAMES dynarmic
  PATHS
    ${PC_DYNARMIC_LIBDIR}
    ${PC_DYNARMIC_LIBRARY_DIRS}
    ${CMAKE_SOURCE_DIR}/externals/dynarmic/build
    /usr/lib
    /usr/local/lib
)

# Set version from pkg-config if available
set(DYNARMIC_VERSION ${PC_DYNARMIC_VERSION})

# Handle the QUIETLY and REQUIRED arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Dynarmic
  FOUND_VAR DYNARMIC_FOUND
  REQUIRED_VARS
    DYNARMIC_LIBRARY
    DYNARMIC_INCLUDE_DIR
  VERSION_VAR DYNARMIC_VERSION
)

# Set output variables
if(DYNARMIC_FOUND)
  set(DYNARMIC_LIBRARIES ${DYNARMIC_LIBRARY})
  set(DYNARMIC_INCLUDE_DIRS ${DYNARMIC_INCLUDE_DIR})
  set(DYNARMIC_DEFINITIONS ${PC_DYNARMIC_CFLAGS_OTHER})
endif()

# Mark as advanced
mark_as_advanced(
  DYNARMIC_INCLUDE_DIR
  DYNARMIC_LIBRARY
)
