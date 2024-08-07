cmake_minimum_required (VERSION 3.25)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif ()

project ("Qwt"
    VERSION 6.3.0
    LANGUAGES CXX)

list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

option (QWT_PIC "Generate position independent code" ON)
set (QWT_LIB_TYPE "SHARED" CACHE STRING "Set either as SHARED or STATIC")
option (QWT_PLOT "Enables all classes that are needed to use the QwtPlot widget" ON)
option (QWT_POLAR "QwtPolar enables all classes, that are needed to use the QwtPolar classes. It can't be enabled without also enabling QwtPlot" ON)
option (QWT_WIDGETS "Enables all classes that are needed to use all the other widgets (sliders, dials, ...) besides QwtPlot" ON)
option (QWT_SVG "If you want to display svg images on the plot canvas, or export a plot to an SVG document" ON)
option (QWT_OPENGL "If you want to use an OpenGL plot canvas" ON)
#option (QWT_MATHML "You can use the MathML renderer of the Qt solutions package to enable MathML support in Qwt. Because of license implications the ( modified ) code of the MML Widget solution is included and linked together with the QwtMathMLTextEngine into an own library." OFF)
option (QWT_DESIGNER "If you want to build the Qwt designer plugin. Otherwise you have to build it from the designer directory." ON)
option (QWT_EXAMPLES "If you want to auto build the examples" OFF)
option (QWT_PLAYGROUND "The playground is primarily intended for the Qwt development to explore and test new features. Nevertheless you might find ideas or code snippets that help for application development. If you want to auto build the applications in playground, enable this option." OFF)
option (QWT_TESTS "If you want to auto build the tests" OFF)

include (TestVisibilityMacros)
test_visibility_macros (QWT_API_EXPORT QWT_API_IMPORT QWT_API_LOCAL)

configure_file ("templates/qwt_global.h.in" "src/qwt_global.h")

find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport Concurrent)

qt_wrap_cpp (QWT_CORE_SOURCES_MOC
    "src/qwt_dyngrid_layout.h"
    "src/qwt_magnifier.h"
    "src/qwt_text_label.h"
    "src/qwt_widget_overlay.h")

if (QWT_PLOT)
	qt_wrap_cpp (QWT_PLOT_SOURCES_MOC
        "src/qwt_abstract_legend.h"
        "src/qwt_legend.h"
        "src/qwt_legend_label.h"
        "src/qwt_panner.h"
        "src/qwt_picker.h"
        "src/qwt_plot.h"
        "src/qwt_plot_canvas.h"
	    # NOTE: qwt_plot_glcanvas.h needs to be used if Qt5
        "src/qwt_plot_opengl_canvas.h"
        "src/qwt_plot_magnifier.h"
        "src/qwt_plot_panner.h"
        "src/qwt_plot_picker.h"
        "src/qwt_plot_renderer.h"
        "src/qwt_plot_rescaler.h"
        "src/qwt_plot_zoomer.h"
        "src/qwt_sampling_thread.h"
        "src/qwt_scale_widget.h")
endif ()

if (QWT_POLAR)

	qt_wrap_cpp (QWT_POLAR_SOURCES_MOC
        "src/qwt_polar_canvas.h"
        "src/qwt_polar_magnifier.h"
        "src/qwt_polar_panner.h"
        "src/qwt_polar_picker.h"
        "src/qwt_polar_plot.h"
        "src/qwt_polar_renderer.h")
endif ()

if (QWT_OPENGL)
    find_package (Qt6 COMPONENTS OpenGLWidgets REQUIRED)
    qt_wrap_cpp (QWT_OPENGL_SOURCES_MOC
        "src/qwt_plot_opengl_canvas.h")
endif ()

if (QWT_SVG)
    find_package (Qt6 COMPONENTS Svg REQUIRED)
endif ()

if (QWT_WIDGETS)
	find_package (Qt6 COMPONENTS Widgets REQUIRED)
	qt_wrap_cpp (QWT_WIDGETS_SOURCES_MOC
        "src/qwt_abstract_scale.h"
        "src/qwt_abstract_slider.h"
        "src/qwt_analog_clock.h"
        "src/qwt_compass.h"
        "src/qwt_counter.h"
        "src/qwt_dial.h"
        "src/qwt_knob.h"
        "src/qwt_slider.h"
        "src/qwt_thermo.h"
        "src/qwt_wheel.h")
endif ()

