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

if(JAMOMA_UMBRELLA)

    file(GLOB_RECURSE PROJECT_HDRS
        ${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}../../../Core/Foundation/library/includes/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}../../../Core/Modular/library/includes/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}../../../Core/Modular/library/PeerObject/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}../../../Core/Modular/library/ProtocolLib/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}../../../Core/Modular/library/SchedulerLib/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../Core/Graph/library/includes/*.h
    )
    include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Foundation/library/includes")
    include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Modular/library/includes")
    include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Modular/library/PeerObject")
    include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Modular/library/ProtocolLib")
    include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Modular/library/SchedulerLib")
    include_directories("${PROJECT_SOURCE_DIR}/../../../Core/DSP/library/includes")
    include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Graph/library/includes")
else()
    file(GLOB_RECURSE PROJECT_HDRS
        ${CMAKE_CURRENT_SOURCE_DIR}/includes/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../JamomaCore/Foundation/library/includes/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../JamomaCore/Modular/library/includes/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../JamomaCore/Modular/library/PeerObject/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../JamomaCore/Modular/library/ProtocolLib/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../JamomaCore/Modular/library/SchedulerLib/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../JamomaCore/Graph/library/includes/*.h
    )
    include_directories("${PROJECT_SOURCE_DIR}/../JamomaCore/Foundation/library/includes")
    include_directories("${PROJECT_SOURCE_DIR}/../JamomaCore/Modular/library/includes")
    include_directories("${PROJECT_SOURCE_DIR}/../JamomaCore/Modular/library/PeerObject")
    include_directories("${PROJECT_SOURCE_DIR}/../JamomaCore/Modular/library/ProtocolLib")
    include_directories("${PROJECT_SOURCE_DIR}/../JamomaCore/Modular/library/SchedulerLib")
    include_directories("${PROJECT_SOURCE_DIR}/../JamomaCore/DSP/library/includes")
    include_directories("${PROJECT_SOURCE_DIR}/../JamomaCore/Graph/library/includes")
endif()

    include_directories("${PROJECT_SOURCE_DIR}/../source/CicmWrapper/Sources")
    include_directories("${PROJECT_SOURCE_DIR}/../source/")


add_library(${PROJECT_NAME} SHARED ${PROJECT_SRCS} ${PROJECT_HDRS})

target_link_libraries(${PROJECT_NAME} Foundation)
target_link_libraries(${PROJECT_NAME} Modular)
target_link_libraries(${PROJECT_NAME} CicmWrapper)

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
