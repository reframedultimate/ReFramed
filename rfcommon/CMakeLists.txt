include (TestVisibilityMacros)
include (TestBigEndian)
include (CheckCXXSourceCompiles)
include (CMakeDependentOption)

project ("rfcommon"
    VERSION ${REFRAMED_VERSION}
    LANGUAGES CXX C)

option (RFCOMMON_LOGGING "Enable logging functionality in the entire application" ON)
option (RFCOMMON_PROFILER "Do inline profiling on functions. May slightly decrease performance." OFF)
option (RFCOMMON_REFCOUNTED_DEBUG "Adds members to the rfcommon::RefCounted class to help find memory leaks" OFF)
set (RFCOMMON_LOG_LEVEL "DEBUG" CACHE STRING "Any log calls under this level are optimized out")
set_property (CACHE RFCOMMON_LOG_LEVEL PROPERTY STRINGS "DEBUG;INFO;NOTICE;WARNING;ERROR;FATAL")

###############################################################################
# Config
###############################################################################

# Determine platform
if (WIN32 OR CYGWIN)
    set (RFCOMMON_PLATFORM "WINDOWS")
elseif (APPLE)
    set (RFCOMMON_PLATFORM "OSX")
elseif (UNIX)
    set (RFCOMMON_PLATFORM "LINUX")
else ()
    message (FATAL_ERROR "Platform detection failed")
endif ()

# Determine bits
if (${CMAKE_SIZEOF_VOID_P} STREQUAL "4")
    set (RFCOMMON_BITS "32")
elseif (${CMAKE_SIZEOF_VOID_P} STREQUAL "8")
    set (RFCOMMON_BITS "64")
else ()
    message (FATAL_ERROR "Unsupported number of bits")
endif ()
    
test_visibility_macros (
    RFCOMMON_API_EXPORT 
    RFCOMMON_API_IMPORT 
    RFCOMMON_API_LOCAL)

# check endianness of system
test_big_endian (I_AM_BIG)

