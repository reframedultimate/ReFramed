include (TestVisibilityMacros)
include (TestBigEndian)
include (CheckCXXSourceCompiles)

project ("rfplot"
    VERSION ${REFRAMED_VERSION}
    LANGUAGES CXX)
    
test_visibility_macros (
    RFPLOT_API_EXPORT 
    RFPLOT_API_IMPORT 
    RFPLOT_API_LOCAL)

configure_file ("cmake/templates/config.hpp.in" "include/rfplot/config.hpp")

find_package (Qt5 COMPONENTS Core Widgets Gui REQUIRED)

qt5_wrap_cpp (RFPLOT_SOURCES_MOC
    "include/rfplot/DeltaPlotPicker.hpp"
    "include/rfplot/Panner.hpp"
    "include/rfplot/RectangleZoomer.hpp"
    "include/rfplot/Zoomer.hpp"
    "include/rfplot/RealtimePlot.hpp")

add_library (rfplot SHARED
    ${RFPLOT_SOURCES_MOC}
    "src/AutoScaler.cpp"
    "src/ColorPalette.cpp"
    "src/DeltaPlotPicker.cpp"
    "src/MousePlotPickerMachine.cpp"
    "src/Panner.cpp"
    "src/RectangleZoomer.cpp"
    "src/Zoomer.cpp"
    "src/RealtimePlot.cpp")

target_include_directories (rfplot
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:include>)
target_compile_definitions (rfplot
    PRIVATE RFPLOT_BUILDING
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options (rfplot
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>)
target_compile_features (rfplot
    PUBLIC cxx_std_17)
target_link_libraries (rfplot
    PRIVATE Qt5::Core
    PRIVATE Qt5::Gui
    PRIVATE Qt5::Widgets
    PUBLIC ReFramed::rfqwt
    PUBLIC ReFramed::rfcommon)
set_target_properties (rfplot
    PROPERTIES
        DEBUG_POSTFIX "d"
        LIBRARY_OUTPUT_DIRECTORY ${REFRAMED_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${REFRAMED_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${REFRAMED_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${REFRAMED_BUILD_BINDIR})

add_library (ReFramed::rfplot ALIAS rfplot)

install (
    TARGETS rfplot
    EXPORT ReFramedTargets
    INCLUDES DESTINATION ${REFRAMED_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${REFRAMED_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${REFRAMED_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${REFRAMED_INSTALL_BINDIR})
install (
    DIRECTORY "include/rfplot"
    DESTINATION ${REFRAMED_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/rfplot"
    DESTINATION ${REFRAMED_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp")
export (
    TARGETS rfplot
    NAMESPACE ReFramed::
    FILE ReFramedTargets.cmake)
