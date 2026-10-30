# Copyright (c) 2026 The Vertcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

#[=======================================================================[
FindGMP
-------

Finds the GMP header and library.

This is a wrapper around find_path()/find_library() commands that:
 - facilitates searching in various build environments
 - prints a standard log message

#]=======================================================================]

find_path(GMP_INCLUDE_DIR
  NAMES gmp.h
)
find_library(GMP_LIBRARY
  NAMES gmp libgmp
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP
  REQUIRED_VARS GMP_LIBRARY GMP_INCLUDE_DIR
)

if(GMP_FOUND AND NOT TARGET GMP::GMP)
  add_library(GMP::GMP UNKNOWN IMPORTED)
  set_target_properties(GMP::GMP PROPERTIES
    IMPORTED_LOCATION "${GMP_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  GMP_INCLUDE_DIR
  GMP_LIBRARY
)
