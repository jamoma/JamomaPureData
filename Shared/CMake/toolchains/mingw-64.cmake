#
# CMake defines to cross-compile to Windows
#

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)
SET(PC_LIBXML_INCLUDEDIR "${JAMOMA_CORE_SRC_PATH}/Foundation/library/libxml2/win32/libmingw-w64-32bit/include")
SET(PC_LIBXML_LIBDIR "${JAMOMA_CORE_SRC_PATH}/Foundation/library/libxml2/win32/libmingw-w64-32bit/lib")
# which compilers to use for C and C++
SET(CMAKE_C_COMPILER   ${CROSS_COMPILER_PATH}/bin/x86_64-w64-mingw32-gcc -m32)
SET(CMAKE_CXX_COMPILER ${CROSS_COMPILER_PATH}/bin/x86_64-w64-mingw32-g++ -m32)
SET(CMAKE_RC_COMPILER  ${CROSS_COMPILER_PATH}/bin/x86_64-w64-mingw32-windres)

# here is the target environment located
#SET(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc /home/alex/mingw-install )
SET(CMAKE_FIND_ROOT_PATH ${CROSS_COMPILER_PATH}/x86_64-w64-mingw32
                         ${JAMOMA_CORE_SRC_PATH}/Foundation/library/libxml2/win32/libmingw-w64-32bit
                         ${JAMOMA_CORE_SRC_PATH}/Foundation/library/libiconv2 )

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_definitions(-DTT_PLATFORM_WIN)
add_definitions(-DTTSTATIC)

install(FILES 
${CROSS_COMPILER_PATH}/x86_64-w64-mingw32/lib32/libstdc++-6.dll
${CROSS_COMPILER_PATH}/x86_64-w64-mingw32/lib32/libgcc_s_sjlj-1.dll
              ${JAMOMA_CORE_SRC_PATH}/Foundation/library/libxml2/win32/libmingw-w64-32bit/bin/libxml2-2.dll
        DESTINATION ${JAMOMAPD_INSTALL_FOLDER}/Jamoma)
