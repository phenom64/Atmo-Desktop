project(styleproject)
cmake_minimum_required(VERSION 2.6)

add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})
add_subdirectory(stylelib)

set(SRCS
    buttons.cpp
    events.cpp
    init.cpp
    inputs.cpp
    items.cpp
    macmenu.cpp
    overlay.cpp
    panels.cpp
    pixelmetric.cpp
    polish.cpp
    sliders.cpp
    styleproject.cpp
    tabs.cpp
    stylehint.cpp
    contentssize.cpp
    subrects.cpp)
    
set(HDRS
    macmenu-dbus.h
    macmenu.h
    overlay.h
    styleproject.h)
    
set(MOC_HDRS
    macmenu-dbus.h
    macmenu.h
    overlay.h
    styleproject.h)
    
set(QT_USE_QTDBUS TRUE)
#set (QT_USE_QTNETWORK TRUE)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
qt4_wrap_cpp(MOC_SRCS ${MOC_HDRS})

find_package(KDE4 REQUIRED)
find_package(X11 REQUIRED)

if (KDE4_FOUND)
    include(KDE4Defaults)
    include(MacroLibrary)
    add_definitions(${KDE4_DEFINITIONS})
    include_directories(${KDE4_INCLUDES})
endif ()

add_library(styleproject SHARED ${SRCS} ${HDRS} ${MOC_SRCS})
target_link_libraries(styleproject stylelib ${QT_LIBRARIES} ${QT_QTDBUS_LIBRARY} ${X11_LIBRARIES})

install(TARGETS styleproject LIBRARY DESTINATION ${QT_PLUGINS_DIR}/styles)

if (KDE4_FOUND)
    install(FILES styleproject.themerc DESTINATION ${DATA_INSTALL_DIR}/kstyle/themes)
    if (X11_FOUND)
        add_subdirectory(kwin)
    endif ()
endif ()
