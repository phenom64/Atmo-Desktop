
#include "xhandler.h"
#include <X11/Xatom.h>
#include <QX11Info>

Q_DECL_EXPORT Atom XHandler::atom[XHandler::ValueCount] =
{
    XInternAtom(QX11Info::display(), "STYLEPROJECT_MAINWINDOW", False),
    XInternAtom(QX11Info::display(), "STYLEPROJECT_HEADCOLOR", False),
    XInternAtom(QX11Info::display(), "_KDE_NET_WM_SHADOW", False),
    XInternAtom(QX11Info::display(), "STYLEPROJECT_STORESHADOW", False)
};

Q_DECL_EXPORT int XHandler::_n = 0;
