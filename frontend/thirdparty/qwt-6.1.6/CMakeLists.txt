cmake_minimum_required (VERSION 3.14)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

project ("Qwt"
    VERSION 6.1.6
    LANGUAGES CXX)

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

option (QWT_PIC "Generate position independent code" ON)
set (QWT_LIB_TYPE "SHARED" CACHE STRING "Set either as SHARED or STATIC")
option (QWT_PLOT "Enables all classes that are needed to use the QwtPlot widget" ON)
option (QWT_WIDGETS "Enables all classes that are needed to use all the other widgets (sliders, dials, ...) besides QwtPlot" ON)
option (QWT_SVG "If you want to display svg images on the plot canvas, or export a plot to an SVG document" OFF)
option (QWT_OPENGL "If you want to use an OpenGL plot canvas" ON)
#option (QWT_MATHML "You can use the MathML renderer of the Qt solutions package to enable MathML support in Qwt. Because of license implications the ( modified ) code of the MML Widget solution is included and linked together with the QwtMathMLTextEngine into an own library." OFF)
option (QWT_DESIGNER "If you want to build the Qwt designer plugin. Otherwise you have to build it from the designer directory." ON)

include (TestVisibilityMacros)
test_visibility_macros (QWT_API_EXPORT QWT_API_IMPORT QWT_API_LOCAL)

configure_file ("templates/qwt_global.h.in" "src/qwt_global.h")

find_package (Qt5 COMPONENTS Core Gui Widgets PrintSupport Concurrent REQUIRED)

qt5_wrap_cpp (QWT_CORE_SOURCES_MOC
    "src/qwt_dyngrid_layout.h"
    "src/qwt_magnifier.h"
    "src/qwt_panner.h"
    "src/qwt_picker.h"
    "src/qwt_text_label.h")

if (QWT_PLOT)
	qt5_wrap_cpp (QWT_PLOT_SOURCES_MOC
        "src/qwt_abstract_legend.h"
        "src/qwt_legend.h"
        "src/qwt_legend_label.h"
        "src/qwt_plot.h"
        "src/qwt_plot_renderer.h"
        "src/qwt_plot_canvas.h"
        "src/qwt_plot_panner.h"
        "src/qwt_plot_picker.h"
        "src/qwt_plot_zoomer.h"
        "src/qwt_plot_magnifier.h"
        "src/qwt_sampling_thread.h"
        "src/qwt_scale_widget.h")
endif ()

if (QWT_OPENGL)
    find_package (Qt5 COMPONENTS OpenGL REQUIRED)
    qt5_wrap_cpp (QWT_OPENGL_SOURCES_MOC
        "src/qwt_plot_glcanvas.h")
endif ()

if (QWT_SVG)
    find_package (Qt5 COMPONENTS Svg REQUIRED)
endif ()

add_library (uhqwt ${QWT_LIB_TYPE}
    ###########################################################################
    # core
    ###########################################################################

    ${QWT_CORE_SOURCES_MOC}
    "src/qwt_abstract_scale_draw.cpp"
    "src/qwt_clipper.cpp"
    "src/qwt_color_map.cpp"
    "src/qwt_column_symbol.cpp"
    "src/qwt_date.cpp"
    "src/qwt_date_scale_draw.cpp"
    "src/qwt_date_scale_engine.cpp"
    "src/qwt_dyngrid_layout.cpp"
    "src/qwt_event_pattern.cpp"
    "src/qwt_graphic.cpp"
    "src/qwt_interval.cpp"
    "src/qwt_interval_symbol.cpp"
    "src/qwt_math.cpp"
    "src/qwt_magnifier.cpp"
    "src/qwt_null_paintdevice.cpp"
    "src/qwt_painter.cpp"
    "src/qwt_painter_command.cpp"
    "src/qwt_panner.cpp"
    "src/qwt_picker.cpp"
    "src/qwt_picker_machine.cpp"
    "src/qwt_pixel_matrix.cpp"
    "src/qwt_point_3d.cpp"
    "src/qwt_point_polar.cpp"
    "src/qwt_round_scale_draw.cpp"
    "src/qwt_scale_div.cpp"
    "src/qwt_scale_draw.cpp"
    "src/qwt_scale_map.cpp"
    "src/qwt_spline.cpp"
    "src/qwt_scale_engine.cpp"
    "src/qwt_symbol.cpp"
    "src/qwt_system_clock.cpp"
    "src/qwt_text_engine.cpp"
    "src/qwt_text_label.cpp"
    "src/qwt_text.cpp"
    "src/qwt_transform.cpp"
    "src/qwt_widget_overlay.cpp"

    ###########################################################################
    # plot
    ###########################################################################

    $<$<BOOL:${QWT_PLOT}>:${QWT_PLOT_SOURCES_MOC}>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_curve_fitter.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_abstract_legend.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_legend.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_legend_data.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_legend_label.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_renderer.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_xml.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_axis.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_curve.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_dict.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_directpainter.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_grid.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_histogram.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_item.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_abstract_barchart.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_barchart.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_multi_barchart.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_intervalcurve.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_zoneitem.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_tradingcurve.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_spectrogram.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_spectrocurve.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_scaleitem.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_legenditem.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_seriesitem.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_shapeitem.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_marker.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_textlabel.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_layout.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_canvas.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_panner.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_rasteritem.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_picker.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_zoomer.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_magnifier.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_plot_rescaler.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_point_mapper.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_raster_data.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_matrix_raster_data.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_sampling_thread.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_series_data.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_point_data.cpp>
	$<$<BOOL:${QWT_PLOT}>:src/qwt_scale_widget.cpp>

    ###########################################################################
    # OpenGL
    ###########################################################################

    $<$<BOOL:${QWT_OPENGL}>:${QWT_OPENGL_SOURCES_MOC}>
    $<$<BOOL:${QWT_OPENGL}>:src/qwt_plot_glcanvas.cpp>

    ###########################################################################
    # SVG
    ###########################################################################

    $<$<BOOL:${QWT_SVG}>:src/qwt_plot_svgitem.cpp>)
target_include_directories (uhqwt
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
    PUBLIC $<INSTALL_INTERFACE:include>)
target_link_libraries (uhqwt
    PRIVATE Qt5::Core
    PRIVATE Qt5::Gui
    PRIVATE Qt5::Widgets
    PRIVATE Qt5::PrintSupport
    PRIVATE Qt5::Concurrent
    PRIVATE $<$<BOOL:${QWT_SVG}>:Qt5::Svg>
    PRIVATE $<$<BOOL:${QWT_OPENGL}>:Qt5::OpenGL>)
target_compile_definitions (uhqwt
    PRIVATE QWT_BUILDING
    PRIVATE $<$<NOT:$<BOOL:${QWT_SVG}>>:QWT_NO_SVG>)
set_target_properties (uhqwt
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${UH_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${UH_BUILD_BINDIR})

install (
    TARGETS uhqwt
    EXPORT uhqwtConfig
    LIBRARY DESTINATION ${UH_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${UH_INSTALL_BINDIR})
install (
    DIRECTORY "src"
    DESTINATION ${UH_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/src"
    DESTINATION ${UH_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")
install (
    EXPORT uhqwtConfig
    DESTINATION ${UH_INSTALL_CMAKEDIR})

