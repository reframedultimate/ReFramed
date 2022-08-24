cmake_minimum_required (VERSION 3.2)

set (PLUGIN_NAME "stats")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.8
    LANGUAGES CXX)

option (STATS_ENABLE_LICENSING "Enables the license agreement popup" OFF)
set (STATS_LICENSED_TO "" CACHE STRING "")

configure_file ("cmake/templates/config.hpp.in" "include/stats/config.hpp")

reframed_add_plugin (plugin-${PLUGIN_NAME}
    FORMS
        "forms/SettingsView.ui"
        "forms/LicenseView.ui"
    SOURCES
        "src/exporters/OBSExporter.cpp"
        "src/models/SettingsModel.cpp"
        "src/models/StatsCalculator.cpp"
        "src/models/PlayerMeta.cpp"
        "src/util/Paths.cpp"
        "src/util/StatsFormatter.cpp"
        "src/views/LicenseDialog.cpp"
        "src/views/MainView.cpp"
        "src/views/SettingsView.cpp"
        "src/views/StatsView.cpp"
        "src/widgets/SettingsDragWidget.cpp"
        "src/widgets/SettingsStatsItem.cpp"
        "src/StatType.cpp"
        "src/StatsPlugin.cpp"
        "src/Plugin.cpp"
    HEADERS
        "include/${PLUGIN_NAME}/exporters/OBSExporter.hpp"
        "include/${PLUGIN_NAME}/listeners/SettingsListener.hpp"
        "include/${PLUGIN_NAME}/listeners/StatsCalculatorListener.hpp"
        "include/${PLUGIN_NAME}/listeners/PlayerMetaListener.hpp"
        "include/${PLUGIN_NAME}/models/SettingsModel.hpp"
        "include/${PLUGIN_NAME}/models/StatsCalculator.hpp"
        "include/${PLUGIN_NAME}/models/PlayerMeta.hpp"
        "include/${PLUGIN_NAME}/util/Paths.hpp"
        "include/${PLUGIN_NAME}/util/StatsFormatter.hpp"
        "include/${PLUGIN_NAME}/views/LicenseDialog.hpp"
        "include/${PLUGIN_NAME}/views/MainView.hpp"
        "include/${PLUGIN_NAME}/views/SettingsView.hpp"
        "include/${PLUGIN_NAME}/views/StatsView.hpp"
        "include/${PLUGIN_NAME}/widgets/SettingsDragWidget.hpp"
        "include/${PLUGIN_NAME}/widgets/SettingsStatsItem.hpp"
        "include/${PLUGIN_NAME}/StatType.hpp"
        "include/${PLUGIN_NAME}/StatsPlugin.hpp"
    INCLUDE_DIRECTORIES
        "include"
        "${PROJECT_BINARY_DIR}/include"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/views/LicenseDialog.hpp"
        "include/${PLUGIN_NAME}/views/SettingsView.hpp"
        "include/${PLUGIN_NAME}/views/StatsView.hpp"
        "include/${PLUGIN_NAME}/widgets/SettingsDragWidget.hpp"
        "include/${PLUGIN_NAME}/widgets/SettingsStatsItem.hpp")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
    if (STATS_ENABLE_LICENSING AND NOT STATS_LICENSED_TO)
        message (FATAL_ERROR "Please specify -DSTATS_LICENSED_TO=\"Organization\"")
    endif ()
endif ()

