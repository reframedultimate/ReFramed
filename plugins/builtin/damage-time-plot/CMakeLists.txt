cmake_minimum_required (VERSION 3.2)

project ("plugin-damage-time-plot"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (plugin-damage-time-plot
    SOURCES
        "src/models/DamageTimePlotModel.cpp"
        "src/views/DamageTimePlotView.cpp"
        "src/Plugin.cpp"
    INCLUDE_DIRECTORIES
        "include")

