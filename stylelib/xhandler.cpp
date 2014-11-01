#include "xhandler.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "fixx11h.h"
#include <QX11Info>
#include <QMouseEvent>
#include <QPainter>
#include "settings.h"

static Atom atom[XHandler::ValueCount] =
{
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_MAINWINDOWDATA", False),
    XInternAtom(QX11Info::display(), "_KDE_NET_WM_SHADOW", False),
    XInternAtom(QX11Info::display(), "_KDE_NET_WM_BLUR_BEHIND_REGION", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_STOREACTIVESHADOW", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_STOREINACTIVESHADOW", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_DECODATA", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_DECOBGPIX", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_CONTPIX", False)
};

void
XHandler::changeProperty(const WId w, const Value v, const TypeSize size, const unsigned char *data, const unsigned int nitems)
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

void
XHandler::mwRes(const QPoint &globalPoint, const WId &win, bool resize)
{
    static Atom netWmMoveResize = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
    //this is stole.... errrhmm copied from qsizegrip.cpp
    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = netWmMoveResize;
    xev.xclient.display = QX11Info::display();
    xev.xclient.window = win;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = globalPoint.x();
    xev.xclient.data.l[1] = globalPoint.y();
//#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT      0
//#define _NET_WM_MOVERESIZE_SIZE_TOP          1
//#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     2
//#define _NET_WM_MOVERESIZE_SIZE_RIGHT        3
//#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  4
//#define _NET_WM_MOVERESIZE_SIZE_BOTTOM       5
//#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   6
//#define _NET_WM_MOVERESIZE_SIZE_LEFT         7
//#define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
//#define _NET_WM_MOVERESIZE_SIZE_KEYBOARD     9   /* size via keyboard */
//#define _NET_WM_MOVERESIZE_MOVE_KEYBOARD    10   /* move via keyboard */
//#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */
    xev.xclient.data.l[2] = resize?4:8;
    xev.xclient.data.l[3] = Button1;
    xev.xclient.data.l[4] = 0;
    XUngrabPointer(QX11Info::display(), QX11Info::appTime()); //is this necessary? ...oh well, sizegrip does it...
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(QX11Info().screen()), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

bool
XHandler::compositingActive()
{
    return QX11Info::isCompositingManagerRunning();
//    static Atom *net_wm_cm = 0;
//    if (!net_wm_cm)
//    {
//        char net_wm_cm_name[ 100 ];
//        sprintf(net_wm_cm_name, "_NET_WM_CM_S%d", DefaultScreen(QX11Info::display()));
//        net_wm_cm = new Atom();
//        *net_wm_cm = XInternAtom(QX11Info::display(), net_wm_cm_name, False);
//    }
//    return XGetSelectionOwner(QX11Info::display(), *net_wm_cm) != None;
}

float
XHandler::opacity()
{
    if (compositingActive())
        return Settings::conf.opacity;
    else
        return 1.0f;
}

QPixmap
XHandler::x11Pix(const QPixmap &pix, const Qt::HANDLE handle)
{
    const Pixmap x = handle?handle:XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), pix.width(), pix.height(), 32);
    QPixmap p = QPixmap::fromX11Pixmap(x, QPixmap::ExplicitlyShared);
    QPainter pt(&p);
    pt.setCompositionMode(QPainter::CompositionMode_Source);
    pt.drawPixmap(p.rect(), pix);
    pt.end();
    return p;
}

void
XHandler::freePix(QPixmap pix)
{
    freePix(pix.handle());
}

void
XHandler::freePix(const Qt::HANDLE handle)
{
    XFreePixmap(QX11Info::display(), handle);
}
