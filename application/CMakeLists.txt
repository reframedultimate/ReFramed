project ("application"
    VERSION ${REFRAMED_VERSION}
    LANGUAGES C CXX)

include (GNUInstallDirs)

configure_file ("cmake/templates/config.hpp.in" "include/application/config.hpp")

find_package (Qt5 COMPONENTS Core Widgets Gui LinguistTools REQUIRED)

set (APPLICATION_SOURCES
    "src/models/ActiveSessionManager.cpp"
    "src/models/AutoAssociateVideosTask.cpp"
    "src/models/Config.cpp"
    "src/models/ConfigAccessor.cpp"
    "src/models/FighterIcon.cpp"
    "src/models/MetaDataEditModel.cpp"
    "src/models/PluginManager.cpp"
    "src/models/Protocol.cpp"
    "src/models/ProtocolTask.cpp"
    "src/models/ProtocolTaskSSB64.cpp"
    "src/models/ReplayGroup.cpp"
    "src/models/ReplayListModel.cpp"
    "src/models/ReplayListSortFilterModel.cpp"
    "src/models/ReplayManager.cpp"
    "src/models/UserMotionLabelsManager.cpp"
    "src/views/ActiveSessionView.cpp"
    "src/views/CategoryTabsView.cpp"
    "src/views/ConnectView.cpp"
    "src/views/ExportReplayPackDialog.cpp"
    "src/views/ImportReplayPackDialog.cpp"
    "src/views/PluginDockView.cpp"
    "src/views/ReplayEditorDialog.cpp"
    #"src/views/ReplayGroupView.cpp"
    "src/views/ReplayGroupListView.cpp"
    "src/views/ReplayListView.cpp"
    "src/views/ReplayManagerView.cpp"
    "src/views/UserMotionLabelsEditor.cpp"
    "src/views/VideoAssociatorDialog.cpp"
    "src/widgets/CollapsibleSplitter.cpp"
    "src/widgets/ConnectionStatusWidget.cpp"
    "src/widgets/MetaDataEditWidget.cpp"
    "src/widgets/MetaDataEditWidget_AutoAssociateVideo.cpp"
    "src/widgets/MetaDataEditWidget_Commentators.cpp"
    "src/widgets/MetaDataEditWidget_Event.cpp"
    "src/widgets/MetaDataEditWidget_Game.cpp"
    "src/widgets/MetaDataEditWidget_Tournament.cpp"
    "src/widgets/ProgressDialog.cpp"
    "src/widgets/ReplaySearchBox.cpp"
    "src/views/MainWindow.cpp"
    "src/Util.cpp"
    "src/main.cpp")
set (APPLICATION_HEADERS
    "include/application/listeners/ActiveSessionManagerListener.hpp"
    "include/application/listeners/DataSetBackgroundLoaderListener.hpp"
    "include/application/listeners/MetaDataEditListener.hpp"
    "include/application/listeners/ReplayGroupListener.hpp"
    "include/application/listeners/ReplayManagerListener.hpp"
    "include/application/listeners/RunningGameSessionManagerListener.hpp"
    "include/application/models/ActiveSessionManager.hpp"
    "include/application/models/ConfigAccessor.hpp"
    "include/application/models/Config.hpp"
    "include/application/models/FighterIcon.hpp"
    "include/application/models/MetaDataEditModel.hpp"
    "include/application/models/PluginManager.hpp"
    "include/application/models/Protocol.hpp"
    "include/application/models/ProtocolTask.hpp"
    "include/application/models/ReplayGroup.hpp"
    "include/application/models/ReplayListModel.hpp"
    "include/application/models/ReplayListSortFilterModel.hpp"
    "include/application/models/ReplayManager.hpp"
    "include/application/models/RunningGameSessionManager.hpp"
    "include/application/models/UserMotionLabelsManager.hpp"
    "include/application/views/RunningGameSessionView.hpp"
    "include/application/widgets/CollapsibleSplitter.hpp"
    "include/application/widgets/ConnectionStatusWidget.hpp"
    "include/application/Util.hpp")
