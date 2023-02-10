macro (reframed_add_plugin PLUGIN)
    set (options "")
    set (oneValueArgs "")
    set (multiValueArgs
        SOURCES 
        HEADERS 
        INCLUDE_DIRECTORIES
        MOC_HEADERS
        FORMS
        DATA)
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
            PRIVATE
                Qt5::Core
                Qt5::Gui
                Qt5::Widgets
                ReFramed::rfplot
                ReFramed::rfqwt
                ReFramed::rfcommon)
        set_target_properties (${PLUGIN}
            PROPERTIES
                PREFIX ""
                OUTPUT_NAME ${${PLUGIN}_OUTPUT_NAME}
                LIBRARY_OUTPUT_DIRECTORY "${REFRAMED_BUILD_PLUGINDIR}"
                LIBRARY_OUTPUT_DIRECTORY_DEBUG "${REFRAMED_BUILD_PLUGINDIR}"
                LIBRARY_OUTPUT_DIRECTORY_RELEASE "${REFRAMED_BUILD_PLUGINDIR}"
                RUNTIME_OUTPUT_DIRECTORY "${REFRAMED_BUILD_PLUGINDIR}"
                RUNTIME_OUTPUT_DIRECTORY_DEBUG "${REFRAMED_BUILD_PLUGINDIR}"
                RUNTIME_OUTPUT_DIRECTORY_RELEASE "${REFRAMED_BUILD_PLUGINDIR}"
                INSTALL_RPATH "${REFRAMED_INSTALL_LIBDIR}"
                VS_DEBUGGER_WORKING_DIRECTORY "${REFRAMED_BUILD_BINDIR}"
                VS_DEBUGGER_COMMAND "${REFRAMED_BUILD_BINDIR}/ReFramed.exe")
        if (${PLUGIN}_DATA)
            foreach (DATAFILE ${${PLUGIN}_DATA})
                add_custom_command (TARGET ${PLUGIN} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${REFRAMED_BUILD_DATADIR}/${PLUGIN}"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/${DATAFILE}" "${REFRAMED_BUILD_DATADIR}/${PLUGIN}"
                    COMMENT "Copying '${DATAFILE}' to '${REFRAMED_BUILD_DATADIR}/${PLUGIN}'"
                    VERBATIM)
            endforeach ()
            install (
                FILES ${${PLUGIN}_DATA}
                DESTINATION "${REFRAMED_INSTALL_DATADIR}/${PLUGIN}")
        endif ()
        set_property (
            DIRECTORY "${PROJECT_SOURCE_DIR}"
            PROPERTY VS_STARTUP_PROJECT ${PLUGIN})
        install (
            TARGETS ${PLUGIN}
            LIBRARY DESTINATION "${REFRAMED_INSTALL_PLUGINDIR}"
            RUNTIME DESTINATION "${REFRAMED_INSTALL_PLUGINDIR}")
    endif ()
endmacro ()

