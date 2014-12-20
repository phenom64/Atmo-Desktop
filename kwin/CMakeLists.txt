message(STATUS "preparing kwin deco")
project(kwin3-styleproject)

find_file(KDECORATION_H kdecoration.h ${KDE4_INCLUDES})
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
    factorydbusadaptor.h
    sizegrip.h)
    
set(KWIN_MOC_HDRS
    kwinclient.h
    factory.h
    factorydbusadaptor.h
    sizegrip.h)

set(QT_USE_DBUS TRUE)
    
#kde4_add_plugin mocs, so no need to moc the moc headers,
#however, factorydbusadaptor has no source file, so needs
#separate handling
qt4_wrap_cpp(KWIN_MOC_DBUS factorydbusadaptor.h)
kde4_add_plugin(kwin3_styleproject ${KWIN_SRCS} ${KWIN_MOC_DBUS})
target_link_libraries(kwin3_styleproject stylelib styleproject_config_lib ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} kdecorations ${X11_LIBRARIES})
install(TARGETS kwin3_styleproject DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES styleproject.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin)
