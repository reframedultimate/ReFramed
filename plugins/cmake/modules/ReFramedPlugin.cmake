macro (reframed_add_plugin PLUGIN)
    set (options "")
    set (oneValueArgs "")
    set (multiValueArgs
        SOURCES 
        HEADERS 
        INCLUDE_DIRECTORIES
        MOC_HEADERS
        FORMS)
    cmake_parse_arguments (${PLUGIN} "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	
	option (REFRAMED_${PLUGIN} "Enable or disable ${PLUGIN} plugin" ON)
	
	if (${REFRAMED_${PLUGIN}})

		include (TestVisibilityMacros)
		test_visibility_macros (
			PLUGIN_API_EXPORT
			PLUGIN_API_IMPORT
			PLUGIN_API_LOCAL)

        find_package (Qt5 COMPONENTS Widgets Gui Core PrintSupport Concurrent OpenGL REQUIRED)

		string (REPLACE "plugin-" "" ${PLUGIN}_OUTPUT_NAME ${PLUGIN})

		qt5_wrap_cpp (${PLUGIN}_MOC_SOURCES ${${PLUGIN}_MOC_HEADERS})
		qt5_wrap_ui (${PLUGIN}_MOC_FORMS ${${PLUGIN}_FORMS})

        configure_file ("${REFRAMED_PLUGIN_TEMPLATE_PATH}/PluginConfig.hpp.in"
			"${PROJECT_BINARY_DIR}/include/${${PLUGIN}_OUTPUT_NAME}/PluginConfig.hpp")

		add_library (${PLUGIN} SHARED
			${${PLUGIN}_SOURCES}
			${${PLUGIN}_HEADERS}
			${${PLUGIN}_MOC_SOURCES}
			${${PLUGIN}_MOC_FORMS})
		target_include_directories (${PLUGIN}
			PRIVATE
				${${PLUGIN}_INCLUDE_DIRECTORIES}
                "${PROJECT_BINARY_DIR}/include"  # For generated header files
                "${PROJECT_BINARY_DIR}")         # For MOC files
		target_compile_definitions (${PLUGIN}
			PRIVATE
				PLUGIN_BUILDING)
		target_link_libraries (${PLUGIN}
			PRIVATE Qt5::Core
			PRIVATE Qt5::Gui
			PRIVATE Qt5::Widgets
            PRIVATE ReFramed::rfplot
            PRIVATE ReFramed::rfcommon)
		set_target_properties (${PLUGIN}
			PROPERTIES
				PREFIX ""
				OUTPUT_NAME ${${PLUGIN}_OUTPUT_NAME}
                LIBRARY_OUTPUT_DIRECTORY "${REFRAMED_BUILD_PLUGINDIR}"
				LIBRARY_OUTPUT_DIRECTORY_DEBUG "${REFRAMED_BUILD_PLUGINDIR}"
				LIBRARY_OUTPUT_DIRECTORY_RELEASE "${REFRAMED_BUILD_PLUGINDIR}"
				RUNTIME_OUTPUT_DIRECTORY "${REFRAMED_BUILD_PLUGINDIR}"
				RUNTIME_OUTPUT_DIRECTORY_DEBUG "${REFRAMED_BUILD_PLUGINDIR}"
				RUNTIME_OUTPUT_DIRECTORY_RELEASE "${REFRAMED_BUILD_PLUGINDIR}")
		install (
			TARGETS ${PLUGIN}
			LIBRARY DESTINATION "${REFRAMED_INSTALL_PLUGINDIR}"
			RUNTIME DESTINATION "${REFRAMED_INSTALL_PLUGINDIR}")
	endif ()
endmacro ()
