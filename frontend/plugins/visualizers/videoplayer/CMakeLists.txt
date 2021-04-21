include (UHPlugin)

project ("plugin-videoplayer"
    VERSION 0.0.1
    LANGUAGES CXX)

uh_add_plugin (videoplayer
    SOURCES
        "src/Plugin.cpp"
    INCLUDE_DIRECTORIES
        "include")

