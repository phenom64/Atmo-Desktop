QT       += widgets

TARGET = styleproject
TEMPLATE = lib

SOURCES += styleproject.cpp \
    init.cpp \
    buttons.cpp \
    pixelmetric.cpp \
    polish.cpp \
    render.cpp \
    inputs.cpp

HEADERS += styleproject.h \
    render.h

unix {
    target.path = $$[QT_INSTALL_PLUGINS]/styles
    #theme.path = $$[QT_INSTALL_DATA]/kstyle/themes
    theme.path = /usr/share/apps/kstyle/themes/
    theme.files = styleproject.themerc
    INSTALLS += target theme
}
