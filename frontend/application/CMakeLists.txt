project ("application"
    VERSION 1.0.0
    LANGUAGES C CXX)

include (GNUInstallDirs)

find_package (Qt5 COMPONENTS Core Widgets Gui LinguistTools REQUIRED)

qt5_wrap_cpp (UH_SOURCES_MOC
    "include/application/models/ActiveRecordingManager.hpp"
    "include/application/models/DataSetBackgroundLoader.hpp"
    "include/application/models/Protocol.hpp"
    "include/application/views/ActiveRecordingView.hpp"
    "include/application/views/AnalysisView.hpp"
    "include/application/views/CategoryView.hpp"
    "include/application/views/ConnectView.hpp"
    "include/application/views/DamageTimePlot.hpp"
    "include/application/views/DataSetFilterView.hpp"
    "include/application/views/DataSetFilterWidget.hpp"
    "include/application/views/DataSetFilterWidget_Date.hpp"
    "include/application/views/DataSetFilterWidget_Game.hpp"
    "include/application/views/DataSetFilterWidget_Matchup.hpp"
    "include/application/views/DataSetFilterWidget_Player.hpp"
    "include/application/views/DataSetFilterWidget_PlayerCount.hpp"
    "include/application/views/DataSetFilterWidget_Stage.hpp"
    "include/application/views/MainWindow.hpp"
    "include/application/views/RecordingDataView.hpp"
    "include/application/views/RecordingListWidget.hpp"
    "include/application/views/RecordingGroupView.hpp"
    "include/application/views/RecordingView.hpp"
    "include/application/views/VisualizerView.hpp"
    "include/application/views/XYPositionPlot.hpp")
qt5_wrap_ui (UH_FORMS_MOC
    "forms/ActiveRecordingView.ui"
    "forms/AnalysisInputView.ui"
    "forms/ConnectView.ui"
    "forms/DataSetFilterView.ui"
    "forms/DataSetFilterWidget_Date.ui"
    "forms/DataSetFilterWidget_Game.ui"
    "forms/DataSetFilterWidget_Matchup.ui"
    "forms/DataSetFilterWidget_Player.ui"
    "forms/DataSetFilterWidget_PlayerCount.ui"
    "forms/DataSetFilterWidget_Stage.ui"
    "forms/RecordingDataView.ui"
    "forms/RecordingGroupView.ui"
    "forms/RecordingView.ui"
    "forms/MainWindow.ui")
qt5_add_resources (UH_RESOURCES_GEN
    "")
qt5_add_translation (UH_QM_FILES
    "")

add_executable (ultimate-hindsight #WIN32
    ${UH_SOURCES_MOC}
    ${UH_FORMS_MOC}
    ${UH_RESOURCES_GEN}
    ${UH_QM_FILES}
    "src/main.cpp"
    "src/Util.cpp"
    "src/models/ActiveRecordingManager.cpp"
    "src/models/Config.cpp"
    "src/models/ConfigAccessor.cpp"
    "src/models/DataSetBackgroundLoader.cpp"
    "src/models/PluginManager.cpp"
    "src/models/Protocol.cpp"
    "src/models/RecordingGroup.cpp"
    "src/models/RecordingManager.cpp"
    "src/views/ActiveRecordingView.cpp"
    "src/views/AnalysisView.cpp"
    "src/views/CategoryView.cpp"
    "src/views/ConnectView.cpp"
    "src/views/DamageTimePlot.cpp"
    "src/views/DataSetFilterView.cpp"
    "src/views/DataSetFilterWidget.cpp"
    "src/views/DataSetFilterWidget_Date.cpp"
    "src/views/DataSetFilterWidget_Game.cpp"
    "src/views/DataSetFilterWidget_Matchup.cpp"
    "src/views/DataSetFilterWidget_Player.cpp"
    "src/views/DataSetFilterWidget_PlayerCount.cpp"
    "src/views/DataSetFilterWidget_Stage.cpp"
    "src/views/RecordingDataView.cpp"
    "src/views/RecordingListWidget.cpp"
    "src/views/RecordingGroupView.cpp"
    "src/views/RecordingView.cpp"
    "src/views/VisualizerView.cpp"
    "src/views/XYPositionPlot.cpp"
    "src/views/MainWindow.cpp")
target_include_directories (ultimate-hindsight
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_options (ultimate-hindsight
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-W -Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-W -Wall -Wextra -pedantic>)
target_link_libraries (ultimate-hindsight
    PRIVATE Qt5::Core
    PRIVATE Qt5::Gui
    PRIVATE Qt5::Widgets
    PRIVATE uhplot)
set_target_properties (ultimate-hindsight
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${UH_BUILD_BINDIR})

###############################################################################
# DLLs need to be copied to the runtime directory on Windows
###############################################################################

if (WIN32 OR CYGWIN)
    add_custom_command (TARGET ultimate-hindsight POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${UH_BUILD_BINDIR}/$<CONFIG>/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5Core$<$<CONFIG:Debug>:d>.dll         ${UH_BUILD_BINDIR}/$<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5Gui$<$<CONFIG:Debug>:d>.dll          ${UH_BUILD_BINDIR}/$<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5OpenGL$<$<CONFIG:Debug>:d>.dll       ${UH_BUILD_BINDIR}/$<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5Widgets$<$<CONFIG:Debug>:d>.dll      ${UH_BUILD_BINDIR}/$<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll ${UH_BUILD_BINDIR}/$<CONFIG>/platforms

        COMMENT "Copying Qt binaries from '${Qt5_DIR}/../../bin/' to '${UH_BUILD_BINDIR}/$<CONFIG>'"
        VERBATIM)
endif ()

install (
    TARGETS ultimate-hindsight
    RUNTIME DESTINATION "${UH_INSTALL_BINDIR}")
set_target_properties (ultimate-hindsight
    PROPERTIES
        INSTALL_RPATH ${UH_INSTALL_LIBDIR})

