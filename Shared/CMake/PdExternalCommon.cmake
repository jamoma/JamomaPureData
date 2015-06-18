set(PROJECT_SRCS ${PROJECT_SRCS} $<TARGET_OBJECTS:PdCommonSyms>)

add_library(${PROJECT_NAME} MODULE ${PROJECT_SRCS})
cotire(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH "")

#[[ block comment
set_property(TARGET ${PROJECT_NAME}
			 PROPERTY BUNDLE True)
set_property(TARGET ${PROJECT_NAME}
			 PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/MaxExternal-Info.plist")

set_property(TARGET ${PROJECT_NAME}
			 PROPERTY BUNDLE_EXTENSION "pd_darwin")


]]

# Since XCode does not like tildas in project names, if
# a project_name ends by _tilda, we replace it with '~' in the
# Pd object.
if("${PROJECT_NAME}" MATCHES ".*_tilda")
	string(REGEX REPLACE "_tilda" "~" JAMOMAPD_EXTERNAL_OUTPUT_NAME "${PROJECT_NAME}")
	set_target_properties(${PROJECT_NAME}
						  PROPERTIES OUTPUT_NAME "${JAMOMAPD_EXTERNAL_OUTPUT_NAME}")
endif()

# TODO same of _equal

# Todo link at a smaller granularity
target_link_libraries(${PROJECT_NAME} Foundation)
target_link_libraries(${PROJECT_NAME} Modular)
target_link_libraries(${PROJECT_NAME} JamomaPd)
target_link_libraries(${PROJECT_NAME} DSP)
target_link_libraries(${PROJECT_NAME} Graph)
target_link_libraries(${PROJECT_NAME} AudioGraph)

### Output ###
# remove "_pd" at the start of project name
string(REGEX REPLACE "^pd_" "" BIN_NAME ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${BIN_NAME})
if(APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".pd_darwin")
elseif(WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dll")
else()
## TODO use l_ia64 and l_i386 instead of pd_linux
	set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".pd_linux")
endif()

### Installation ###
if(APPLE)
    set(PD_EXTERNAL_RPATH "@loader_path/../support;@loader_path")
    set(PD_LOADER_RPATH "@loader_path/support;@loader_path/externals")
elseif( NOT WIN32 )
    set(PD_EXTERNAL_RPATH "\$ORIGIN/../support;\$ORIGIN")
    set(PD_LOADER_RPATH "\$ORIGIN/support;\$ORIGIN/externals")
endif()

if("${PROJECT_NAME}" STREQUAL "pd_Jamoma")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${PD_LOADER_RPATH}")
	install(TARGETS ${PROJECT_NAME}
            DESTINATION "${JAMOMAPD_INSTALL_FOLDER}/Jamoma"
            COMPONENT JamomaPd)
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${PD_EXTERNAL_RPATH}")
	install(TARGETS ${PROJECT_NAME}
            DESTINATION "${JAMOMAPD_INSTALL_FOLDER}/Jamoma/externals"
            COMPONENT JamomaPd)
endif()

### Tests ###
addTestTarget()
