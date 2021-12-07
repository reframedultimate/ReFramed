include (UHPlugin)

project ("plugin-video-player"
    VERSION 0.0.1
    LANGUAGES CXX)

uh_add_plugin (plugin-video-player
    SOURCES
        "src/Plugin.cpp"
        "src/VideoPlayer.cpp"
    MOC_HEADERS
        "include/video-player/VideoPlayer.hpp"
    INCLUDE_DIRECTORIES
        "include")

if (${UH_plugin-video-player})
    find_package (FFMPEG COMPONENTS avcodec avformat avutil avdevice REQUIRED)
    target_link_libraries (plugin-video-player
        PRIVATE FFMPEG::avutil
        PRIVATE FFMPEG::avcodec
        PRIVATE FFMPEG::avformat
        PRIVATE FFMPEG::avdevice)
endif ()
