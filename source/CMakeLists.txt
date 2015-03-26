cmake_minimum_required(VERSION 3.0)
project(PdExternals)
add_definitions(-fPIC)

include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Foundation/library/includes")
include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Modular/library/includes")
include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Modular/library/PeerObject")
include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Modular/library/ModularForPd")
include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Modular/library/SchedulerLib")
include_directories("${PROJECT_SOURCE_DIR}/../../../Core/Modular/library/ProtocolLib")
include_directories("${PROJECT_SOURCE_DIR}/../../../Core/DSP/library/includes")
include_directories("${PROJECT_SOURCE_DIR}/classwrapper")
include_directories("${PROJECT_SOURCE_DIR}/CicmWrapper/Sources")
include_directories("${PROJECT_SOURCE_DIR}/../library/includes")
include_directories(${PROJECT_SOURCE_DIR})

# link_directories("${PROJECT_SOURCE_DIR}/CicmWrapper/Sources")

add_library(PdCommonSyms OBJECT
			${CMAKE_CURRENT_SOURCE_DIR}/commonsyms.cpp)

add_library(TTClassWrapperPd OBJECT
                        ${CMAKE_CURRENT_SOURCE_DIR}/classwrapper/TTClassWrapperPd.cpp)

add_library(TTModularClassWrapperPd OBJECT
                        ${CMAKE_CURRENT_SOURCE_DIR}/classwrapper/TTModularClassWrapperPd.cpp)

add_library(TTModularClassWrapperPdArray OBJECT
                        ${CMAKE_CURRENT_SOURCE_DIR}/classwrapper/TTModularClassWrapperPd.cpp)

set_property(TARGET TTModularClassWrapperPdArray
                         PROPERTY COMPILE_DEFINITIONS ARRAY_EXTERNAL)

#addMaxsupport()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

cmake_policy(SET CMP0014 OLD) # don't warn if folder doesn't contain a CMakeLists.txt file	
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
SET(IS_EXTERNAL 1)
FOREACH(subdir ${SUBDIRS})
    ADD_SUBDIRECTORY(${subdir})
ENDFOREACH()
