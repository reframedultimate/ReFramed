project ("application"
    VERSION ${REFRAMED_VERSION}
    LANGUAGES C CXX)

include (GNUInstallDirs)

configure_file ("cmake/templates/config.hpp.in" "include/application/config.hpp")

find_package (Qt5 COMPONENTS Core Widgets Gui LinguistTools REQUIRED)

qt5_wrap_cpp (APPLICATION_SOURCES_MOC
    "include/application/models/ActiveSessionManager.hpp"
    "include/application/models/Protocol.hpp"
    "include/application/models/ProtocolTask.hpp"
    "include/application/views/ActiveSessionView.hpp"
    "include/application/views/CategoryView.hpp"
    "include/application/views/ConnectView.hpp"
    "include/application/views/ExportReplayPackDialog.hpp"
    "include/application/views/ImportReplayPackDialog.hpp"
    "include/application/views/MainWindow.hpp"
    "include/application/views/ReplayEditorDialog.hpp"
    "include/application/views/ReplayGroupView.hpp"
    "include/application/views/ReplayListWidget.hpp"
    "include/application/views/ReplayViewer.hpp"
    "include/application/views/UserMotionLabelsEditor.hpp"
    "include/application/views/VideoAssociatorDialog.hpp")
qt5_wrap_ui (APPLICATION_FORMS_MOC
    "forms/ActiveSessionView.ui"
    "forms/AnalysisInputView.ui"
    "forms/ConnectView.ui"
    "forms/ExportReplayPackDialog.ui"
    "forms/ImportReplayPackDialog.ui"
	"forms/ReplayEditorDialog.ui"
    "forms/ReplayGroupView.ui"
    "forms/TrainingModeView.ui"
    "forms/VideoAssociatorDialog.ui"
    "forms/MainWindow.ui")
qt5_add_resources (APPLICATION_RESOURCES_GEN
    "res/reframed.qrc"
	"res/qdarkstyle/light/lightstyle.qrc"
	"res/qdarkstyle/dark/darkstyle.qrc")
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
    "src/models/ActiveSessionManager.cpp"
    "src/models/CategoryModel.cpp"
    "src/models/Config.cpp"
    "src/models/ConfigAccessor.cpp"
    "src/models/PluginManager.cpp"
    "src/models/Protocol.cpp"
    "src/models/ProtocolTask.cpp"
    "src/models/ReplayGroup.cpp"
    "src/models/ReplayManager.cpp"
    "src/models/UserMotionLabelsManager.cpp"
    "src/views/ActiveSessionView.cpp"
    "src/views/CategoryView.cpp"
    "src/views/ConnectView.cpp"
    "src/views/ExportReplayPackDialog.cpp"
	"src/views/ImportReplayPackDialog.cpp"
    "src/views/ReplayEditorDialog.cpp"
    "src/views/ReplayGroupView.cpp"
    "src/views/ReplayListWidget.cpp"
    "src/views/ReplayViewer.cpp"
    "src/views/UserMotionLabelsEditor.cpp"
    "src/views/VideoAssociatorDialog.cpp"
    "src/views/MainWindow.cpp")
target_include_directories (application
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_compile_options (application
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-W -Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-W -Wall -Wextra -pedantic>)
target_link_libraries (application
    PRIVATE
        nlohmann_json::nlohmann_json
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
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
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/data/ParamLabels.csv" ${REFRAMED_BUILD_DATADIR}
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

