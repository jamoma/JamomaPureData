cmake_minimum_required(VERSION 3.0)
project(PdExternals)
add_definitions(-fPIC)

include_directories("${JAMOMA_CORE_SRC_PATH}/Foundation/library/includes")
include_directories("${JAMOMA_CORE_SRC_PATH}/Modular/library/includes")
include_directories("${JAMOMA_CORE_SRC_PATH}/Modular/library/PeerObject")
include_directories("${JAMOMA_CORE_SRC_PATH}/Modular/library/ModularForPd")
include_directories("${JAMOMA_CORE_SRC_PATH}/Modular/library/SchedulerLib")
include_directories("${JAMOMA_CORE_SRC_PATH}/Modular/library/ProtocolLib")
include_directories("${JAMOMA_CORE_SRC_PATH}/DSP/library/includes")
include_directories("${JAMOMA_CORE_SRC_PATH}/Score/library/includes")

include_directories("${PROJECT_SOURCE_DIR}/classwrapper")
include_directories("${PROJECT_SOURCE_DIR}/CicmWrapper/Sources")
include_directories("${PROJECT_SOURCE_DIR}/../library/includes")
include_directories(${PROJECT_SOURCE_DIR})

# link_directories("${PROJECT_SOURCE_DIR}/CicmWrapper/Sources")

add_library(PdCommonSyms OBJECT
			${CMAKE_CURRENT_SOURCE_DIR}/commonsyms.cpp)
cotire(PdCommonSyms)
set_target_properties(PdCommonSyms PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH "")

add_library(TTClassWrapperPd OBJECT
                        ${CMAKE_CURRENT_SOURCE_DIR}/classwrapper/TTClassWrapperPd.cpp)
cotire(TTClassWrapperPd)
set_target_properties(TTClassWrapperPd PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH "")

add_library(TTModularClassWrapperPd OBJECT
                        ${CMAKE_CURRENT_SOURCE_DIR}/classwrapper/TTModularClassWrapperPd.cpp)
cotire(TTModularClassWrapperPd)
set_target_properties(TTModularClassWrapperPd PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH "")

add_library(TTModularClassWrapperPdArray OBJECT
                        ${CMAKE_CURRENT_SOURCE_DIR}/classwrapper/TTModularClassWrapperPd.cpp)

set_property(TARGET TTModularClassWrapperPdArray
                         PROPERTY COMPILE_DEFINITIONS ARRAY_EXTERNAL)
cotire(TTModularClassWrapperPdArray)
set_target_properties(TTModularClassWrapperPdArray PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH "")

#addMaxsupport()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

cmake_policy(SET CMP0014 OLD) # don't warn if folder doesn't contain a CMakeLists.txt file	
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
SET(IS_EXTERNAL 1)
FOREACH(subdir ${SUBDIRS})
    ADD_SUBDIRECTORY(${subdir})
ENDFOREACH()
