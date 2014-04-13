QT       += widgets dbus
CONFIG   += qt dbus
TARGET   = styleproject
TEMPLATE = lib

SOURCES += styleproject.cpp \
    init.cpp \
    buttons.cpp \
    pixelmetric.cpp \
    polish.cpp \
    render.cpp \
    inputs.cpp \
    ops.cpp \
    panels.cpp \
    overlay.cpp \
    macmenu.cpp \
    items.cpp \
    sliders.cpp \
    tabs.cpp \
    events.cpp

HEADERS += styleproject.h \
    render.h \
    ops.h \
    overlay.h \
    macmenu.h \
    macmenu-dbus.h

unix {
    target.path = $$[QT_INSTALL_PLUGINS]/styles
    #theme.path = $$[QT_INSTALL_DATA]/kstyle/themes
    theme.path = /usr/share/apps/kstyle/themes/
    theme.files = styleproject.themerc
    INSTALLS += target theme
}
