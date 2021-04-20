cmake_minimum_required(VERSION 3.1)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zlib
	VERSION 1.2.11
    LANGUAGES C)

set(VERSION "1.2.11")

option(ASM686 "Enable building i686 assembly implementation")
option(AMD64 "Enable building amd64 assembly implementation")

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")
set(INSTALL_MAN_DIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE PATH "Installation directory for manual pages")
set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)

#
# Check to see if we have large file support
#
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)
# We add these other definitions here because CheckTypeSize.cmake
# in CMake 2.4.x does not automatically do so and we want
# compatibility with CMake 2.4.x.
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()
check_type_size(off64_t OFF64_T)
set(CMAKE_REQUIRED_DEFINITIONS) # clear variable

#
# Check for unistd.h
#
check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
    # If we're doing an out of source build and the user has a zconf.h
    # in their source tree...
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h)
        message(STATUS "Renaming")
        message(STATUS "    ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h")
        message(STATUS "to 'zconf.h.included' because this file is included with zlib")
        message(STATUS "but CMake generates it automatically in the build directory.")
        file(RENAME ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h ${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.included)
  endif()
endif()

set(ZLIB_PC ${CMAKE_CURRENT_BINARY_DIR}/zlib.pc)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/zlib.pc.cmakein
		${ZLIB_PC} @ONLY)
configure_file("zconf.h.cmakein" "zconf.h" @ONLY)

#============================================================================
# zlib
#============================================================================

add_library(zlibstatic STATIC
	"src/adler32.c"
	"src/compress.c"
	"src/crc32.c"
	"src/deflate.c"
	"src/gzclose.c"
	"src/gzlib.c"
	"src/gzread.c"
	"src/gzwrite.c"
	"src/infback.c"
	"src/inffast.c"
	"src/inflate.c"
	"src/inftrees.c"
	"src/trees.c"
	"src/uncompr.c"
	"src/zutil.c")

target_compile_definitions(zlibstatic 
	PRIVATE $<$<BOOL:${HAVE_OFF64_T}>:_LARGEFILE64_SOURCE=1>)

target_include_directories(zlibstatic 
	PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/public>
	PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/private>
	PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>)

if(CMAKE_COMPILER_IS_GNUCC)
    if(ASM686)
        set(ZLIB_ASMS "contrib/asm686/match.S")
    elseif (AMD64)
        set(ZLIB_ASMS "contrib/amd64/amd64-match.S")
    endif ()

	if(ZLIB_ASMS)
		target_sources(zlibstatic PRIVATE ${ZLIB_ASMS})
		target_compile_definitions(zlibstatic PRIVATE DASMV)
        set_source_files_properties(${ZLIB_ASMS} PROPERTIES LANGUAGE C COMPILE_FLAGS -DNO_UNDERLINE)
	endif()
endif()

if(MSVC)
    if(ASM686)
		enable_language(ASM_MASM)
        set(ZLIB_ASMS
			"contrib/masmx86/inffas32.asm"
			"contrib/masmx86/match686.asm")
    elseif (AMD64)
		enable_language(ASM_MASM)
        set(ZLIB_ASMS
			"contrib/masmx64/gvmat64.asm"
			"contrib/masmx64/inffasx64.asm")
    endif()

	if(ZLIB_ASMS)
		target_sources(zlibstatic PRIVATE ${ZLIB_ASMS})
		target_compile_definitions(ASMV ASMINF)
	endif()
endif()

# parse the full version number from zlib.h and include in ZLIB_FULL_VERSION
file(READ ${PROJECT_SOURCE_DIR}/include/public/zlib.h _zlib_h_contents)
string(REGEX REPLACE ".*#define[ \t]+ZLIB_VERSION[ \t]+\"([-0-9A-Za-z.]+)\".*"
    "\\1" ZLIB_FULL_VERSION ${_zlib_h_contents})

if(UNIX)
    # On unix-like platforms the library is almost always called libz
   set_target_properties(zlibstatic PROPERTIES OUTPUT_NAME z)
   if(NOT APPLE)
     set_target_properties(zlibstatic PROPERTIES LINK_FLAGS "-Wl,--version-script,\"${PROJECT_SOURCE_DIR}/zlib.map\"")
   endif()
endif ()

if(MSVC)
	set_target_properties(zlibstatic
		PROPERTIES DEBUG_POSTFIX "d")
	target_compile_definitions(zlibstatic
		PRIVATE _CRT_SECURE_NO_DEPRECATE 
		PRIVATE _CRT_NONSTDC_NO_DEPRECATE)
endif()

