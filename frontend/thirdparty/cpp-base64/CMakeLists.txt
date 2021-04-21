cmake_minimum_required (VERSION 3.1)
project ("cpp-base64"
    VERSION 2.08
    LANGUAGES CXX)

option (CPP_BASE64_PIC "Generate position independent code" OFF)

add_library (cpp-base64 STATIC
    "src/base64.cpp")
target_include_directories (cpp-base64
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
set_target_properties (cpp-base64
    PROPERTIES POSITION_INDEPENDENT_CODE ${CPP_BASE64_PIC})

