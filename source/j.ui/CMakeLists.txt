cmake_minimum_required(VERSION 3.0)
#[[
project(pd_j.ui)

set(PROJECT_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/j.ui.cpp
${CMAKE_CURRENT_SOURCE_DIR}/j.ui.internals.cpp
${CMAKE_CURRENT_SOURCE_DIR}/j.ui.presets.cpp
${CMAKE_CURRENT_SOURCE_DIR}/TTUiInfo.cpp
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/CMake/PdExternalCommon.cmake)
]]
