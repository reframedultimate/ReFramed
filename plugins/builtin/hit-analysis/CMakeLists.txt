cmake_minimum_required (VERSION 3.2)

project ("plugin-hit-analysis"
    VERSION 0.0.1
    LANGUAGES CXX)

include (ReFramedPlugin)

reframed_add_plugin (hit-analysis
    SOURCES
        "src/Plugin.cpp"
        "src/HitAnalyzer.cpp"
    MOC_HEADERS
        "include/hit-analysis/HitAnalyzer.hpp"
    FORMS
        "forms/HitAnalyzer.ui"
    INCLUDE_DIRECTORIES
        "include")

