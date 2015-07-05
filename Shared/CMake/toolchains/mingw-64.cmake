#
# CMake defines to cross-compile to Windows
#

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc-posix)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++-posix)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

# here is the target environment located
#SET(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc /home/alex/mingw-install )
SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32/
                         ${JAMOMA_CORE_SRC_PATH}/Foundation/library/libxml2/win32/libmingw-w64
                         ${JAMOMA_CORE_SRC_PATH}/Foundation/library/libiconv2 )

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_definitions(-DTT_PLATFORM_WIN)
add_definitions(-DTTSTATIC)

install(FILES /usr/lib/gcc/i686-w64-mingw32/4.9-posix/libstdc++-6.dll
              /usr/lib/gcc/i686-w64-mingw32/4.9-posix/libgcc_s_sjlj-1.dll
              ${JAMOMA_CORE_SRC_PATH}/Foundation/library/libxml2/win32/libmingw-w64/bin/libxml2-2.dll
        DESTINATION ${JAMOMAPD_INSTALL_FOLDER}/Jamoma)
