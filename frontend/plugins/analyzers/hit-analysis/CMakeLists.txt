include (UHPlugin)

project ("plugin-hit-analysis"
    VERSION 0.0.1
    LANGUAGES CXX)

uh_add_plugin (plugin-hit-analysis
    SOURCES
        "src/Plugin.cpp"
    INCLUDE_DIRECTORIES
        "include")

