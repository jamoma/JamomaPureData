set(PROJECT_SRCS ${PROJECT_SRCS} $<TARGET_OBJECTS:PdCommonSyms>)

add_library(${PROJECT_NAME} MODULE ${PROJECT_SRCS})

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

#[[ block comment
if(APPLE)
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_MACH_O_TYPE "mh_bundle")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_WRAPPER_EXTENSION "mxo")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/../c74support/max-includes/macho-prefix.pch")
		#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../c74support/max-includes/Info.plist")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_WARNING_CFLAGS "-Wmost -Wno-four-char-constants -Wno-unknown-pragmas")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_DEPLOYMENT_LOCATION "YES")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_DSTROOT "${EXECUTABLE_OUTPUT_PATH}")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_INSTALL_PATH "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/externals")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_DYNAMIC_NO_PIC "NO")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "@loader_path/../support;@loader_path")
elseif( NOT WIN32 )
                SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN/../support;\$ORIGIN")
endif()
]]

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

if("${PROJECT_NAME}" STREQUAL "pd_jamoma")
        install(TARGETS ${PROJECT_NAME}
                        DESTINATION "${JAMOMAPD_INSTALL_FOLDER}/Jamoma"
                        COMPONENT JamomaPd)
else()
        install(TARGETS ${PROJECT_NAME}
                        DESTINATION "${JAMOMAPD_INSTALL_FOLDER}/Jamoma/externals"
                        COMPONENT JamomaPd)
endif()


### Tests ###
addTestTarget()
