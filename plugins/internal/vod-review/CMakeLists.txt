cmake_minimum_required (VERSION 3.2)

cmake_policy (SET CMP0074 NEW)
set (PLUGIN_NAME "vod-review")

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)
reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/models/AVDecoder.cpp"
        "src/models/BufferedSeekableDecoder.cpp"
        "src/models/VideoPlayerModel.cpp"
		"src/models/VODReviewModel.cpp"
        "src/views/VideoPlayerView.cpp"
        "src/views/VODReviewView.cpp"
        "src/VideoPlayerPlugin.cpp"
        "src/widgets/TimelineWidget.cpp"
        "src/VODReviewPlugin.cpp"
        "src/Plugin.cpp"
    HEADERS
        "include/${PLUGIN_NAME}/listeners/VideoPlayerListener.hpp"
        "include/${PLUGIN_NAME}/listeners/VODReviewListener.hpp"
        "include/${PLUGIN_NAME}/interfaces/AVDecoderInterface.hpp"
        "include/${PLUGIN_NAME}/models/AVDecoder.hpp"
        "include/${PLUGIN_NAME}/models/BufferedSeekableDecoder.hpp"
        "include/${PLUGIN_NAME}/models/VideoPlayerModel.hpp"
        "include/${PLUGIN_NAME}/models/VODReviewModel.hpp"
        "include/${PLUGIN_NAME}/views/VideoPlayerView.hpp"
        "include/${PLUGIN_NAME}/views/VODReviewView.hpp"
        "include/${PLUGIN_NAME}/widgets/TimelineWidget.hpp"
        "include/${PLUGIN_NAME}/VideoPlayerPlugin.hpp"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/views/VideoPlayerView.hpp"
        "include/${PLUGIN_NAME}/views/VODReviewView.hpp"
        "include/${PLUGIN_NAME}/models/VideoPlayerModel.hpp"
    FORMS
        "forms/VODReviewView.ui"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
    find_package (FFMPEG COMPONENTS avcodec avformat avutil avdevice swscale swresample REQUIRED)
    target_link_libraries (plugin-${PLUGIN_NAME}
        PRIVATE FFMPEG::avutil
        PRIVATE FFMPEG::avcodec
        PRIVATE FFMPEG::avformat
        PRIVATE FFMPEG::avdevice
        PRIVATE FFMPEG::swresample
        PRIVATE FFMPEG::swscale)

    # On Windows we have to copy the DLLs to our binary directory
    if (WIN32 OR CYGWIN)
        foreach (LIB ${FFMPEG_LIBRARIES})
            get_filename_component (LIB_FILENAME ${LIB} NAME_WE)
            get_filename_component (FFMPEG_BIN_DIR ${LIB} DIRECTORY)
            get_filename_component (FFMPEG_BIN_DIR "${FFMPEG_BIN_DIR}/../bin" ABSOLUTE)

            file (GLOB DLL_FILENAMES "${FFMPEG_BIN_DIR}/${LIB_FILENAME}*.dll")
            list (GET DLL_FILENAMES 0 DLL_FILENAME)
            if (NOT DLL_FILENAME)
                message (FATAL_ERROR "Failed to find DLL matching library ${LIB}")
            endif ()

            add_custom_command (TARGET plugin-${PLUGIN_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL_FILENAME}" "${REFRAMED_BUILD_PLUGINDEPSDIR}"
                COMMENT "Copying FFMPEG DLL ${DLL_FILENAME} to runtime dir ${REFRAMED_BUILD_PLUGINDEPSDIR}"
                VERBATIM)
			install (
				FILES ${DLL_FILENAME}
				DESTINATION ${REFRAMED_INSTALL_PLUGINDEPSDIR})
        endforeach ()
    endif ()
endif ()
