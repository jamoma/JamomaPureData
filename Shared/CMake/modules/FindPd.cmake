if (WIN32)
	
	find_path(PD_MAIN_PATH /pd/)
	message("PD_MAIN_PATH : ${PD_MAIN_PATH}")
	
	if (PD_MAIN_PATH)
		find_path(PD_INCLUDE_DIRS m_pd.h HINTS "${PD_MAIN_PATH}/src/" "${PD_MAIN_PATH}/include/")
		#pd.lib is needed in Win32
		find_library(PD_LIBRARY NAMES pd.lib HINTS "${PD_MAIN_PATH}/bin/")
	endif (PD_MAIN_PATH)
	
	if (PD_INCLUDE_DIRS AND PD_LIBRARY)
		set(PD_FOUND TRUE)
	endif (PD_INCLUDE_DIRS AND PD_LIBRARY)
	
else (WIN32)

	if (LINUX)
		find_path(PD_INCLUDE_DIRS m_pd.h HINTS "/usr/local/include/pd"  "/usr/include/pd" "/usr/include/pdextended")
	elseif (APPLE)
		find_path(PD_INCLUDE_DIRS m_pd.h HINTS "/Applications/Pd-*.app/Contents/Resources/include" "/Applications/Pd-extended.app/Contents/Resources/include")
	endif (LINUX)
	
	if (PD_INCLUDE_DIRS)
		set(PD_FOUND TRUE)
	endif (PD_INCLUDE_DIRS)

endif (WIN32)

if (PD_FOUND)
	if (NOT PD_FIND_QUIETLY)
		message (STATUS "Found PD: ${PD_INCLUDE_DIRS}")
		if (WIN32)
			message (STATUS "Found PD lib: ${PD_LIBRARY}")
		endif (WIN32)
	endif (NOT PD_FIND_QUIETLY)
else (PD_FOUND)
	if (PD_FIND_REQUIRED)
		message (FATAL_ERROR "Could not find PD")
	endif (PD_FIND_REQUIRED)
endif (PD_FOUND)

# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Pd
                                  REQUIRED_VARS PD_INCLUDE_DIRS )

mark_as_advanced(PD_INCLUDE_DIRS PD_LIBRARY)