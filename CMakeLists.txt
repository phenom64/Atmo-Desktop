cmake_minimum_required(VERSION 2.8.11)
option(QT5BUILD "build for qt5" OFF)

if (QT5BUILD)
    set(PROJECT styleproject_qt5)
else (QT5BUILD)
    set(PROJECT styleproject)
endif (QT5BUILD)

project(${PROJECT})

find_package(X11 REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (QT5BUILD)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5X11Extras)
    find_package(Qt5DBus)
    include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5X11Extras_INCLUDE_DIRS} /usr/include/qt/QtWidgets/5.4.1/QtWidgets /usr/include/qt/QtCore/5.4.1/QtCore/)
    if (Qt5DBus_FOUND)
        include_directories(${Qt5DBus_INCLUDE_DIRS})
        add_definitions(${Qt5DBus_DEFINITIONS})
        set(QTDBUS_FOUND ON)
    endif (Qt5DBus_FOUND)
    add_definitions(${Qt5Widgets_DEFINITIONS})
    
    # uninstall target, stolen from http://www.cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
else (QT5BUILD)
    find_package(Qt4 REQUIRED)
    find_package(KDE4)
    include(${QT_USE_FILE})
    add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})
    set(QTDBUS_FOUND ${QT_QTDBUS_FOUND})
    if (KDE4_FOUND)
	add_definitions(${KDE4_DEFINITIONS})
	include_directories(${KDE4_INCLUDES})
	include(KDE4Defaults)
	include(MacroLibrary)
    endif (KDE4_FOUND)
endif (QT5BUILD)

add_subdirectory(config)

#source
# file(GLOB SRCS *.cpp)
#file(GLOB HDRS *.h)
set(SRCS buttons.cpp contentssize.cpp events.cpp init.cpp inputs.cpp items.cpp overlay.cpp panels.cpp pixelmetric.cpp polish.cpp sliders.cpp stylehint.cpp styleproject.cpp subrects.cpp tabs.cpp)
set(HDRS overlay.h styleproject.h)
set(MOC_HDRS overlay.h styleproject.h)

#stylelib source...
file(GLOB_RECURSE libSRCS stylelib/*.cpp)
set(SRCS ${SRCS} ${libSRCS})
file(GLOB_RECURSE libHDRS stylelib/*.h)
set(HDRS ${HDRS} ${libHDRS})
set(MOC_HDRS ${MOC_HDRS} stylelib/animhandler.h stylelib/handlers.h stylelib/ops.h stylelib/progresshandler.h stylelib/widgets.h)

#config source...
set(SRCS ${SRCS} config/settings.cpp)
set(HDRS ${HDRS} config/settings.h)
set(MOC_HDRS ${MOC_HDRS} config/settings.h)

if (QTDBUS_FOUND)
    message(STATUS "Found qtdbus, building bespin macmenu support")
    set(SRCS ${SRCS} macmenu.cpp)
    set(MOC_HDRS ${MOC_HDRS} macmenu.h macmenu-dbus.h)
endif (QTDBUS_FOUND)

set_property(SOURCE moc_styleproject.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/styleproject.json)

if (QT5BUILD)
    qt5_wrap_cpp(MOC_SRCS ${MOC_HDRS})
else (QT5BUILD)
    qt4_wrap_cpp(MOC_SRCS ${MOC_HDRS})
endif (QT5BUILD)

add_library(${PROJECT} SHARED ${SRCS} ${MOC_SRCS} ${HDRS})
# set_target_properties(${PROJECT} PROPERTIES AUTOMOC ON)

if (QT5BUILD)
    target_link_libraries(${PROJECT} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus Qt5X11Extras ${X11_X11_LIB} ${X11_LIBRARIES})
    get_target_property(QMAKE Qt5::qmake IMPORTED_LOCATION)
    exec_program(${QMAKE} ARGS "-query QT_INSTALL_PLUGINS" OUTPUT_VARIABLE PLUGINPATH)
    install(TARGETS ${PROJECT} LIBRARY DESTINATION ${PLUGINPATH}/styles)
else (QT5BUILD)
    target_link_libraries(${PROJECT} ${QT_LIBRARIES} ${QT_QTDBUS_LIBRARY} ${X11_LIBRARIES})
    install(TARGETS ${PROJECT} LIBRARY DESTINATION ${QT_PLUGINS_DIR}/styles)
    install(FILES styleproject.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
endif (QT5BUILD)

if (KDE4_FOUND)
    add_subdirectory(kwin)
endif (KDE4_FOUND)
