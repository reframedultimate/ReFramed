cmake_minimum_required (VERSION 3.25)

set (PLUGIN_NAME "stats")

project ("plugin-${PLUGIN_NAME}"
    VERSION 0.0.9
    LANGUAGES CXX)

option (STATS_WEBSOCKET_SERVER "Enable the websocket server" ON)
option (STATS_ENABLE_LICENSING "Enables the license agreement popup" OFF)
set (STATS_LICENSED_TO "" CACHE STRING "Specify who or which organiziation this plugin is licensed to")

configure_file ("cmake/templates/config.hpp.in" "include/stats/config.hpp")

reframed_add_plugin (plugin-${PLUGIN_NAME}
    FORMS
        "forms/ExportView.ui"
        "forms/StatsArrangeView.ui"
        "forms/LicenseView.ui"
    SOURCES
        "src/exporters/OBSExporter.cpp"
        "src/exporters/WSExporter.cpp"
        "src/models/SettingsModel.cpp"
        "src/models/StatsCalculator.cpp"
        "src/models/PlayerMeta.cpp"
        "src/models/WebSocketServer.cpp"
        "src/util/Paths.cpp"
        "src/util/StatsFormatter.cpp"
        "src/views/ExportView.cpp"
        "src/views/LicenseDialog.cpp"
        "src/views/MainView.cpp"
        "src/views/StatsView.cpp"
        "src/views/StatsArrangeView.cpp"
        "src/widgets/SettingsDragWidget.cpp"
        "src/widgets/SettingsStatsItem.cpp"
        "src/StatType.cpp"
        "src/StatsPlugin.cpp"
        "src/Plugin.cpp"
    HEADERS
        "include/${PLUGIN_NAME}/exporters/OBSExporter.hpp"
        "include/${PLUGIN_NAME}/exporters/WSExporter.hpp"
        "include/${PLUGIN_NAME}/listeners/SettingsListener.hpp"
        "include/${PLUGIN_NAME}/listeners/StatsCalculatorListener.hpp"
        "include/${PLUGIN_NAME}/listeners/PlayerMetaListener.hpp"
        "include/${PLUGIN_NAME}/listeners/WebSocketServerListener.hpp"
        "include/${PLUGIN_NAME}/models/SettingsModel.hpp"
        "include/${PLUGIN_NAME}/models/StatsCalculator.hpp"
        "include/${PLUGIN_NAME}/models/PlayerMeta.hpp"
        "include/${PLUGIN_NAME}/models/WebSocketServer.hpp"
        "include/${PLUGIN_NAME}/util/Paths.hpp"
        "include/${PLUGIN_NAME}/util/StatsFormatter.hpp"
        "include/${PLUGIN_NAME}/views/ExportView.hpp"
        "include/${PLUGIN_NAME}/views/LicenseDialog.hpp"
        "include/${PLUGIN_NAME}/views/MainView.hpp"
        "include/${PLUGIN_NAME}/views/StatsView.hpp"
        "include/${PLUGIN_NAME}/views/StatsArrangeView.hpp"
        "include/${PLUGIN_NAME}/widgets/SettingsDragWidget.hpp"
        "include/${PLUGIN_NAME}/widgets/SettingsStatsItem.hpp"
        "include/${PLUGIN_NAME}/StatType.hpp"
        "include/${PLUGIN_NAME}/StatsPlugin.hpp"
        "cmake/templates/config.hpp.in"
    INCLUDE_DIRECTORIES
        "include"
        "${PROJECT_BINARY_DIR}/include"
    MOC_HEADERS
        "include/${PLUGIN_NAME}/models/WebSocketServer.hpp"
        "include/${PLUGIN_NAME}/views/ExportView.hpp"
        "include/${PLUGIN_NAME}/views/LicenseDialog.hpp"
        "include/${PLUGIN_NAME}/views/StatsView.hpp"
        "include/${PLUGIN_NAME}/views/StatsArrangeView.hpp"
        "include/${PLUGIN_NAME}/widgets/SettingsDragWidget.hpp"
        "include/${PLUGIN_NAME}/widgets/SettingsStatsItem.hpp")

if (${REFRAMED_plugin-${PLUGIN_NAME}})
    if (STATS_ENABLE_LICENSING AND NOT STATS_LICENSED_TO)
        message (FATAL_ERROR "Please specify -DSTATS_LICENSED_TO=\"Organization\"")
    endif ()

    ###########################################################################
    # Qt Websockets library
    ###########################################################################

    find_package (Qt6 COMPONENTS Network WebSockets REQUIRED)
    target_link_libraries (plugin-${PLUGIN_NAME}
        PRIVATE Qt6::Network
        PRIVATE Qt6::WebSockets)

    if (WIN32 OR CYGWIN)
        add_custom_command (TARGET plugin-${PLUGIN_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${REFRAMED_BUILD_PLUGINDEPSDIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_DIR}/../../../bin/Qt6Network$<$<CONFIG:Debug>:d>.dll ${REFRAMED_BUILD_PLUGINDEPSDIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_DIR}/../../../bin/Qt6WebSockets$<$<CONFIG:Debug>:d>.dll ${REFRAMED_BUILD_PLUGINDEPSDIR}
            COMMENT "Copying Qt binaries from '${Qt6_DIR}/../../bin/' to '${REFRAMED_BUILD_PLUGINDIR}'"
            VERBATIM)
        install (
            FILES ${Qt6_DIR}/../../../bin/Qt6Network$<$<CONFIG:Debug>:d>.dll
            FILES ${Qt6_DIR}/../../../bin/Qt6WebSockets$<$<CONFIG:Debug>:d>.dll
            DESTINATION ${REFRAMED_INSTALL_PLUGINDEPSDIR})
    endif ()
endif ()
