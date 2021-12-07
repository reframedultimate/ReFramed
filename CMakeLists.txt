cmake_minimum_required (VERSION 3.8.0)

cmake_policy (SET CMP0074 NEW)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take their
# definition and dump it in the cache along with proper documentation, otherwise
# set MAKE_BUILD_TYPE to Debug prior to calling PROJECT()
if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()
message (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

project ("ultimate-hindsight"
    LANGUAGES)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Sets the version string for all UH related subprojects (not thirdparty)
set (UH_VERSION "0.0.1")
set (UH_VERSION_RC "alpha")

# Configure build and install directory structures
set (UH_BUILD_BINDIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
set (UH_BUILD_LIBDIR "${UH_BUILD_BINDIR}/lib")
set (UH_BUILD_PLUGINDIR "${UH_BUILD_BINDIR}/share/uh/plugins")

set (UH_INSTALL_BINDIR ".")
set (UH_INSTALL_LIBDIR "lib")
set (UH_INSTALL_INCLUDEDIR "include")
set (UH_INSTALL_DATADIR "share/uh")
set (UH_INSTALL_PLUGINDIR "share/uh/plugins")
set (UH_INSTALL_CMAKEDIR "share/uh/cmake")

# Need to compile static libraries with position independent code on because
# libuh is a shared library
set (CPP_BASE64_PIC ON CACHE INTERNAL "")
set (ZLIB_PIC ON CACHE INTERNAL "")

# Don't install thirdparty libs
set (INSTALL_GTEST OFF CACHE INTERNAL "")
set (gtest_force_shared_crt ON CACHE INTERNAL "")

add_subdirectory ("thirdparty/qwt-6.1.6")
add_subdirectory ("thirdparty/googletest-1.10.0")
add_subdirectory ("thirdparty/json")
add_subdirectory ("thirdparty/zlib-1.2.11")
add_subdirectory ("thirdparty/cpp-base64")
add_subdirectory ("uh")
add_subdirectory ("uhplot")
add_subdirectory ("plugins")
add_subdirectory ("application")
add_subdirectory ("tests")

# Workaround because uic generates headers into CMAKE_CURRENT_BINARY_DIR and
# we want to include them from <uh/ui_*.h>
target_include_directories (ultimate-hindsight
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

set_target_properties (gmock
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${UH_BUILD_LIBDIR}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${UH_BUILD_LIBDIR}
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${UH_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})

# Why is this required (who knows), prevent PDB files from being written to a "Debug" folder
set_target_properties (gmock
    PROPERTIES
        PDB_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
        PDB_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
        PDB_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})
set_target_properties (gtest
    PROPERTIES
        PDB_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
        PDB_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
        PDB_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})
set_target_properties (gmock_main
    PROPERTIES
        PDB_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
        PDB_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
        PDB_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})
set_target_properties (gtest_main
    PROPERTIES
        PDB_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
        PDB_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
        PDB_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})

