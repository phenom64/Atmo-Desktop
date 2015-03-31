message(STATUS "preparing kwin deco")
project(kwin3-styleproject)

if (QT5BUILD)
    set(DECOHEADER kdecoration2.h)
else ()
    set(DECOHEADER kdecoration.h)
endif ()

find_file(KDECORATION_H ${DECOHEADER} ${KDE4_INCLUDES})
if (KDECORATION_H)
    message(STATUS "kwin deco header found: " ${KDECORATION_H})
else ()
    message(WARNING "kwin deco header not found, build will likely fail")
endif ()

set(KWIN_SRCS
    kwinclient.cpp
    factory.cpp
    sizegrip.cpp)
    
set(KWIN_HDRS
    kwinclient.h
    factory.h
    sizegrip.h)
    
set(KWIN_MOC_HDRS
    kwinclient.h
    factory.h
    sizegrip.h)

if (QT5BUILD)
    message(STATUS "not yet kde5 deco")
else ()
    kde4_add_plugin(kwin3_styleproject ${KWIN_SRCS} ${KWIN_MOC_DBUS})
    target_link_libraries(kwin3_styleproject stylelib styleproject_config_lib ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} kdecorations ${X11_LIBRARIES})
    install(TARGETS kwin3_styleproject DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES styleproject.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin)
endif ()
