cmake_minimum_required (VERSION 3.2)

set (PLUGIN_NAME "video-player")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)
reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/models/VideoDecoder.cpp"
        "src/models/VideoPlayerModel.cpp"
        "src/views/VideoPlayerView.cpp"
        "src/VideoPlayerPlugin.cpp"
        "src/Plugin.cpp"
    HEADERS
        "include/${PLUGIN_NAME}/listeners/VideoPlayerListener.hpp"
        "include/${PLUGIN_NAME}/models/VideoDecoder.hpp"
        "include/${PLUGIN_NAME}/models/VideoPlayerModel.hpp"
        "include/${PLUGIN_NAME}/views/VideoPlayerView.hpp"
        "include/${PLUGIN_NAME}/VideoPlayerPlugin.hpp"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/models/VideoDecoder.hpp"
        "include/${PLUGIN_NAME}/views/VideoPlayerView.hpp"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
    find_package (FFMPEG COMPONENTS avcodec avformat avutil avdevice REQUIRED)
    target_link_libraries (plugin-${PLUGIN_NAME}
        PRIVATE FFMPEG::avutil
        PRIVATE FFMPEG::avcodec
        PRIVATE FFMPEG::avformat
        PRIVATE FFMPEG::avdevice)
endif ()
