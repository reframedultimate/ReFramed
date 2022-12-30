list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
set (REFRAMED_PLUGIN_TEMPLATE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/templates")

configure_file ("cmake/modules/ReFramedPlugin.cmake" "${REFRAMED_BUILD_CMAKEDIR}/modules/ReFramedPlugin.cmake" COPYONLY)
configure_file ("cmake/templates/PluginConfig.hpp.in" "${REFRAMED_BUILD_CMAKEDIR}/templates/PluginConfig.hpp.in" COPYONLY)

install (
    FILES "cmake/modules/ReFramedPlugin.cmake"
    DESTINATION "${REFRAMED_INSTALL_CMAKEDIR}/modules")
install (
    FILES "cmake/templates/PluginConfig.hpp.in"
    DESTINATION "${REFRAMED_INSTALL_CMAKEDIR}/templates")

add_subdirectory ("internal")
#add_subdirectory ("contrib")
