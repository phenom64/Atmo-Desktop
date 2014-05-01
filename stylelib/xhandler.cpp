#include "xhandler.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "fixx11h.h"
#include <QX11Info>

static Atom atom[XHandler::ValueCount] =
{
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_MAINWINDOWDATA", False),
    XInternAtom(QX11Info::display(), "_KDE_NET_WM_SHADOW", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_STORESHADOW", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_DECODATA", False)
};

void
XHandler::changeProperty(const WId w, const Value v, const Size size, const unsigned char *data, const unsigned int nitems)
{
    if (!data)
        return;
//    Atom a = ((v == KwinShadows||v == StoreShadow) ? XA_CARDINAL : XA_ATOM);
    XChangeProperty(QX11Info::display(), w, atom[v], XA_CARDINAL, size, PropModeReplace, data, nitems);
    XSync(QX11Info::display(), False);
}

unsigned char
*XHandler::fetchProperty(const WId w, const Value v, int &n)
{
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char *d = 0;
    unsigned char **data = &d;
//    Atom a = ((v == KwinShadows||v == StoreShadow) ? XA_CARDINAL : XA_ATOM);
    XGetWindowProperty(QX11Info::display(), w, atom[v], 0L, 0xffffffff, False, XA_CARDINAL, &type, &format, &nitems, &after, data);
    n = nitems;
    return d;
}

void
XHandler::deleteXProperty(const WId w, const Value v)
{
    XDeleteProperty(QX11Info::display(), w, atom[v]);
}
