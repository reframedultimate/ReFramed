cmake_minimum_required (VERSION 3.2)

set (PLUGIN_NAME "frame-data-list")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/models/FrameDataListModel.cpp"
        "src/models/MetaDataModel.cpp"
        "src/views/FrameDataListView.cpp"
        "src/FrameDataListPlugin.cpp"
        "src/Plugin.cpp"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/views/FrameDataListView.hpp"
    FORMS
        "forms/FrameDataListView.ui"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()

