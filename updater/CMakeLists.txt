project ("updater"
    VERSION 1.0.0
    LANGUAGES C CXX)

include (GNUInstallDirs)

find_package (Qt6 COMPONENTS Core Widgets Gui LinguistTools REQUIRED)

qt5_wrap_cpp (UPDATER_SOURCES_MOC
    "include/updater/views/DownloadView.hpp")
qt5_wrap_ui (UPDATER_FORMS_MOC
    "")
qt5_add_resources (UPDATER_RESOURCES_GEN
    "")
qt5_add_translation (UPDATER_QM_FILES
    "")

add_executable (uh-updater WIN32
    ${UPDATER_SOURCES_MOC}
    ${UPDATER_FORMS_MOC}
    ${UPDATER_RESOURCES_GEN}
    ${UPDATER_QM_FILES}
    "src/views/DownloadView.cpp"
    "src/models/DownloadModel.cpp"
    "src/main.cpp")
target_include_directories (uh-updater
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_compile_options (uh-updater
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-W -Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-W -Wall -Wextra -pedantic>)
target_link_libraries (uh-updater
    PRIVATE Qt6::Core
    PRIVATE Qt6::Gui
    PRIVATE Qt6::Widgets)
set_target_properties (uh-updater
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${UH_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${UH_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${UH_BUILD_BINDIR})

###############################################################################
# DLLs need to be copied to the runtime directory on Windows
###############################################################################

if (WIN32 OR CYGWIN)
    add_custom_command (TARGET uh-updater POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${UH_BUILD_BINDIR}/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_DIR}/../../../bin/Qt5Core$<$<CONFIG:Debug>:d>.dll         ${UH_BUILD_BINDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_DIR}/../../../bin/Qt5Gui$<$<CONFIG:Debug>:d>.dll          ${UH_BUILD_BINDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_DIR}/../../../bin/Qt5OpenGL$<$<CONFIG:Debug>:d>.dll       ${UH_BUILD_BINDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_DIR}/../../../bin/Qt5Widgets$<$<CONFIG:Debug>:d>.dll      ${UH_BUILD_BINDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Qt6_DIR}/../../../plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll ${UH_BUILD_BINDIR}/platforms

        COMMENT "Copying Qt binaries from '${Qt6_DIR}/../../bin/' to '${UH_BUILD_BINDIR}'"
        VERBATIM)
endif ()

install (
    TARGETS uh-updater
    RUNTIME DESTINATION "${UH_INSTALL_BINDIR}")
set_target_properties (uh-updater
    PROPERTIES
        INSTALL_RPATH ${UH_INSTALL_LIBDIR})

