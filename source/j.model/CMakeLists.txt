cmake_minimum_required(VERSION 3.0)
project(pd_j.model)

set(PROJECT_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/j.model.cpp
${CMAKE_CURRENT_SOURCE_DIR}/j.model_preset.cpp
${CMAKE_CURRENT_SOURCE_DIR}/j.model_signal.cpp
${CMAKE_CURRENT_SOURCE_DIR}/TTModelInfo.cpp
)

set(PROJECT_SRCS ${PROJECT_SRCS} $<TARGET_OBJECTS:TTModularClassWrapperPd>)
add_definitions(-DJCOM_MODEL)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/CMake/PdExternalCommon.cmake)
