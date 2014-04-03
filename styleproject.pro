#-------------------------------------------------
#
# Project created by QtCreator 2014-04-03T21:24:49
#
#-------------------------------------------------

QT       += widgets

TARGET = styleproject
TEMPLATE = lib

SOURCES += styleproject.cpp

HEADERS += styleproject.h

unix {
    target.path = $$[QT_INSTALL_PLUGINS]/styles
    #theme.path = $$[QT_INSTALL_DATA]/kstyle/themes
    theme.path = /usr/share/apps/kstyle/themes/
    theme.files = styleproject.themerc
    INSTALLS += target theme
}
