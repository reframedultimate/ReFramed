include (UHPlugin)

project ("plugin-videoplayer"
    VERSION 0.0.1
    LANGUAGES CXX)

uh_add_plugin (plugin-videoplayer
    SOURCES
        "src/Plugin.cpp"
        "src/VideoPlayer.cpp"
    MOC_HEADERS
        "include/videoplayer/VideoPlayer.hpp"
    INCLUDE_DIRECTORIES
        "include")

find_path (AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library (AVCODEC_LIBRARY avcodec)

find_path (AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library (AVFORMAT_LIBRARY avformat)

find_path (AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library (AVUTIL_LIBRARY avutil)

find_path (AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library (AVDEVICE_LIBRARY avdevice)

target_include_directories (plugin-videoplayer
    PRIVATE ${AVCODEC_INCLUDE_DIR}
    PRIVATE ${AVFORMAT_INCLUDE_DIR}
    PRIVATE ${AVUTIL_INCLUDE_DIR}
    PRIVATE ${AVDEVICE_INCLUDE_DIR})
target_link_libraries (plugin-videoplayer
    PRIVATE ${AVCODEC_LIBRARY}
    PRIVATE ${AVFORMAT_LIBRARY}
    PRIVATE ${AVUTIL_LIBRARY}
    PRIVATE ${AVDEVICE_LIBRARY})

