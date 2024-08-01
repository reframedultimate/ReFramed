cmake_minimum_required(VERSION 3.5)
project(QtAdvancedDockingSystem LANGUAGES CXX VERSION ${VERSION_SHORT})
include(GNUInstallDirs)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
if (UNIX AND NOT APPLE)
    include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})
endif()
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(BUILD_STATIC)
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
    src/AutoHideSideBar.cpp
    src/AutoHideTab.cpp
    src/AutoHideDockContainer.cpp
    src/PushButton.cpp
    src/ResizeHandle.cpp
    ads.qrc
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
    include/ads/AutoHideSideBar.h
    include/ads/AutoHideTab.h
    include/ads/AutoHideDockContainer.h
    include/ads/PushButton.h
    include/ads/ResizeHandle.h
)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
if (UNIX AND NOT APPLE)
    set(ads_SRCS src/linux/FloatingWidgetTitleBar.cpp ${ads_SRCS})
    set(ads_HEADERS include/ads/linux/FloatingWidgetTitleBar.h ${ads_HEADERS})
endif()

set(library_name "qt6advanceddocking")
if(BUILD_STATIC)
    add_library(${library_name} STATIC ${ads_SRCS} ${ads_HEADERS})
    target_compile_definitions( ${library_name} PUBLIC ADS_STATIC)
else()
    add_library( ${library_name} SHARED ${ads_SRCS} ${ads_HEADERS})
    target_compile_definitions( ${library_name} PRIVATE ADS_SHARED_EXPORT)
endif()

add_library(ads::${library_name} ALIAS ${library_name})

target_link_libraries(${library_name}
    PUBLIC
        Qt6::Core 
        Qt6::Gui 
        Qt6::Widgets)
if (UNIX AND NOT APPLE)
  if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
      find_package(X11 REQUIRED)
      target_link_libraries(${library_name} PUBLIC X11::xcb)
  else()
    target_link_libraries(${library_name} PUBLIC xcb)
  endif()
endif()
set_target_properties(${library_name} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    CXX_EXTENSIONS OFF
    VERSION ${VERSION_SHORT}
    EXPORT_NAME ${library_name}
    DEBUG_POSTFIX "d"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/bin"
)
if(QT_VERSION_MAJOR STREQUAL "5")
    set_target_properties(${library_name} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON)
elseif(QT_VERSION_MAJOR STREQUAL "6")
    set_target_properties(${library_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${library_name}ConfigVersion.cmake"
    VERSION ${VERSION_SHORT}
    COMPATIBILITY SameMajorVersion
)
install(FILES ${ads_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${library_name}
    COMPONENT headers
)
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/../gnu-lgpl-v2.1.md"
    DESTINATION license/ads
    COMPONENT license
)
install(TARGETS ${library_name}
    EXPORT adsTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${library_name}
)

install(EXPORT adsTargets
    FILE adsTargets.cmake
    NAMESPACE ads::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${library_name}
)
install(FILES qtadvanceddockingConfig.cmake RENAME ${library_name}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${library_name}
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${library_name}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${library_name}
)

target_include_directories(${library_name} PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
