project ("uh-application"
    VERSION 1.0.0
    LANGUAGES C CXX)

include (GNUInstallDirs)

find_package (Qt5 COMPONENTS Widgets Gui LinguistTools REQUIRED)

qt5_wrap_cpp (UH_SOURCES_MOC
    "include/uh/models/ActiveRecordingManager.hpp"
    "include/uh/models/Protocol.hpp"
    "include/uh/plot/DeltaPlotPicker.hpp"
    "include/uh/plot/Panner.hpp"
    "include/uh/plot/RectangleZoomer.hpp"
    "include/uh/plot/Zoomer.hpp"
    "include/uh/views/ActiveRecordingView.hpp"
    "include/uh/views/CategoryView.hpp"
    "include/uh/views/ConnectView.hpp"
    "include/uh/views/DamageTimePlot.hpp"
    "include/uh/views/MainWindow.hpp"
    "include/uh/views/RecordingDataView.hpp"
    "include/uh/views/RealtimePlot.hpp"
    "include/uh/views/RecordingGroupView.hpp"
    "include/uh/views/RecordingView.hpp"
    "include/uh/views/XYPositionPlot.hpp")
qt5_wrap_ui (UH_FORMS_MOC
    "forms/ActiveRecordingView.ui"
    "forms/ConnectView.ui"
    "forms/RecordingDataView.ui"
    "forms/RecordingGroupView.ui"
    "forms/RecordingView.ui"
    "forms/MainWindow.ui")
qt5_add_resources (UH_RESOURCES_GEN
    "")
qt5_add_translation (UH_QM_FILES
    "")

add_executable (ultimate-hindsight WIN32
    ${UH_SOURCES_MOC}
    ${UH_FORMS_MOC}
    ${UH_RESOURCES_GEN}
    ${UH_QM_FILES}
    "src/main.cpp"
    "src/Util.cpp"
    "src/models/ActiveRecordingManager.cpp"
    "src/models/ActiveRecording.cpp"
    "src/models/Config.cpp"
    "src/models/ConfigAccessor.cpp"
    "src/models/DecisionGraph.cpp"
    "src/models/FighterStatusMapping.cpp"
    "src/models/FighterIDMapping.cpp"
    "src/models/HitStatusMapping.cpp"
    "src/models/MappingInfo.cpp"
    "src/models/PlayerState.cpp"
    "src/models/Protocol.cpp"
    "src/models/Recording.cpp"
    "src/models/RecordingGroup.cpp"
    "src/models/RecordingManager.cpp"
    "src/models/SavedRecording.cpp"
    "src/models/SetFormat.cpp"
    "src/models/StageIDMapping.cpp"
    "src/platform/tcp_socket.c"
    "src/plot/AutoScaler.cpp"
    "src/plot/ColorPalette.cpp"
    "src/plot/DeltaPlotPicker.cpp"
    "src/plot/MousePlotPickerMachine.cpp"
    "src/plot/Panner.cpp"
    "src/plot/RectangleZoomer.cpp"
    "src/plot/Zoomer.cpp"
    "src/views/ActiveRecordingView.cpp"
    "src/views/CategoryView.cpp"
    "src/views/ConnectView.cpp"
    "src/views/DamageTimePlot.cpp"
    "src/views/RecordingDataView.cpp"
    "src/views/RealtimePlot.cpp"
    "src/views/RecordingGroupView.cpp"
    "src/views/RecordingView.cpp"
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
    PRIVATE qwtinternal)
set_target_properties (ultimate-hindsight
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${UH_RUNTIME_DIR})

if (WIN32)
	target_link_libraries (ultimate-hindsight
		PRIVATE wsock32
		PRIVATE ws2_32)
endif()

# output qwt dll to bin directory
set_target_properties (qwtinternal
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${UH_RUNTIME_DIR})

###############################################################################
# DLLs need to be copied to the runtime directory on Windows
###############################################################################

if (WIN32 OR CYGWIN)
    add_custom_command (TARGET ultimate-hindsight POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${UH_RUNTIME_DIR}/$<CONFIG>/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5Core$<$<CONFIG:Debug>:d>.dll         ${UH_RUNTIME_DIR}/$<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5Gui$<$<CONFIG:Debug>:d>.dll          ${UH_RUNTIME_DIR}/$<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5OpenGL$<$<CONFIG:Debug>:d>.dll       ${UH_RUNTIME_DIR}/$<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../bin/Qt5Widgets$<$<CONFIG:Debug>:d>.dll      ${UH_RUNTIME_DIR}/$<CONFIG>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt5_DIR}/../../../plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll ${UH_RUNTIME_DIR}/$<CONFIG>/platforms

        COMMENT "Copying Qt binaries from '${Qt5_DIR}/../../bin/' to '${UH_RUNTIME_DIR}/$<CONFIG>'"
        VERBATIM)
endif ()
