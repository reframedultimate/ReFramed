project ("htv"
    VERSION 1.0.0
    LANGUAGES CXX)

option (HTV_PIC "Generate position independent code" ON)

set (HTV_HEADERS_MOC
    "include/htv/HierarchicalHeaderView.h")
set (HTV_SOURCES
    "src/HierarchicalHeaderView.cpp")

qt_wrap_cpp (HTV_SOURCES_MOC ${HTV_HEADERS_MOC})

add_library (htv
    ${HTV_HEADERS_MOC}
    ${HTV_SOURCES}
    ${HTV_SOURCES_MOC})
target_include_directories (htv
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)
target_link_libraries (htv
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Widgets)
set_target_properties (htv
    PROPERTIES
        POSITION_INDEPENDENT_CODE ${HTV_PIC})

