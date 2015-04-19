cmake_minimum_required(VERSION 3.0)
project(pd_j.receive)

set(PROJECT_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/j.receive.cpp
)

set(PROJECT_SRCS ${PROJECT_SRCS} $<TARGET_OBJECTS:TTModularClassWrapperPd>)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../Shared/CMake/PdExternalCommon.cmake)

install(TARGETS ${PROJECT_NAME}
           DESTINATION "${JAMOMAMAX_MODULAR_INSTALL_FOLDER}/Jamoma/externals"
           COMPONENT JamomaPdModularOnly)
