include (UHPlugin)

project ("plugin-hit-analysis"
    VERSION 0.0.1
    LANGUAGES CXX)

uh_add_plugin (plugin-hit-analysis
    SOURCES
        "src/Plugin.cpp"
        "src/HitAnalyzer.cpp"
    MOC_HEADERS
        "include/hit-analysis/HitAnalyzer.hpp"
    FORMS
        "forms/HitAnalyzer.ui"
    INCLUDE_DIRECTORIES
        "include")

