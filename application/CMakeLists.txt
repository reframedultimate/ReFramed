project ("application"
    VERSION ${REFRAMED_VERSION}
    LANGUAGES C CXX)

include (GNUInstallDirs)

configure_file ("cmake/templates/config.hpp.in" "include/application/config.hpp")

find_package (Qt5 COMPONENTS Core Widgets Gui LinguistTools REQUIRED)

qt5_wrap_cpp (APPLICATION_SOURCES_MOC
    "include/application/models/DataSetBackgroundLoader.hpp"
    "include/application/models/Protocol.hpp"
    "include/application/models/ProtocolConnectTask.hpp"
    "include/application/models/ProtocolCommunicateTask.hpp"
    "include/application/models/RunningGameSessionManager.hpp"
    "include/application/views/AnalysisView.hpp"
    "include/application/views/CategoryView.hpp"
    "include/application/views/ConnectView.hpp"
    "include/application/views/DataSetFilterView.hpp"
    "include/application/views/DataSetFilterWidget.hpp"
    "include/application/views/DataSetFilterWidget_Date.hpp"
    "include/application/views/DataSetFilterWidget_Game.hpp"
    "include/application/views/DataSetFilterWidget_Matchup.hpp"
    "include/application/views/DataSetFilterWidget_Player.hpp"
    "include/application/views/DataSetFilterWidget_PlayerCount.hpp"
    "include/application/views/DataSetFilterWidget_Stage.hpp"
    "include/application/views/MainWindow.hpp"
    "include/application/views/RunningGameSessionView.hpp"
    "include/application/views/SavedGameSessionListWidget.hpp"
    "include/application/views/SavedGameSessionGroupView.hpp"
    "include/application/views/SessionView.hpp"
    "include/application/views/TrainingModeView.hpp"
    "include/application/views/VisualizerView.hpp")
qt5_wrap_ui (APPLICATION_FORMS_MOC
    "forms/AnalysisInputView.ui"
    "forms/ConnectView.ui"
    "forms/DataSetFilterView.ui"
    "forms/DataSetFilterWidget_Date.ui"
    "forms/DataSetFilterWidget_Game.ui"
    "forms/DataSetFilterWidget_Matchup.ui"
    "forms/DataSetFilterWidget_Player.ui"
    "forms/DataSetFilterWidget_PlayerCount.ui"
    "forms/DataSetFilterWidget_Stage.ui"
    "forms/RunningGameSessionView.ui"
    "forms/SavedGameSessionGroupView.ui"
    "forms/SessionView.ui"
    "forms/TrainingModeView.ui"
    "forms/MainWindow.ui")
qt5_add_resources (APPLICATION_RESOURCES_GEN
    "res/reframed.qrc")
qt5_add_translation (APPLICATION_QM_FILES
    "")

set (APPLICATION_WIN32_EXE "WIN32")
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set (APPLICATION_WIN32_EXE "")
endif ()

add_executable (application ${APPLICATION_WIN32_EXE}
    ${APPLICATION_SOURCES_MOC}
    ${APPLICATION_FORMS_MOC}
    ${APPLICATION_RESOURCES_GEN}
    ${APPLICATION_QM_FILES}
    ${APPLICATION_RESOURCES_GEN}
    $<$<PLATFORM_ID:Windows>:res/windows.rc>
    "src/main.cpp"
    "src/Util.cpp"
    "src/models/CategoryModel.cpp"
    "src/models/Config.cpp"
    "src/models/ConfigAccessor.cpp"
    "src/models/DataSetBackgroundLoader.cpp"
    "src/models/PluginManager.cpp"
    "src/models/Protocol.cpp"
    "src/models/ProtocolConnectTask.cpp"
    "src/models/ProtocolCommunicateTask.cpp"
    "src/models/RunningGameSessionManager.cpp"
    "src/models/SavedGameSessionGroup.cpp"
    "src/models/SavedGameSessionManager.cpp"
    "src/models/TrainingModeModel.cpp"
    "src/views/AnalysisView.cpp"
    "src/views/CategoryView.cpp"
    "src/views/ConnectView.cpp"
    "src/views/DataSetFilterView.cpp"
    "src/views/DataSetFilterWidget.cpp"
    "src/views/DataSetFilterWidget_Date.cpp"
    "src/views/DataSetFilterWidget_Game.cpp"
    "src/views/DataSetFilterWidget_Matchup.cpp"
    "src/views/DataSetFilterWidget_Player.cpp"
    "src/views/DataSetFilterWidget_PlayerCount.cpp"
    "src/views/DataSetFilterWidget_Stage.cpp"
    "src/views/RunningGameSessionView.cpp"
    "src/views/SavedGameSessionListWidget.cpp"
    "src/views/SavedGameSessionGroupView.cpp"
    "src/views/SessionView.cpp"
    "src/views/TrainingModeView.cpp"
    "src/views/VisualizerView.cpp"
    "src/views/MainWindow.cpp")
