cmake_minimum_required(VERSION 3.5)

if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif (POLICY CMP0091)

# By default, the version information is extracted from the git index. However,
# we can override this behavior by explicitly setting ADS_VERSION and
# skipping the git checks. This is useful for cases where this project is being
# used independently of its original git repo (e.g. vendored in another project)
set (ADS_VERSION "4.3.1")
string(REGEX MATCHALL "[\.]" VERSION_DOT_MATCHES ${ADS_VERSION})
list(LENGTH VERSION_DOT_MATCHES VERSION_DOT_COUNT)
if(VERSION_DOT_COUNT EQUAL 2)
    set(VERSION_SHORT ${ADS_VERSION})
else()
    message(FATAL_ERROR "ADS_VERSION must be in major.minor.patch format, e.g. 3.8.1. Got ${ADS_VERSION}")
endif()

project(QtADS LANGUAGES CXX VERSION ${VERSION_SHORT})

option(BUILD_STATIC "Build the static library" OFF)
option(BUILD_EXAMPLES "Build the examples" OFF)

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(ads_PlatformDir "x86")
else()
    set(ads_PlatformDir "x64")
endif()

add_subdirectory(ads)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
    add_subdirectory(demo)
endif()