add_library (rfqwt ${QWT_LIB_TYPE}
    ###########################################################################
    # core
    ###########################################################################

    ${QWT_CORE_SOURCES_MOC}
    "src/qwt.cpp"
    "src/qwt_abstract_scale_draw.cpp"
    "src/qwt_bezier.cpp"
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
    "src/qwt_scale_engine.cpp"
    "src/qwt_spline.cpp"
    "src/qwt_spline_basis.cpp"
    "src/qwt_spline_parametrization.cpp"
    "src/qwt_spline_local.cpp"
    "src/qwt_spline_cubic.cpp"
    "src/qwt_spline_pleasing.cpp"
    "src/qwt_spline_polynomial.cpp"
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
    $<$<BOOL:${QWT_PLOT}>:src/qwt_spline_curve_fitter.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_weeding_curve_fitter.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_abstract_legend.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_legend.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_legend_data.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_legend_label.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_renderer.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_axis.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_curve.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_dict.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_directpainter.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_graphicitem.cpp>
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
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_vectorfield.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_marker.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_textlabel.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_layout.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_plot_abstract_canvas.cpp>
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
    $<$<BOOL:${QWT_PLOT}>:src/qwt_vectorfield_symbol.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_sampling_thread.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_series_data.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_point_data.cpp>
    $<$<BOOL:${QWT_PLOT}>:src/qwt_scale_widget.cpp>

    ###########################################################################
    # QwtPolar
    ###########################################################################
	
	$<$<BOOL:${QWT_POLAR}>:${QWT_POLAR_SOURCES_MOC}>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_canvas.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_curve.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_fitter.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_grid.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_item.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_itemdict.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_layout.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_magnifier.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_marker.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_panner.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_picker.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_plot.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_renderer.cpp>
    $<$<BOOL:${QWT_POLAR}>:src/qwt_polar_spectrogram.cpp>

    ###########################################################################
    # OpenGL
    ###########################################################################

    $<$<BOOL:${QWT_OPENGL}>:${QWT_OPENGL_SOURCES_MOC}>
    $<$<BOOL:${QWT_OPENGL}>:src/qwt_plot_opengl_canvas.cpp>
	
	# NOTE: qwt_plot_glcanvas.cpp needs to be used if Qt5

    ###########################################################################
    # SVG
    ###########################################################################

    $<$<BOOL:${QWT_SVG}>:src/qwt_plot_svgitem.cpp>

    ###########################################################################
    # QwtWidgets
    ###########################################################################

	$<$<BOOL:${QWT_WIDGETS}>:${QWT_WIDGETS_SOURCES_MOC}>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_abstract_slider.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_abstract_scale.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_arrow_button.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_analog_clock.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_compass.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_compass_rose.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_counter.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_dial.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_dial_needle.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_knob.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_slider.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_thermo.cpp>
    $<$<BOOL:${QWT_WIDGETS}>:src/qwt_wheel.cpp>)
target_include_directories (rfqwt
    PUBLIC 
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
		$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
		$<INSTALL_INTERFACE:include>)
target_link_libraries (rfqwt
    PUBLIC 
		Qt6::Core
		Qt6::Gui
		Qt6::PrintSupport
		Qt6::Concurrent
		$<$<BOOL:${QWT_SVG}>:Qt6::Svg>
        $<$<BOOL:${QWT_OPENGL}>:Qt6::OpenGLWidgets>
		$<$<BOOL:${QWT_WIDGETS}>:Qt6::Widgets>)
target_compile_definitions (rfqwt
    PRIVATE 
		QWT_BUILDING
		$<$<NOT:$<BOOL:${QWT_SVG}>>:QWT_NO_SVG>
        $<$<NOT:$<BOOL:${QWT_OPENGL}>>:QWT_NO_OPENGL>)
set_target_properties (rfqwt
    PROPERTIES
		CXX_STANDARD 17
        DEBUG_POSTFIX "d"
        LIBRARY_OUTPUT_DIRECTORY ${REFRAMED_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${REFRAMED_BUILD_LIBDIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${REFRAMED_BUILD_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${REFRAMED_BUILD_BINDIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${REFRAMED_BUILD_BINDIR})

add_library (ReFramed::rfqwt ALIAS rfqwt)

install (
    TARGETS rfqwt
    EXPORT ReFramedTargets
    INCLUDES DESTINATION ${REFRAMED_INSTALL_INCLUDEDIR}
    ARCHIVE DESTINATION ${REFRAMED_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${REFRAMED_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${REFRAMED_INSTALL_BINDIR})
install (
    DIRECTORY "src/"
    DESTINATION ${REFRAMED_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")
install (
    DIRECTORY "${PROJECT_BINARY_DIR}/src/"
    DESTINATION ${REFRAMED_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")

