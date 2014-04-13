project (kwin3-styleproject)

set (KWIN_SRCS
    kwinclient.cpp
    factory.cpp)
    
set (KWIN_HDRS
    kwinclient.h
    factory.h)
    
set (KWIN_MOC_HDRS
    kwinclient.h
    factory.h)
    
#qt4_wrap_cpp (KWIN_MOC_SRCS ${KWIN_MOC_HDRS})

kde4_add_plugin (kwin3_styleproject ${KWIN_SRCS})
target_link_libraries (kwin3_styleproject ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} kdecorations ${X11_Xrender_LIB})
install (TARGETS kwin3_styleproject DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES styleproject.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin)
