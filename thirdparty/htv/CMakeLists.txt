cmake_minimum_required (VERSION 3.18)

project ("HierarchicalTableView"
    LANGUAGES CXX)

option (HTV_EXAMPLES "Build examples" OFF)

if (NOT QT5_FOUND AND NOT QT6_FOUND)
    find_package (Qt6 COMPONENTS Core Gui Widgets)
endif ()
if (NOT QT5_FOUND AND NOT QT6_FOUND)
    find_package (Qt5 REQUIRED COMPONENTS Core Gui Widgets)
endif ()

add_subdirectory ("htv")
if (HTV_EXAMPLES)
    add_subdirectory ("examples")
endif ()

