include (TestVisibilityMacros)
include (TestBigEndian)
include (CheckCXXSourceCompiles)

project ("uh"
    VERSION 0.0.1
    LANGUAGES CXX C)
    
option (UH_REFCOUNTED_DEBUG "Adds members to the uh::RefCounted class to help find memory leaks" OFF)
    
test_visibility_macros (
    UH_API_EXPORT 
    UH_API_IMPORT 
    UH_API_LOCAL)

# We need to define a special macro for exporting template instantiations for
# Windows DLLs. MacOSX and Linux don't care about this.
if (WIN32)
    set (UH_TEMPLATE_EXPORT ${UH_API_EXPORT})
    set (UH_TEMPLATE_IMPORT ${UH_API_IMPORT})
endif ()

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
    check_endian_intrinsic("__builtin_bswap16" "generic_bswap16" "" UH_SWAP_ENDIAN_16)
    check_endian_intrinsic("__builtin_bswap32" "generic_bswap32" "" UH_SWAP_ENDIAN_32)
    check_endian_intrinsic("__builtin_bswap64" "generic_bswap64" "" UH_SWAP_ENDIAN_64)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    check_endian_intrinsic("_byteswap_ushort" "generic_bswap16" "intrin.h" UH_SWAP_ENDIAN_16)
    check_endian_intrinsic("_byteswap_ulong"  "generic_bswap32" "intrin.h" UH_SWAP_ENDIAN_32)
    check_endian_intrinsic("_byteswap_uint64" "generic_bswap64" "intrin.h" UH_SWAP_ENDIAN_64)
else ()
    message (FATAL_ERROR "Endian byteswap check was not implemented for this compiler (yet). Please check the above cmake code and see if you can implement it, or submit an issue")
endif ()
    
configure_file ("templates/config.hpp.in" "include/uh/config.hpp")
configure_file ("templates/Endian.hpp.in" "include/uh/Endian.hpp")

add_library (uh SHARED
    "src/ActiveRecording.cpp"
    "src/AnalyzerPlugin.cpp"
    "src/AnalysisResult.cpp"
    "src/crc32.c"
    "src/DataPoint.cpp"
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
    "src/DecisionGraph.cpp"
    "src/FighterIDMapping.cpp"
    "src/FighterStatusMapping.cpp"
    "src/hash40.c"
    "src/HashMap.cpp"
    "src/HitStatusMapping.cpp"
    "src/init.c"
    "src/PlayerState.cpp"
    "src/Recording.cpp"
    "src/RefCounted.cpp"
    "src/SavedRecording.cpp"
    "src/SetFormat.cpp"
    "src/StageIDMapping.cpp"
    "src/StreamBuffer.cpp"
    "src/TemplateInstantiations.cpp"
    "src/TrainingModeContext.cpp"
    "src/Vector.cpp"
    "src/platform/dynlib.c"
    "src/platform/tcp_socket.c"
    "src/platform/time.cpp")
target_include_directories (uh
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:include>)
target_compile_definitions (uh
    PRIVATE UH_BUILDING
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options (uh
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4910>)  # '__declspec(dllexport)' and 'extern' are incompatible on an explicit instantation -> MSVC is wrong about this.
target_compile_features (uh
    PUBLIC cxx_std_17)
target_link_libraries (uh 
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE zlibstatic
    PRIVATE cpp-base64)
set_target_properties (uh
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${UH_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${UH_BUILD_BINDIR})

if (UNIX)
    find_package (DL REQUIRED)
    target_link_libraries (uh PRIVATE DL::DL)
endif ()

if (WIN32)
    target_link_libraries (uh
        PRIVATE wsock32
        PRIVATE ws2_32)
endif()

install (
    TARGETS uh
    EXPORT UHConfig
    LIBRARY DESTINATION ${UH_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${UH_INSTALL_BINDIR})
install (
    DIRECTORY "include/uh"
    DESTINATION ${UH_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/uh"
    DESTINATION ${UH_INSTALL_INCLUDEDIR})
install (
    EXPORT UHConfig
    DESTINATION ${UH_INSTALL_CMAKEDIR})

