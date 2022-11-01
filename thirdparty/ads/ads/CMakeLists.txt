cmake_minimum_required(VERSION 3.5)
project(QtAdvancedDockingSystem LANGUAGES CXX VERSION ${VERSION_SHORT})
#find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
#find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
if (UNIX AND NOT APPLE)
    include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
endif()
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(ADS_BUILD_STATIC)
    set(CMAKE_STATIC_LIBRARY_SUFFIX "_static${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()
set(ads_SRCS
    src/ads_globals.cpp
    src/DockAreaTabBar.cpp
    src/DockAreaTitleBar.cpp
    src/DockAreaWidget.cpp
    src/DockContainerWidget.cpp
    src/DockManager.cpp
    src/DockOverlay.cpp
    src/DockSplitter.cpp
    src/DockWidget.cpp
    src/DockWidgetTab.cpp
    src/DockingStateReader.cpp
    src/DockFocusController.cpp
    src/ElidingLabel.cpp
    src/FloatingDockContainer.cpp
    src/FloatingDragPreview.cpp
    src/IconProvider.cpp
    src/DockComponentsFactory.cpp
    src/ads.qrc
)
set(ads_HEADERS
    include/ads/ads_globals.h
    include/ads/DockAreaTabBar.h
    include/ads/DockAreaTitleBar.h
    include/ads/DockAreaTitleBar_p.h
    include/ads/DockAreaWidget.h
    include/ads/DockContainerWidget.h
    include/ads/DockManager.h
    include/ads/DockOverlay.h
    include/ads/DockSplitter.h
    include/ads/DockWidget.h
    include/ads/DockWidgetTab.h
    include/ads/DockingStateReader.h
    include/ads/DockFocusController.h
    include/ads/ElidingLabel.h
    include/ads/FloatingDockContainer.h
    include/ads/FloatingDragPreview.h
    include/ads/IconProvider.h
    include/ads/DockComponentsFactory.h
)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
if (UNIX AND NOT APPLE)
    set(ads_SRCS src/linux/FloatingWidgetTitleBar.cpp ${ads_SRCS})
    set(ads_HEADERS include/ads/linux/FloatingWidgetTitleBar.h ${ads_HEADERS})
endif()
if(ADS_BUILD_STATIC)
    add_library(qtadvanceddocking STATIC ${ads_SRCS} ${ads_HEADERS})
    target_compile_definitions(qtadvanceddocking PUBLIC ADS_STATIC)
else()
    add_library(qtadvanceddocking SHARED ${ads_SRCS} ${ads_HEADERS})
    target_compile_definitions(qtadvanceddocking PRIVATE ADS_SHARED_EXPORT)
endif()
target_include_directories(qtadvanceddocking PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(ads::qtadvanceddocking ALIAS qtadvanceddocking)

target_link_libraries(qtadvanceddocking PUBLIC Qt${QT_VERSION_MAJOR}::Core 
                                               Qt${QT_VERSION_MAJOR}::Gui 
                                               Qt${QT_VERSION_MAJOR}::Widgets)
if (UNIX AND NOT APPLE)
  target_link_libraries(qtadvanceddocking PUBLIC xcb)
endif()
set_target_properties(qtadvanceddocking PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    CXX_EXTENSIONS OFF
    VERSION ${VERSION_SHORT}
    EXPORT_NAME "qtadvanceddocking"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/bin"
)
if(QT_VERSION_MAJOR STREQUAL "5")
    set_target_properties(qtadvanceddocking PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON)
elseif(QT_VERSION_MAJOR STREQUAL "6")
    set_target_properties(qtadvanceddocking PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
endif()

if (ADS_INSTALL)
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "qtadvanceddockingConfigVersion.cmake"
        VERSION ${VERSION_SHORT}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES ${ads_HEADERS}
        DESTINATION include
        COMPONENT headers
    )
    install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE"
        "${CMAKE_CURRENT_SOURCE_DIR}/../gnu-lgpl-v2.1.md"
        DESTINATION license/ads
        COMPONENT license
    )
    install(TARGETS qtadvanceddocking
        EXPORT adsTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
    )

    install(EXPORT adsTargets
        FILE adsTargets.cmake
        NAMESPACE ads::
        DESTINATION lib/cmake/qtadvanceddocking
    )
    install(FILES qtadvanceddockingConfig.cmake "${CMAKE_CURRENT_BINARY_DIR}/qtadvanceddockingConfigVersion.cmake"
        DESTINATION lib/cmake/qtadvanceddocking
    )
endif ()

