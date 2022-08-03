list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
set (REFRAMED_PLUGIN_TEMPLATE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/templates")

add_subdirectory ("internal")
#add_subdirectory ("contrib")

install (
    FILES "cmake/modules/ReFramedPlugin.cmake"
    DESTINATION "${REFRAMED_INSTALL_CMAKEDIR}/modules")
install (
    FILES "cmake/templates/PluginConfig.hpp.in"
    DESTINATION "${REFRAMED_INSTALL_CMAKEDIR}/templates")
