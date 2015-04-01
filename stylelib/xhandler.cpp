#include "xhandler.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "fixx11h.h"

#include <QX11Info>
#include <QPainter>
#include <stdio.h>
//#include <QAbstractEventDispatcher>
#include "../config/settings.h"

static Atom atom[XHandler::ValueCount] =
{
    XInternAtom(QX11Info::display(), "_NET_WORKAREA", False),
    XInternAtom(QX11Info::display(), "_NET_CURRENT_DESKTOP", False),
    XInternAtom(QX11Info::display(), "_NET_WM_ICON", False),
    XInternAtom(QX11Info::display(), "_KDE_NET_WM_SHADOW", False),
    XInternAtom(QX11Info::display(), "_KDE_NET_WM_BLUR_BEHIND_REGION", False),
    XInternAtom(QX11Info::display(), "_NET_FRAME_EXTENTS", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_MAINWINDOWDATA", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_STOREACTIVESHADOW", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_STOREINACTIVESHADOW", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_DECODATA", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_DECOBGPIX", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_CONTPIX", False),
    XInternAtom(QX11Info::display(), "_STYLEPROJECT_REPAINT", False)
};

unsigned long
XHandler::xAtom(Value v)
{
    return atom[v];
}

void
XHandler::changeProperty(const XWindow w, const Value v, const TypeSize size, const unsigned char *data, const unsigned int nitems)
{
    if (!data)
        return;
//    Atom a = ((v == KwinShadows||v == StoreShadow) ? XA_CARDINAL : XA_ATOM);
    XChangeProperty(QX11Info::display(), w, atom[v], XA_CARDINAL, size, PropModeReplace, data, nitems);
    XSync(QX11Info::display(), False);
}

unsigned char
*XHandler::fetchProperty(const XWindow w, const Value v, int &n, unsigned long offset, unsigned long length)
{
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char *d(0);
    XGetWindowProperty(QX11Info::display(), w, atom[v], offset, length, False, XA_CARDINAL, &type, &format, &nitems, &after, &d);
    n = nitems;
    return d;
}

void
XHandler::deleteXProperty(const XWindow w, const Value v)
{
    XDeleteProperty(QX11Info::display(), w, atom[v]);
    XSync(QX11Info::display(), False);
}

void
XHandler::freeData(void *data)
{
    XFree(data);
}

void
XHandler::mwRes(const QPoint &globalPoint, const XWindow &win, bool resize)
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
    xev.xclient.data.l[2] = resize?_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:_NET_WM_MOVERESIZE_MOVE;
    xev.xclient.data.l[3] = Button1;
    xev.xclient.data.l[4] = 0;
    XUngrabPointer(QX11Info::display(), QX11Info::appTime()); //is this necessary? ...oh well, sizegrip does it...
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

bool
XHandler::compositingActive()
{
#if QT_VERSION < 0x050000
    return QX11Info::isCompositingManagerRunning();
#else
    static Atom *net_wm_cm = 0;
    if (!net_wm_cm)
    {
        char net_wm_cm_name[ 100 ];
        sprintf(net_wm_cm_name, "_NET_WM_CM_S%d", DefaultScreen(QX11Info::display()));
        net_wm_cm = new Atom();
        *net_wm_cm = XInternAtom(QX11Info::display(), net_wm_cm_name, False);
    }
    return XGetSelectionOwner(QX11Info::display(), *net_wm_cm) != None;
#endif
}

float
XHandler::opacity()
{
//    if (compositingActive())
        return dConf.opacity;
//    else
//        return 1.0f;
}

QPixmap
XHandler::x11Pix(const QPixmap &pix, XWindow &handle, const XWindow winId)
{
#if QT_VERSION < 0x050000
    if (pix.isNull())
    {
        handle = 0;
        return pix;
    }

    if (handle && pix.size() != QPixmap::fromX11Pixmap(handle).size())
    {
        if (winId)
            deleteXProperty(winId, DecoBgPix);
        freePix(handle);
        handle = 0;
    }

    const Pixmap x = handle?handle:XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), pix.width(), pix.height(), 32);
    QPixmap p = QPixmap::fromX11Pixmap(x, QPixmap::ExplicitlyShared);
    QPainter pt(&p);
    pt.setCompositionMode(QPainter::CompositionMode_Source);
    pt.drawPixmap(p.rect(), pix);
    pt.end();
    handle = x;
    return p;
#else
    return QPixmap();
#endif
}

QPixmap
XHandler::x11Pix(const QPixmap &pix)
{
#if QT_VERSION < 0x050000
    const Pixmap x = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), pix.width(), pix.height(), 32);
    QPixmap p = QPixmap::fromX11Pixmap(x, QPixmap::ExplicitlyShared);
    QPainter pt(&p);
    pt.setCompositionMode(QPainter::CompositionMode_Source);
    pt.drawPixmap(p.rect(), pix);
    pt.end();
    return p;
#else
    return QPixmap();
#endif
}

void
XHandler::freePix(QPixmap pix)
{
#if QT_VERSION < 0x050000
    freePix(pix.handle());
#endif
}

void
XHandler::freePix(const XWindow handle)
{
#if QT_VERSION < 0x050000
    XFreePixmap(QX11Info::display(), handle);
#endif
}

void
XHandler::updateDeco(const XWindow w)
{
    XEvent xce;
    xce.xclient.type = ClientMessage;
    xce.xclient.message_type = xAtom(Repaint);
    xce.xclient.display = QX11Info::display();
    xce.xclient.window = w;
    xce.xclient.format = 32;
    xce.xclient.data.l[0] = 0;
    XUngrabPointer(QX11Info::display(), QX11Info::appTime());
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureNotifyMask, &xce);
}

QPoint
XHandler::strutTopLeft()
{
    int n(0);
    unsigned int *desktop = getXProperty<unsigned int>(QX11Info::appRootWindow(), _NET_CURRENT_DESKTOP);
    if (!desktop)
        return QPoint();
    const unsigned char d(*desktop);
    freeData(desktop);
    unsigned long *struts = getXProperty<unsigned long>(QX11Info::appRootWindow(), _NET_WORKAREA, n, d*4);
    if (!struts)
        return QPoint();
    freeData(struts);
    if (n < 2)
        return QPoint();
    return QPoint(struts[0], struts[1]);
}

void
XHandler::getDecoBorders(int &left, int &right, int &top, int &bottom, const XWindow id)
{
    int n;
    unsigned long *data = getXProperty<unsigned long>(id, _NET_FRAME_EXTENTS, n);
    if (n != 4)
        left = right = top = bottom = 0;
    else
    {
        left = data[0];
        right = data[1];
        top = data[2];
        bottom = data[3];
    }
    if (data)
        freeData(data);
}
