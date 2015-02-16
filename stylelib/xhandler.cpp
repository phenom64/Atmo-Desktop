#include "xhandler.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "fixx11h.h"
#include <QX11Info>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
//#include <QAbstractEventDispatcher>
#include "../config/settings.h"

static Atom atom[XHandler::ValueCount] =
{
    XInternAtom(QX11Info::display(), "_NET_WM_ICON", False),
    XInternAtom(QX11Info::display(), "_KDE_NET_WM_SHADOW", False),
    XInternAtom(QX11Info::display(), "_KDE_NET_WM_BLUR_BEHIND_REGION", False),
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

XHandler XHandler::s_instance;

XHandler
*XHandler::instance()
{
    return &s_instance;
}

XHandler::XHandler(QObject *parent)
    : QObject(parent)
//      m_timer(new QTimer(this))
{
//    connect(m_timer, SIGNAL(timeout()), this, SLOT(clearX11PixmapsSlot()));
}

XHandler::~XHandler()
{

}

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
*XHandler::fetchProperty(const WId w, const Value v, int &n, unsigned long offset, unsigned long length)
{
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char *d = 0;
    unsigned char **data = &d;
//    Atom a = ((v == KwinShadows||v == StoreShadow) ? XA_CARDINAL : XA_ATOM);
    XGetWindowProperty(QX11Info::display(), w, atom[v], offset, length, False, XA_CARDINAL, &type, &format, &nitems, &after, data);
    n = nitems;
    return d;
}

void
XHandler::deleteXProperty(const WId w, const Value v)
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
    xev.xclient.data.l[2] = resize?_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:_NET_WM_MOVERESIZE_MOVE;
    xev.xclient.data.l[3] = Button1;
    xev.xclient.data.l[4] = 0;
    XUngrabPointer(QX11Info::display(), QX11Info::appTime()); //is this necessary? ...oh well, sizegrip does it...
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(QX11Info().screen()), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

bool
XHandler::compositingActive()
{
#if 1
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
XHandler::x11Pix(const QPixmap &pix, Qt::HANDLE &handle, const QWidget *win)
{
    if (pix.isNull())
    {
        handle = 0;
        return pix;
    }

    if (handle && pix.size() != QPixmap::fromX11Pixmap(handle).size())
    {
        if (win)
            deleteXProperty(win->winId(), DecoBgPix);
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
}

QPixmap
XHandler::emptyX11Pix(const QSize &sz, const WId w)
{
    const Pixmap p = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), sz.width(), sz.height(), 32);
    return QPixmap::fromX11Pixmap(p, QPixmap::ExplicitlyShared);
}

QPixmap
XHandler::x11Pix(const QPixmap &pix)
{
    const Pixmap x = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), pix.width(), pix.height(), 32);
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

void
XHandler::updateDeco(const WId w)
{
    XEvent xce;
    xce.xclient.type = ClientMessage;
    xce.xclient.message_type = xAtom(Repaint);
    xce.xclient.display = QX11Info::display();
    xce.xclient.window = w;
    xce.xclient.format = 32;
    xce.xclient.data.l[0] = 0;
    XUngrabPointer(QX11Info::display(), QX11Info::appTime());
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(QX11Info().screen()), False, SubstructureNotifyMask, &xce);
}
