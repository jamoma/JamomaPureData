cmake_minimum_required(VERSION 3.0)
project(pd_j.parameter_array)

add_definitions(-DARRAY_EXTERNAL)

set(PROJECT_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/j.parameter_array.cpp
)

set(PROJECT_SRCS ${PROJECT_SRCS} $<TARGET_OBJECTS:TTModularClassWrapperPdArray>)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/CMake/PdExternalCommon.cmake)
