cmake_minimum_required (VERSION 3.2)

set (PLUGIN_NAME "data-viewer")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-${PLUGIN_NAME}
    SOURCES
        "src/models/BaseStatusIDModel.cpp"
        "src/models/DataViewerModel.cpp"
        "src/models/FighterIDModel.cpp"
        "src/models/FighterStatesModel.cpp"
        "src/models/HitStatusIDModel.cpp"
        "src/models/MetaDataModel.cpp"
        "src/models/SpecificStatusIDModel.cpp"
        "src/models/StageIDModel.cpp"
        "src/views/DataViewerView.cpp"
        "src/DataViewerPlugin.cpp"
        "src/Plugin.cpp"
	HEADERS
        "include/${PLUGIN_NAME}/listeners/DataViewerListener.hpp"
        "include/${PLUGIN_NAME}/models/BaseStatusIDModel.hpp"
        "include/${PLUGIN_NAME}/models/DataViewerModel.hpp"
        "include/${PLUGIN_NAME}/models/FighterIDModel.hpp"
        "include/${PLUGIN_NAME}/models/FighterStatesModel.hpp"
        "include/${PLUGIN_NAME}/models/HitStatusIDModel.hpp"
        "include/${PLUGIN_NAME}/models/MetaDataModel.hpp"
        "include/${PLUGIN_NAME}/models/SpecificStatusIDModel.hpp"
        "include/${PLUGIN_NAME}/models/StageIDModel.hpp"
        "include/${PLUGIN_NAME}/views/DataViewerView.hpp"
        "include/${PLUGIN_NAME}/DataViewerPlugin.hpp"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/views/DataViewerView.hpp"
    FORMS
        "forms/DataViewerView.ui"
    INCLUDE_DIRECTORIES
        "include")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
endif ()
