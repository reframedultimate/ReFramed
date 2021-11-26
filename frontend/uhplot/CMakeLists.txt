include (TestVisibilityMacros)
include (TestBigEndian)
include (CheckCXXSourceCompiles)

project ("uhplot"
    VERSION 0.0.1
    LANGUAGES CXX)
    
test_visibility_macros (
    UHPLOT_API_EXPORT 
    UHPLOT_API_IMPORT 
    UHPLOT_API_LOCAL)

configure_file ("templates/config.hpp.in" "include/uhplot/config.hpp")

find_package (Qt5 COMPONENTS Core Widgets Gui REQUIRED)

qt5_wrap_cpp (UHPLOT_SOURCES_MOC
    "include/uhplot/DeltaPlotPicker.hpp"
    "include/uhplot/Panner.hpp"
    "include/uhplot/RectangleZoomer.hpp"
    "include/uhplot/Zoomer.hpp"
    "include/uhplot/RealtimePlot.hpp")

add_library (uhplot SHARED
    ${UHPLOT_SOURCES_MOC}
    "src/AutoScaler.cpp"
    "src/ColorPalette.cpp"
    "src/DeltaPlotPicker.cpp"
    "src/MousePlotPickerMachine.cpp"
    "src/Panner.cpp"
    "src/RectangleZoomer.cpp"
    "src/Zoomer.cpp"
    "src/RealtimePlot.cpp")

target_include_directories (uhplot
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:include>)
target_compile_definitions (uhplot
    PRIVATE UHPLOT_BUILDING
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options (uhplot
    PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>)
target_compile_features (uhplot
    PUBLIC cxx_std_17)
target_link_libraries (uhplot
    PRIVATE Qt5::Core
    PRIVATE Qt5::Gui
    PRIVATE Qt5::Widgets
    PUBLIC uhqwt
    PUBLIC uh)
set_target_properties (uhplot
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${UH_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${UH_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${UH_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${UH_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${UH_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${UH_BUILD_BINDIR})

install (
    TARGETS uhplot
    EXPORT UHPlotConfig
    LIBRARY DESTINATION ${UH_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${UH_INSTALL_BINDIR})
install (
    DIRECTORY "include/uhplot"
    DESTINATION ${UH_INSTALL_INCLUDEDIR})
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/include/uhplot"
    DESTINATION ${UH_INSTALL_INCLUDEDIR})
install (
    EXPORT UHPlotConfig
    DESTINATION ${UH_INSTALL_CMAKEDIR})

