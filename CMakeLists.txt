cmake_minimum_required(VERSION 3.0)
project(Jamoma4Pd)

set(BUILD_JAMOMAPD True)

message("Configuring Implementation/PureData build")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/JamomaPd-externals)

if (APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined suppress -flat_namespace")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -undefined suppress -flat_namespace")
endif(APPLE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/Shared/CMake/modules/")
include(${PROJECT_SOURCE_DIR}/Shared/CMake/JamomaConfiguration.cmake NO_POLICY_SCOPE)
include(${PROJECT_SOURCE_DIR}/Shared/CMake/JamomaUtilFunctions.cmake NO_POLICY_SCOPE)

if(NOT JAMOMA_UMBRELLA)

    include(${PROJECT_SOURCE_DIR}/Shared/CMake/JamomaPlatformSpecific.cmake NO_POLICY_SCOPE)

    if (APPLE)
        option(FAT_BINARY "Build a fat binary (32bit and 64bit)" OFF)
    endif(APPLE)
    option(BUILD_JAMOMAPD "Build Jamoma for PureData package" OFF)
    set(JAMOMA_CORE_SRC_PATH "${PROJECT_SOURCE_DIR}/JamomaCore" CACHE STRING "Path to JamomaCore source folder")

    if(FAT_BINARY)
        set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
    endif(FAT_BINARY)

    set(JAMOMAPD_INSTALL_FOLDER "JamomaPd" CACHE STRING "PD Package Install folder")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Jamoma"
        DESTINATION ${JAMOMAPD_INSTALL_FOLDER}
            COMPONENT JamomaPd)
    add_subdirectory(JamomaCore)
else()
    set(JAMOMA_CORE_SRC_PATH "${PROJECT_SOURCE_DIR}/../../Core/")
endif()

find_package(Pd REQUIRED)
include_directories(${PD_INCLUDE_DIRS})
set(LIBS ${LIBS} ${PD_LIBRARY})

add_subdirectory(library)
add_subdirectory(source)