# configure endian swap intrinsics
# Checks if an intrinsic symbol exists and compiles. If it compiles
# successfully, then the symbol is stored in RET_SYMBOL. If compilation
# fails, then FALLBACK_SYMBOL is stored in RET_SYMBOL instead.
# HEADER is an optional string containing a header file to include.
macro (check_endian_intrinsic SYMBOL FALLBACK_SYMBOL HEADER RET_SYMBOL)
    if (NOT "${HEADER}" STREQUAL "")
        set (DO_INCLUDE "#include <${HEADER}>")
    else ()
        set (DO_INCLUDE "")
    endif ()

    check_cxx_source_compiles ("
        ${DO_INCLUDE}
        int main() {
            (void)${SYMBOL}(1);
        }" ${SYMBOL}_RESULT)

    if (${SYMBOL}_RESULT)
        set (${RET_SYMBOL} ${SYMBOL})
    else ()
        set (${RET_SYMBOL} ${FALLBACK_SYMBOL})
    endif ()
endmacro ()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    check_endian_intrinsic("__builtin_bswap16" "generic_bswap16" "" RFCOMMON_SWAP_ENDIAN_16)
    check_endian_intrinsic("__builtin_bswap32" "generic_bswap32" "" RFCOMMON_SWAP_ENDIAN_32)
    check_endian_intrinsic("__builtin_bswap64" "generic_bswap64" "" RFCOMMON_SWAP_ENDIAN_64)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    check_endian_intrinsic("_byteswap_ushort" "generic_bswap16" "intrin.h" RFCOMMON_SWAP_ENDIAN_16)
    check_endian_intrinsic("_byteswap_ulong"  "generic_bswap32" "intrin.h" RFCOMMON_SWAP_ENDIAN_32)
    check_endian_intrinsic("_byteswap_uint64" "generic_bswap64" "intrin.h" RFCOMMON_SWAP_ENDIAN_64)
else ()
    message (FATAL_ERROR "Endian byteswap check was not implemented for this compiler (yet). Please check the above cmake code and see if you can implement it, or submit an issue")
endif ()

# Determine host computer information
set (RFCOMMON_HOST_COMPUTER ${CMAKE_HOST_SYSTEM})
set (RFCOMMON_COMPILER ${CMAKE_CXX_COMPILER_ID})
find_program (UNAME_PROGRAM uname)
if (UNAME_PROGRAM)
    execute_process (
        COMMAND ${UNAME_PROGRAM} -o -r -m -p -i
        OUTPUT_VARIABLE RFCOMMON_HOST_COMPUTER)
    string (REPLACE "\n" "" RFCOMMON_HOST_COMPUTER ${RFCOMMON_HOST_COMPUTER})
else ()
    set (RFCOMMON_HOST_COMPUTER ${CMAKE_HOST_SYSTEM_NAME})
endif ()

# Required for build info
find_package (Git)
if (GIT_FOUND)
    execute_process (COMMAND ${GIT_EXECUTABLE} describe --tags RESULT_VARIABLE RESULT OUTPUT_VARIABLE COMMIT_TAG)
    if (RESULT)
        set (RFCOMMIT_TAG "(unknown tag)")
    else ()
        string (REPLACE "\n" "" COMMIT_TAG ${COMMIT_TAG})
    endif ()
    execute_process (COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD RESULT_VARIABLE RESULT OUTPUT_VARIABLE COMMIT_HASH)
    if (RESULT)
        set (RFCOMMIT_HASH "(commit hash unknown)")
    else ()
        string (REPLACE "\n" "" COMMIT_HASH ${COMMIT_HASH})
    endif ()
    set (RFCOMMON_COMMIT_INFO "${COMMIT_TAG} (${COMMIT_HASH})")
else ()
    set (RFCOMMON_COMMIT_INFO "unknown (git not found!)")
    message (WARNING "Git not found. Build will not contain git revision info.")
endif ()
    
configure_file ("cmake/templates/config.hpp.in" "include/rfcommon/config.hpp")
configure_file ("cmake/templates/Endian.hpp.in" "include/rfcommon/Endian.hpp")
configure_file ("cmake/templates/BuildInfo.cpp.in" "src/BuildInfo.cpp")

###############################################################################
# source files and library definition
###############################################################################

add_library (rfcommon SHARED
    "cmake/templates/config.hpp.in"
    "cmake/templates/Endian.hpp.in"
    "cmake/templates/BuildInfo.cpp.in"

    "include/rfcommon/Utf8.hpp"

    "src/AnalysisResult.cpp"
    "src/crc32.c"
    "src/DataSet.cpp"
    "src/DataSetFilter.cpp"
    "src/DataSetFilter_Date.cpp"
    "src/DataSetFilter_Game.cpp"
    "src/DataSetFilter_Matchup.cpp"
    "src/DataSetFilter_Player.cpp"
    "src/DataSetFilter_PlayerCount.cpp"
    "src/DataSetFilter_Stage.cpp"
    "src/DataSetFilterChain.cpp"
    "src/DataSetProcessor.cpp"
    "src/DeltaTime.cpp"
    "src/Deserializer.cpp"
    "src/EventType.cpp"
    "src/FighterFlags.cpp"
    "src/FighterHitStatus.cpp"
    "src/FighterID.cpp"
    "src/FighterMotion.cpp"
    "src/FighterState.cpp"
    "src/FighterStatus.cpp"
    "src/FighterStocks.cpp"
    "src/FrameData.cpp"
    "src/FrameIndex.cpp"
    "src/FramesLeft.cpp"
    "src/GameMetaData.cpp"
    "src/GameNumber.cpp"
    "src/hash40.cpp"
    "src/Hash40Strings.cpp"
    "src/HashMap.cpp"
    "src/HitstunCalculator.cpp"
    "src/init.cpp"
    "src/Log.cpp"
    "src/KnockbackCalculator.cpp"
    "src/MappingInfo.cpp"
    "src/MappingInfoFighter.cpp"
    "src/MappingInfoHitStatus.cpp"
    "src/MappingInfoStage.cpp"
    "src/MappingInfoStatus.cpp"
    "src/MemoryBuffer.cpp"
    "src/MetaData.cpp"
    "src/Plugin.cpp"
    $<$<BOOL:${RFCOMMON_PROFILER}>:src/Profiler.cpp>
    $<$<BOOL:${RFCOMMON_PROFILER}>:src/Profiler_exportDOT.cpp>
    "src/RefCounted.cpp"
    "src/ReplayFileParts.cpp"
    "src/Round.cpp"
    "src/ScoreCount.cpp"
    "src/SetFormat.cpp"
    "src/Serializer.cpp"
    "src/Session.cpp"
    "src/SessionNumber.cpp"
    "src/StageID.cpp"
    "src/TimeStamp.cpp"
    "src/TrainingMetaData.cpp"
    "src/UserMotionLabels.cpp"
    "src/Vector.cpp"
    "src/Vec2.cpp"
    "src/VideoEmbed.cpp"
    "src/VideoMeta.cpp"
    "src/VisualizerContext.cpp"
    "src/VisualizerData.cpp"
    "${PROJECT_BINARY_DIR}/src/BuildInfo.cpp"
    $<$<STREQUAL:${RFCOMMON_PLATFORM},WINDOWS>:src/platform/windows/SharedLibrary_windows.cpp>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},WINDOWS>:src/platform/windows/HighresTimer_windows.cpp>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},WINDOWS>:src/platform/windows/LastWindowsError.cpp>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},WINDOWS>:src/platform/windows/MappedFile_windows.cpp>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},WINDOWS>:src/platform/windows/tcp_socket_windows.c>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},WINDOWS>:src/platform/windows/Time_windows.cpp>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},WINDOWS>:src/platform/windows/Utf8_windows.cpp>

    $<$<STREQUAL:${RFCOMMON_PLATFORM},LINUX>:src/platform/linux/SharedLibrary_linux.cpp>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},LINUX>:src/platform/linux/HighresTimer_linux.cpp>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},LINUX>:src/platform/linux/MappedFile_linux.cpp>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},LINUX>:src/platform/linux/tcp_socket_linux.c>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},LINUX>:src/platform/linux/Time_linux.cpp>
    $<$<STREQUAL:${RFCOMMON_PLATFORM},LINUX>:src/platform/linux/Utf8_linux.cpp>)
