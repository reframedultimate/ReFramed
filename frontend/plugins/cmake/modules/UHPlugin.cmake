macro (uh_add_plugin PLUGIN)
    set (options "")
    set (oneValueArgs "")
    set (multiValueArgs
        SOURCES 
        HEADERS 
        INCLUDE_DIRECTORIES
        MOC_HEADERS
        FORMS)
    cmake_parse_arguments (${PLUGIN} "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	
	option (UH_${PLUGIN} "Enable or disable ${PLUGIN} plugin" ON)
	
	if (${UH_${PLUGIN}})

		include (TestVisibilityMacros)
		test_visibility_macros (
			PLUGIN_API_EXPORT
			PLUGIN_API_IMPORT
			PLUGIN_API_LOCAL)

		find_package (Qt5 COMPONENTS Widgets Gui Core REQUIRED)

		string (REPLACE "plugin-" "" ${PLUGIN}_OUTPUT_NAME ${PLUGIN})

		qt5_wrap_cpp (${PLUGIN}_MOC_SOURCES ${${PLUGIN}_MOC_HEADERS})
		qt5_wrap_ui (${PLUGIN}_MOC_FORMS ${${PLUGIN}_FORMS})

		configure_file ("${PLUGIN_CONFIG_TEMPLATE_PATH}/PluginConfig.hpp.in"
			"${PROJECT_BINARY_DIR}/include/${${PLUGIN}_OUTPUT_NAME}/PluginConfig.hpp")

		add_library (${PLUGIN} SHARED
			${${PLUGIN}_SOURCES}
			${${PLUGIN}_HEADERS}
			${${PLUGIN}_MOC_SOURCES}
			${${PLUGIN}_MOC_FORMS})
		target_include_directories (${PLUGIN}
			PRIVATE
				${${PLUGIN}_INCLUDE_DIRECTORIES}
				"${PROJECT_BINARY_DIR}/include")
		target_compile_definitions (${PLUGIN}
			PRIVATE
				PLUGIN_BUILDING)
		target_link_libraries (${PLUGIN}
			PRIVATE Qt5::Core
			PRIVATE Qt5::Gui
			PRIVATE Qt5::Widgets
			PRIVATE uh)
		set_target_properties (${PLUGIN}
			PROPERTIES
				PREFIX ""
				OUTPUT_NAME ${${PLUGIN}_OUTPUT_NAME}
				LIBRARY_OUTPUT_DIRECTORY "${UH_BUILD_PLUGINDIR}"
				RUNTIME_OUTPUT_DIRECTORY "${UH_BUILD_PLUGINDIR}")
		install (
			TARGETS ${PLUGIN}
			LIBRARY DESTINATION "${UH_INSTALL_PLUGINDIR}"
			RUNTIME DESTINATION "${UH_INSTALL_PLUGINDIR}")
	endif ()
endmacro ()
