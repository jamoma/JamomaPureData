# - Try to find Pd
# Once done this will define
#  PD_FOUND - System has Pd
#  PD_INCLUDE_DIRS - The Pd include directories
#  PD_LIBRARIES - The libraries needed to use Pd
#  PD_DEFINITIONS - Compiler switches required for using Pd

find_package(PkgConfig)
pkg_check_modules(PC_PD pd)
set(PD_DEFINITIONS ${PC_PD_CFLAGS_OTHER})

find_path(PD_INCLUDE_DIR m_pd.h
          HINTS ${PC_PD_INCLUDEDIR} ${PC_PD_INCLUDE_DIRS}
          PATH_SUFFIXES pd )

find_library(PD_LIBRARY NAMES pd
             HINTS ${PC_PD_LIBDIR} ${PC_PD_LIBRARY_DIRS} )

set(PD_LIBRARIES ${PD_LIBRARY} )
set(PD_INCLUDE_DIRS ${PD_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PD_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Pd  DEFAULT_MSG PD_INCLUDE_DIR)

mark_as_advanced(PD_INCLUDE_DIR)
