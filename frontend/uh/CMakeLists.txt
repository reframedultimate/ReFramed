project ("uh-application"
    VERSION 1.0.0
    LANGUAGES C CXX)

include (GNUInstallDirs)

find_package (Qt5Widgets REQUIRED)
find_package (Qt5Gui REQUIRED)
find_package (Qt5Network REQUIRED)
find_package (Qt5LinguistTools REQUIRED)

qt5_wrap_cpp (UH_SOURCES_MOC
    "include/uh/models/Protocol.hpp"
    "include/uh/views/ActiveRecordingView.hpp"
    "include/uh/views/ConnectView.hpp"
    "include/uh/views/MainWindow.hpp"
    "include/uh/views/RecordingView.hpp")
qt5_wrap_ui (UH_FORMS_MOC
    "forms/ActiveRecordingView.ui"
    "forms/ConnectView.ui"
    "forms/RecordingView.ui"
    "forms/MainWindow.ui")
qt5_add_resources (UH_RESOURCES_GEN
    "")
qt5_add_translation (UH_QM_FILES
    "")

add_executable (ultimate-hindsight
    ${UH_SOURCES_MOC}
    ${UH_FORMS_MOC}
    ${UH_RESOURCES_GEN}
    ${UH_QM_FILES}
    "src/main.cpp"
    "src/models/FighterStatusMapping.cpp"
    "src/models/FighterIDMapping.cpp"
    "src/models/StageIDMapping.cpp"
    "src/models/GameInfo.cpp"
    "src/models/MappingInfo.cpp"
    "src/models/PlayerInfo.cpp"
    "src/models/PlayerState.cpp"
    "src/models/Protocol.cpp"
    "src/models/Recording.cpp"
    "src/models/Settings.cpp"
    "src/platform/tcp_socket.c"
    "src/views/ActiveRecordingView.cpp"
    "src/views/ConnectView.cpp"
    "src/views/RecordingView.cpp"
    "src/views/MainWindow.cpp")
target_include_directories (ultimate-hindsight
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_options (ultimate-hindsight
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-W -Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-W -Wall -Wextra -pedantic>)
target_link_libraries (ultimate-hindsight
    PRIVATE Qt5::Core
    PRIVATE Qt5::Gui
    PRIVATE Qt5::Network
    PRIVATE Qt5::Widgets)
set_target_properties (ultimate-hindsight
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${UH_RUNTIME_DIR})
