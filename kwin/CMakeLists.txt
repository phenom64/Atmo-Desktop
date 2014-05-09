project (kwin3-styleproject)

set (KWIN_SRCS
    kwinclient.cpp
    factory.cpp
    button.cpp
    widgets.cpp)
    
set (KWIN_HDRS
    kwinclient.h
    factory.h
    button.h
    factorydbusadaptor.h
    widgets.h)
    
set (KWIN_MOC_HDRS
    kwinclient.h
    factory.h
    button.h
    factorydbusadaptor.h
    widgets.)

set (QT_USE_DBUS TRUE)

#message (${QT_LIBRARIES})
    
#kde4_add_plugin mocs, so no need to moc the moc headers,
#however, factorydbusadaptor has no source file, so needs
#separate handling
qt4_wrap_cpp (KWIN_MOC_DBUS factorydbusadaptor.h)

kde4_add_plugin (kwin3_styleproject ${KWIN_SRCS} ${KWIN_MOC_DBUS})
target_link_libraries (kwin3_styleproject stylelib ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} kdecorations ${X11_LIBRARIES} ${X11_Xrender_LIB})
install (TARGETS kwin3_styleproject DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES styleproject.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin)
