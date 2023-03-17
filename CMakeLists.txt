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
set (REFRAMED_VERSION "0.0.13")
set (REFRAMED_VERSION_RC "alpha")

# Some modules need to know where the root source directory is located
set (REFRAMED_SOURCE_ROOT ${PROJECT_SOURCE_DIR})

# Configure build and install directory structures
set (REFRAMED_BUILD_BINDIR "${PROJECT_BINARY_DIR}/bin")
set (REFRAMED_BUILD_LIBDIR "${REFRAMED_BUILD_BINDIR}/lib")
set (REFRAMED_BUILD_DATADIR "${REFRAMED_BUILD_BINDIR}/share/reframed/data")
set (REFRAMED_BUILD_PLUGINDIR "${REFRAMED_BUILD_BINDIR}/share/reframed/plugins")
set (REFRAMED_BUILD_PLUGINDEPSDIR "${REFRAMED_BUILD_BINDIR}/share/reframed/plugindeps")
set (REFRAMED_BUILD_CMAKEDIR "${REFRAMED_BUILD_BINDIR}/share/reframed/cmake")
set (REFRAMED_BUILD_DOCDIR "${REFRAMED_BUILD_BINDIR}/doc")

set (REFRAMED_INSTALL_BINDIR ".")
set (REFRAMED_INSTALL_LIBDIR "lib")
set (REFRAMED_INSTALL_INCLUDEDIR "include")
set (REFRAMED_INSTALL_DATADIR "share/reframed/data")
set (REFRAMED_INSTALL_PLUGINDIR "share/reframed/plugins")
set (REFRAMED_INSTALL_PLUGINDEPSDIR "share/reframed/plugindeps")
set (REFRAMED_INSTALL_CMAKEDIR "share/reframed/cmake")
set (REFRAMED_INSTALL_DOCDIR "doc")

# Need to compile static libraries with position independent code on because
# rfcommon is a shared library
set (CPP_BASE64_PIC ON CACHE INTERNAL "")
set (ZLIB_PIC ON CACHE INTERNAL "")

# Don't install thirdparty libs
set (INSTALL_GTEST OFF CACHE INTERNAL "")
set (gtest_force_shared_crt ON CACHE INTERNAL "")

add_subdirectory ("thirdparty/json")
add_subdirectory ("thirdparty/zlib-1.2.11")
add_subdirectory ("thirdparty/qwt-6.2.0")
add_subdirectory ("thirdparty/cpp-base64")
add_subdirectory ("thirdparty/ads")
add_subdirectory ("thirdparty/htv")
add_subdirectory ("rfcommon")
add_subdirectory ("rfplot")
add_subdirectory ("application")
add_subdirectory ("plugins")
add_subdirectory ("doc/gen-readme")
add_subdirectory ("doc/user-manual")

option (REFRAMED_TESTS "Enable building unit tests" ON)
if (REFRAMED_TESTS)
    add_subdirectory ("thirdparty/googletest-1.12.1")
    add_subdirectory ("tests")
endif ()

option (REFRAMED_LINT "Run the 'lint.py' script as part of the build process" ON)
if (REFRAMED_LINT)
    include (ReFramedLintTarget)
    reframed_lint_target()
endif ()

# "application" target should be the startup project in VS
set_property(
    DIRECTORY "${PROJECT_SOURCE_DIR}"
    PROPERTY VS_STARTUP_PROJECT application)

# All subprojects have added targets to this export
# group, so make sure to install it
install (
    EXPORT ReFramedTargets
    NAMESPACE ReFramed::
    DESTINATION ${REFRAMED_INSTALL_CMAKEDIR})
export (
    EXPORT ReFramedTargets
    NAMESPACE ReFramed::
    FILE ${REFRAMED_BUILD_CMAKEDIR}/ReFramedTargets.cmake)

configure_file ("cmake/modules/TestVisibilityMacros.cmake" "${REFRAMED_BUILD_CMAKEDIR}/modules/TestVisibilityMacros.cmake" COPYONLY)

install (
    DIRECTORY "cmake/modules/"
    DESTINATION ${REFRAMED_INSTALL_CMAKEDIR}/modules)

include(CMakePackageConfigHelpers)
configure_package_config_file("cmake/ReFramedConfig.cmake.in"
    "${REFRAMED_BUILD_CMAKEDIR}/ReFramedConfig.cmake"
    INSTALL_DESTINATION "${REFRAMED_INSTALL_CMAKEDIR}")
install(
    FILES "${REFRAMED_BUILD_CMAKEDIR}/ReFramedConfig.cmake"
    DESTINATION ${REFRAMED_INSTALL_CMAKEDIR})
