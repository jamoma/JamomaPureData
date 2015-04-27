cmake_minimum_required(VERSION 3.0)
project(Jamoma4Pd)

set(BUILD_JAMOMAPD True)

message("Configuring Implementation/PureData build")

if (APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined suppress -flat_namespace")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -undefined suppress -flat_namespace")
endif(APPLE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/Shared/CMake/modules/")
include(${PROJECT_SOURCE_DIR}/Shared/CMake/JamomaConfiguration.cmake NO_POLICY_SCOPE)
include(${PROJECT_SOURCE_DIR}/Shared/CMake/JamomaUtilFunctions.cmake NO_POLICY_SCOPE)

if(NOT JAMOMA_UMBRELLA)
    set(JAMOMAPD_INSTALL_FOLDER "pd-externals")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Jamoma"
        DESTINATION ${JAMOMAPD_INSTALL_FOLDER}
            COMPONENT JamomaPd)
    add_subdirectory(JamomaCore)
endif()

add_subdirectory(library)
add_subdirectory(source)
