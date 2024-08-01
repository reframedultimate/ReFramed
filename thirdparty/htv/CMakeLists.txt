cmake_minimum_required (VERSION 3.18)

project ("HierarchicalTableView"
    LANGUAGES CXX)

option (HTV_EXAMPLES "Build examples" OFF)

find_package (Qt6 COMPONENTS Core Gui Widgets)

add_subdirectory ("htv")
if (HTV_EXAMPLES)
    add_subdirectory ("examples")
endif ()

