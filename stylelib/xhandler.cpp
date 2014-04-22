
#include "xhandler.h"
#include <X11/Xatom.h>
#include <QX11Info>

Q_DECL_EXPORT Atom XHandler::atom[XHandler::ValueCount] =
{
    XInternAtom(QX11Info::display(), "STYLEPROJECT_MAINWINDOWDATA", False),
    XInternAtom(QX11Info::display(), "_KDE_NET_WM_SHADOW", False),
    XInternAtom(QX11Info::display(), "STYLEPROJECT_STORESHADOW", False),
    XInternAtom(QX11Info::display(), "STYLEPROJECT_DECODATA", False)
};

Q_DECL_EXPORT int XHandler::_n = 0;