target_include_directories (application
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_compile_options (application
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-W -Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-W -Wall -Wextra -pedantic>)
target_link_libraries (application
    PRIVATE Qt5::Core
    PRIVATE Qt5::Gui
    PRIVATE Qt5::Widgets
    PRIVATE rfplot)
set_target_properties (application
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${REFRAMED_BUILD_BINDIR}
        OUTPUT_NAME "ReFramed")

# workaround because uic generates headers into CMAKE_CURRENT_BINARY_DIR
# and we want to include them from "application/ui_*.h"
target_include_directories (application
    PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/../>)

###############################################################################
# DLLs need to be copied to the runtime directory on Windows
###############################################################################

if (WIN32 OR CYGWIN)
    add_custom_command (TARGET application POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${REFRAMED_BUILD_BINDIR}/platforms
        COMMAND ${CMAKE_COMMAND} -E make_directory ${REFRAMED_BUILD_BINDIR}/imageformats
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5Core$<$<CONFIG:Debug>:d>.dll         ${REFRAMED_BUILD_BINDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5Gui$<$<CONFIG:Debug>:d>.dll          ${REFRAMED_BUILD_BINDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5OpenGL$<$<CONFIG:Debug>:d>.dll       ${REFRAMED_BUILD_BINDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5Widgets$<$<CONFIG:Debug>:d>.dll      ${REFRAMED_BUILD_BINDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll ${REFRAMED_BUILD_BINDIR}/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../plugins/imageformats/qico$<$<CONFIG:Debug>:d>.dll ${REFRAMED_BUILD_BINDIR}/imageformats

        COMMENT "Copying Qt binaries from '${Qt5_DIR}/../../bin/' to '${REFRAMED_BUILD_BINDIR}'"
        VERBATIM)
    install (
        FILES
            ${Qt5_DIR}/../../../bin/Qt5Core$<$<CONFIG:Debug>:d>.dll
            ${Qt5_DIR}/../../../bin/Qt5Gui$<$<CONFIG:Debug>:d>.dll
            ${Qt5_DIR}/../../../bin/Qt5OpenGL$<$<CONFIG:Debug>:d>.dll
            ${Qt5_DIR}/../../../bin/Qt5Widgets$<$<CONFIG:Debug>:d>.dll
        DESTINATION ${REFRAMED_INSTALL_BINDIR})
    install (
        FILES ${Qt5_DIR}/../../../plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll
        DESTINATION "${REFRAMED_INSTALL_BINDIR}/platforms")
    install (
        FILES ${Qt5_DIR}/../../../plugins/imageformats/qico$<$<CONFIG:Debug>:d>.dll
        DESTINATION "${REFRAMED_INSTALL_BINDIR}/imageformats")
endif ()

install (
    TARGETS application
    RUNTIME DESTINATION "${REFRAMED_INSTALL_BINDIR}")
set_target_properties (application
    PROPERTIES
        INSTALL_RPATH ${REFRAMED_INSTALL_LIBDIR})

