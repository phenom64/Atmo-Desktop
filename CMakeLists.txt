cmake_minimum_required(VERSION 3.5)
project(atmo)

option(DEBUG "verbose debugging messages in output" OFF)

if (DEBUG)
    add_definitions(-DDEBUG=1)
endif ()

if (NOT LIBDIR)
    set(LIBDIR lib)
endif ()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5X11Extras)
find_package(Qt5OpenGL)
find_package(Qt5DBus)
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5X11Extras_INCLUDE_DIRS})
if (Qt5DBus_FOUND)
    include_directories(${Qt5DBus_INCLUDE_DIRS})
    add_definitions(${Qt5DBus_DEFINITIONS})
    set(QTDBUS_FOUND ON)
endif ()
add_definitions(${Qt5Widgets_DEFINITIONS})
find_package(ECM)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

add_subdirectory(config)

# source
set(SRCS buttons.cpp contentssize.cpp events.cpp init.cpp inputs.cpp items.cpp overlay.cpp panels.cpp pixelmetric.cpp polish.cpp sliders.cpp stylehint.cpp nse.cpp subrects.cpp tabs.cpp)
set(HDRS overlay.h nse.h namespace.h)
set(MOC_HDRS overlay.h nse.h)

if (QTDBUS_FOUND)
    message(STATUS "Found QtDBus, building macmenu support")
    add_definitions(-DHASDBUS=1)
endif ()

qt5_wrap_cpp(MOC_SRCS ${MOC_HDRS})

add_library(${PROJECT_NAME} SHARED ${SRCS} ${MOC_SRCS} ${HDRS})

# xcb and x11
find_package(XCB COMPONENTS XCB)
if (XCB_FOUND)
    add_definitions(-DHASXCB=1)
    target_link_libraries(${PROJECT_NAME} XCB::XCB)
    include_directories(${XCB_INCLUDES})
else ()
    find_package(X11)
    if (X11_FOUND)
        add_definitions(-DHASX11=1)
        include_directories(${X11_INCLUDES})
        target_link_libraries(${PROJECT_NAME} ${X11_X11_LIB} ${X11_LIBRARIES})
    endif ()
endif ()

set(KF5_MIN_VERSION "5.9.0")
find_package(KF5 ${KF5_MIN_VERSION} COMPONENTS WindowSystem)
if (KF5_FOUND)
    include(KDEInstallDirs)
    include(KDECMakeSettings)
    include(KDECompilerSettings)
    target_link_libraries(${PROJECT_NAME} KF5::WindowSystem)
    add_definitions(-DHASKF5)
endif ()

add_subdirectory(atmolib)

# link
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus Qt5::X11Extras nsesettings atmolib)

# install style plugin to Qt plugins/styles
get_target_property(QMAKE Qt5::qmake IMPORTED_LOCATION)
execute_process(COMMAND ${QMAKE} -query QT_INSTALL_PLUGINS OUTPUT_VARIABLE PLUGINPATH OUTPUT_STRIP_TRAILING_WHITESPACE)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${PLUGINPATH}/styles)

# KWin decoration (KF5 only)
find_package(KDecoration2)
if (KDecoration2_FOUND)
    add_subdirectory(kwin)
endif ()

# Install default config to share for first-run provisioning
install(FILES ${CMAKE_SOURCE_DIR}/NSE.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/share/atmo)