target_include_directories (rfcommon
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_compile_definitions (rfcommon
    PRIVATE
        RFCOMMON_BUILDING
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options (rfcommon
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>)
target_compile_features (rfcommon
    PUBLIC cxx_std_17)
target_link_libraries (rfcommon 
    PRIVATE
        nlohmann_json::nlohmann_json
        zlibstatic
        cpp-base64)
set_target_properties (rfcommon
    PROPERTIES
        DEBUG_POSTFIX "d"
        LIBRARY_OUTPUT_DIRECTORY ${REFRAMED_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${REFRAMED_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${REFRAMED_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${REFRAMED_BUILD_BINDIR})

add_library (ReFramed::rfcommon ALIAS rfcommon)

###############################################################################
# Dependencies
###############################################################################

if (UNIX)
    target_link_libraries (rfcommon
        PRIVATE ${CMAKE_DL_LIBS})
endif ()

if (WIN32)
    target_link_libraries (rfcommon
        PRIVATE wsock32
        PRIVATE ws2_32)
endif()

###############################################################################
# Install targets
###############################################################################

install (
    TARGETS rfcommon
    EXPORT ReFramedTargets
    INCLUDES DESTINATION ${REFRAMED_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${REFRAMED_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${REFRAMED_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${REFRAMED_INSTALL_BINDIR})
install (
    DIRECTORY "include/rfcommon"
    DESTINATION ${REFRAMED_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/rfcommon"
    DESTINATION ${REFRAMED_INSTALL_INCLUDEDIR})
export (
    TARGETS rfcommon
    NAMESPACE ReFramed::
    FILE ReFramedTargets.cmake)
