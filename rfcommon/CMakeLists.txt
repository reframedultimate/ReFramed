include (TestVisibilityMacros)
include (TestBigEndian)
include (CheckCXXSourceCompiles)

project ("rfcommon"
    VERSION ${REFRAMED_VERSION}
    LANGUAGES CXX C)
    
option (RFCOMMON_REFCOUNTED_DEBUG "Adds members to the rfcommon::RefCounted class to help find memory leaks" OFF)
    
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
    
configure_file ("cmake/templates/config.hpp.in" "include/rfcommon/config.hpp")
configure_file ("cmake/templates/Endian.hpp.in" "include/rfcommon/Endian.hpp")

add_library (rfcommon SHARED
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
    "src/FighterFlags.cpp"
    "src/FighterHitStatus.cpp"
    "src/FighterID.cpp"
    "src/FighterMotion.cpp"
    "src/FighterStatus.cpp"
    "src/FighterStocks.cpp"
    "src/FrameData.cpp"
    "src/FrameIndex.cpp"
    "src/FramesLeft.cpp"
    "src/GameNumber.cpp"
    "src/hash40.cpp"
    "src/Hash40Strings.cpp"
    "src/HashMap.cpp"
    "src/HitstunCalculator.cpp"
    "src/init.c"
    "src/KnockbackCalculator.cpp"
    "src/MappingInfo.cpp"
    "src/MappingInfoFighter.cpp"
    "src/MappingInfoHitStatus.cpp"
    "src/MappingInfoMotion.cpp"
    "src/MappingInfoStage.cpp"
    "src/MappingInfoStatus.cpp"
    "src/MetaData.cpp"
    "src/FighterState.cpp"
    "src/Plugin.cpp"
    "src/RefCounted.cpp"
    "src/SetFormat.cpp"
    "src/SetNumber.cpp"
    "src/Session.cpp"
    "src/StageID.cpp"
    "src/StreamBuffer.cpp"
    "src/TimeStamp.cpp"
    "src/UserLabels.cpp"
    "src/Vector.cpp"
    "src/Vec2.cpp"
    "src/platform/dynlib.c"
    "src/platform/tcp_socket.c"
    "src/platform/time.cpp")
target_include_directories (rfcommon
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:include>)
target_compile_definitions (rfcommon
    PRIVATE RFCOMMON_BUILDING
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options (rfcommon
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4910>)  # '__declspec(dllexport)' and 'extern' are incompatible on an explicit instantation -> MSVC is wrong about this.
target_compile_features (rfcommon
    PUBLIC cxx_std_17)
target_link_libraries (rfcommon 
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE zlibstatic
    PRIVATE cpp-base64)
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

if (UNIX)
    find_package (DL REQUIRED)
    target_link_libraries (rfcommon PRIVATE DL::DL)
endif ()

if (WIN32)
    target_link_libraries (rfcommon
        PRIVATE wsock32
        PRIVATE ws2_32)
endif()

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