set (APPLICATION_HEADERS_MOC
    "include/application/models/ActiveSessionManager.hpp"
    "include/application/models/AutoAssociateVideosTask.hpp"
    "include/application/models/Protocol.hpp"
    "include/application/models/ProtocolTask.hpp"
    "include/application/models/ProtocolTaskSSB64.hpp"
    "include/application/views/ActiveSessionView.hpp"
    "include/application/views/CategoryTabsView.hpp"
    "include/application/views/ConnectView.hpp"
    "include/application/views/ExportReplayPackDialog.hpp"
    "include/application/views/ImportReplayPackDialog.hpp"
    "include/application/views/MainWindow.hpp"
    "include/application/views/PluginDockView.hpp"
    "include/application/views/ReplayEditorDialog.hpp"
    #"include/application/views/ReplayGroupView.hpp"
    "include/application/views/ReplayGroupListView.hpp"
    "include/application/views/ReplayListView.hpp"
    "include/application/views/ReplayManagerView.hpp"
    "include/application/views/UserMotionLabelsEditor.hpp"
    "include/application/views/VideoAssociatorDialog.hpp"
    "include/application/widgets/MetaDataEditWidget.hpp"
    "include/application/widgets/MetaDataEditWidget_AutoAssociateVideo.hpp"
    "include/application/widgets/MetaDataEditWidget_Commentators.hpp"
    "include/application/widgets/MetaDataEditWidget_Event.hpp"
    "include/application/widgets/MetaDataEditWidget_Game.hpp"
    "include/application/widgets/MetaDataEditWidget_Tournament.hpp"
    "include/application/widgets/ProgressDialog.hpp"
	"include/application/widgets/ReplaySearchBox.hpp")
set (APPLICATION_FORMS
    "forms/ConnectView.ui"
    "forms/ExportReplayPackDialog.ui"
    "forms/ImportReplayPackDialog.ui"
    "forms/MetaDataEditWidget_Game.ui"
    "forms/VideoAssociatorDialog.ui"
    "forms/MainWindow.ui")
set (APPLICATION_RES
    "res/reframed.qrc"
    "res/qdarkstyle/light/lightstyle.qrc"
    "res/qdarkstyle/dark/darkstyle.qrc")

qt5_wrap_cpp (APPLICATION_SOURCES_MOC ${APPLICATION_HEADERS_MOC})
qt5_wrap_ui (APPLICATION_FORMS_MOC ${APPLICATION_FORMS})
qt5_add_resources (APPLICATION_RESOURCES_GEN ${APPLICATION_RES})
qt5_add_translation (APPLICATION_QM_FILES "")

set (APPLICATION_WIN32_EXE "WIN32")
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set (APPLICATION_WIN32_EXE "")
endif ()

add_executable (application #${APPLICATION_WIN32_EXE}
    ${APPLICATION_SOURCES}
    ${APPLICATION_SOURCES_MOC}
    ${APPLICATION_HEADERS}
    ${APPLICATION_HEADERS_MOC}
    ${APPLICATION_FORMS}
    ${APPLICATION_FORMS_MOC}
    ${APPLICATION_RESOURCES}
    ${APPLICATION_RESOURCES_GEN}
    ${APPLICATION_QM_FILES}
    $<$<PLATFORM_ID:Windows>:res/windows.rc>)
target_include_directories (application
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_compile_options (application
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-W -Wall -Wextra -Wno-unused-parameter -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-W -Wall -Wextra -pedantic>)
target_link_libraries (application
    PRIVATE
        nlohmann_json::nlohmann_json
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        ads::qtadvanceddocking
        rfplot)
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
# Copy data files to bin directory
###############################################################################

add_custom_command (TARGET application POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${REFRAMED_BUILD_DATADIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/data/motion/ParamLabels.csv" "${REFRAMED_BUILD_DATADIR}/motion/ParamLabels.csv"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/data/motion/1_Specific.json" "${REFRAMED_BUILD_DATADIR}/motion/1_Specific.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/data/motion/2_General.json" "${REFRAMED_BUILD_DATADIR}/motion/2_General.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/data/motion/3_English.json" "${REFRAMED_BUILD_DATADIR}/motion/3_English.json"
    COMMENT "Copying data files to ${REFRAMED_BUILD_DATADIR}"
    VERBATIM)

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
    DIRECTORY ${PROJECT_SOURCE_DIR}/data/
    DESTINATION ${REFRAMED_INSTALL_DATADIR})
install (
    TARGETS application
    RUNTIME DESTINATION "${REFRAMED_INSTALL_BINDIR}")
set_target_properties (application
    PROPERTIES
        INSTALL_RPATH ${REFRAMED_INSTALL_LIBDIR})

