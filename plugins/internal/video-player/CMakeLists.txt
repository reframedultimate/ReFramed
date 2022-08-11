cmake_minimum_required (VERSION 3.2)

cmake_policy (SET CMP0074 NEW)

project ("plugin-video-player"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-video-player
    SOURCES
        "src/Plugin.cpp"
        "src/VideoPlayer.cpp"
    MOC_HEADERS
        "include/video-player/VideoPlayer.hpp"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-video-player})
    find_package (FFMPEG COMPONENTS avcodec avformat avutil avdevice REQUIRED)
    target_link_libraries (plugin-video-player
        PRIVATE FFMPEG::avutil
        PRIVATE FFMPEG::avcodec
        PRIVATE FFMPEG::avformat
        PRIVATE FFMPEG::avdevice)
endif ()
