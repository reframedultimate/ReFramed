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

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

project ("ReFramed"
    LANGUAGES)

# Sets the version string for all ReFramed related subprojects (not thirdparty)
set (REFRAMED_VERSION "0.0.8")
set (REFRAMED_VERSION_RC "alpha")

# Configure build and install directory structures
set (REFRAMED_BUILD_BINDIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
set (REFRAMED_BUILD_LIBDIR "${REFRAMED_BUILD_BINDIR}/lib")
set (REFRAMED_BUILD_DATADIR "${REFRAMED_BUILD_BINDIR}/share/reframed/data")
set (REFRAMED_BUILD_PLUGINDIR "${REFRAMED_BUILD_BINDIR}/share/reframed/plugins")

set (REFRAMED_INSTALL_BINDIR ".")
set (REFRAMED_INSTALL_LIBDIR "lib")
set (REFRAMED_INSTALL_INCLUDEDIR "include")
set (REFRAMED_INSTALL_DATADIR "share/reframed/data")
set (REFRAMED_INSTALL_PLUGINDIR "share/reframed/plugins")
set (REFRAMED_INSTALL_CMAKEDIR "share/reframed/cmake")

# Need to compile static libraries with position independent code on because
# rfcommon is a shared library
set (CPP_BASE64_PIC ON CACHE INTERNAL "")
set (ZLIB_PIC ON CACHE INTERNAL "")

# Don't install thirdparty libs
set (INSTALL_GTEST OFF CACHE INTERNAL "")
set (gtest_force_shared_crt ON CACHE INTERNAL "")

add_subdirectory ("thirdparty/qwt-6.2.0")
add_subdirectory ("thirdparty/googletest-1.12.1")
add_subdirectory ("thirdparty/json")
add_subdirectory ("thirdparty/zlib-1.2.11")
add_subdirectory ("thirdparty/cpp-base64")
add_subdirectory ("rfcommon")
add_subdirectory ("rfplot")
add_subdirectory ("application")
add_subdirectory ("tests")
add_subdirectory ("plugins")

# set_target_properties (gmock
#     PROPERTIES
#         ARCHIVE_OUTPUT_DIRECTORY ${UH_BUILD_LIBDIR}
#         ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${UH_BUILD_LIBDIR}
#         ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${UH_BUILD_LIBDIR}
#         LIBRARY_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
#         LIBRARY_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
#         LIBRARY_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR}
#         RUNTIME_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
#         RUNTIME_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
#         RUNTIME_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})
#
# # Why is this required (who knows), prevent PDB files from being written to a "Debug" folder
# set_target_properties (gmock
#     PROPERTIES
#         PDB_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
#         PDB_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
#         PDB_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})
# set_target_properties (gtest
#     PROPERTIES
#         PDB_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
#         PDB_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
#         PDB_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})
# set_target_properties (gmock_main
#     PROPERTIES
#         PDB_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
#         PDB_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
#         PDB_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})
# set_target_properties (gtest_main
#     PROPERTIES
#         PDB_OUTPUT_DIRECTORY ${UH_INSTALL_BINDIR}
#         PDB_OUTPUT_DIRECTORY_DEBUG ${UH_INSTALL_BINDIR}
#         PDB_OUTPUT_DIRECTORY_RELEASE ${UH_INSTALL_BINDIR})
#

# All subprojects have added targets to this export
# group, so make sure to install it
install (
    EXPORT ReFramedTargets
    NAMESPACE ReFramed::
    DESTINATION ${REFRAMED_INSTALL_CMAKEDIR})

install (
    DIRECTORY "cmake/modules/"
    DESTINATION ${REFRAMED_INSTALL_CMAKEDIR}/modules)

include(CMakePackageConfigHelpers)
configure_package_config_file("cmake/ReFramedConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ReFramedConfig.cmake"
    INSTALL_DESTINATION "${REFRAMED_INSTALL_CMAKEDIR}")
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/ReFramedConfig.cmake"
    DESTINATION ${REFRAMED_INSTALL_CMAKEDIR})
