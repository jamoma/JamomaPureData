cmake_minimum_required(VERSION 3.0)
project(pd_j.message)

add_definitions(-DJMOD_MESSAGE)

set(PROJECT_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/../j.parameter/j.parameter.cpp
)

set(PROJECT_SRCS ${PROJECT_SRCS} $<TARGET_OBJECTS:TTModularClassWrapperPd>)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/CMake/PdExternalCommon.cmake)
