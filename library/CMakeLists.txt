cmake_minimum_required(VERSION 3.0)
project(JamomaPd)

add_definitions(-DJAMOMA_EXPORTS)

# find_package(Pd REQUIRED)
# include_directories(${PD_INCLUDE_DIR})

### GetGitDescription ###
include(${CMAKE_CURRENT_SOURCE_DIR}/../Shared/CMake/modules/GetGitRevisionDescription.cmake	)
get_git_head_revision(REFSPEC HASH --always --tags)
git_get_exact_tag(GIT_TAG)
string(SUBSTRING ${HASH} 0 7 GIT_SHA_SHORT)
message("building on Git rev : " ${GIT_SHA_SHORT})
message("Git tag : " ${GIT_TAG})
string(LENGTH "${GIT_TAG}" taglen)
if (taglen GREATER 0)
string(REPLACE "." ";" GIT_TAG_LIST ${GIT_TAG})
list(LENGTH GIT_TAG_LIST len)
if (len GREATER 0)
	list(GET GIT_TAG_LIST 0 GIT_VERSION_MAJ)
endif (len GREATER 0)
if (len GREATER 1)
	list(GET GIT_TAG_LIST 1 GIT_VERSION_MIN)
endif (len GREATER 1)
if (len GREATER 2)
	list(GET GIT_TAG_LIST 2 GIT_VERSION_SUB)
endif (len GREATER 2)
if (len GREATER 3)
	list(GET GIT_TAG_LIST 3 GIT_VERSION_MOD_LONG)
endif (len GREATER 3)
list(LENGTH GIT_VERSION_MOD_LONG len2)
if (len2 GREATER 0)
	string(REPLACE "-" ";" GIT_VERSION_MOD_LIST ${GIT_VERSION_MOD_LONG})
	list(GET GIT_VERSION_MOD_LIST 0 GIT_VERSION_MOD)
endif (len2 GREATER 0)
endif (taglen GREATER 0)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/includes/JamomaPdVersion.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/includes/JamomaPdVersion.h" @ONLY)

set(PROJECT_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/source/JamomaForPd.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/JamomaModularForPd.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/JamomaObject.cpp
${CMAKE_CURRENT_SOURCE_DIR}/source/JamomaSymbols.cpp
$<TARGET_OBJECTS:PdCommonSyms>
)

include_directories("${PROJECT_SOURCE_DIR}/includes")

    file(GLOB_RECURSE PROJECT_HDRS
        ${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h
        ${JAMOMA_CORE_SRC_PATH}/Foundation/library/includes/*.h
        ${JAMOMA_CORE_SRC_PATH}/Modular/library/includes/*.h
        ${JAMOMA_CORE_SRC_PATH}/Modular/library/PeerObject/*.h
        ${JAMOMA_CORE_SRC_PATH}/Modular/library/ProtocolLib/*.h
        ${JAMOMA_CORE_SRC_PATH}/Modular/library/SchedulerLib/*.h
        ${JAMOMA_CORE_SRC_PATH}/Graph/library/includes/*.h
    )
    include_directories("${JAMOMA_CORE_SRC_PATH}/Foundation/library/includes")
    include_directories("${JAMOMA_CORE_SRC_PATH}/Modular/library/includes")
    include_directories("${JAMOMA_CORE_SRC_PATH}/Modular/library/PeerObject")
    include_directories("${JAMOMA_CORE_SRC_PATH}/Modular/library/ProtocolLib")
    include_directories("${JAMOMA_CORE_SRC_PATH}/Modular/library/SchedulerLib")
    include_directories("${JAMOMA_CORE_SRC_PATH}/DSP/library/includes")
    include_directories("${JAMOMA_CORE_SRC_PATH}/Graph/library/includes")

    include_directories("${PROJECT_SOURCE_DIR}/../source/CicmWrapper/Sources")
    include_directories("${PROJECT_SOURCE_DIR}/../source/")


add_library(${PROJECT_NAME} SHARED ${PROJECT_SRCS} ${PROJECT_HDRS})

target_link_libraries(${PROJECT_NAME} Foundation)
target_link_libraries(${PROJECT_NAME} Modular)
target_link_libraries(${PROJECT_NAME} CicmWrapper)

if (WIN32)
	target_link_libraries(${PROJECT_NAME} ${PD_LIBRARY})
endif()

install(TARGETS ${PROJECT_NAME}
    DESTINATION "${JAMOMAPD_INSTALL_FOLDER}/Jamoma/externals"
    COMPONENT JamomaPd)

if( APPLE )
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "@loader_path/../support;@loader_path")
elseif( NOT WIN32 )
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN/../support;\$ORIGIN")
endif()

# addAppleFramework(Carbon)

### Output ###
setOutput()

### Tests ###
addTestTarget()
