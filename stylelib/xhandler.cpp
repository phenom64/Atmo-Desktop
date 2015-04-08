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
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
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

unsigned long
XHandler::x11Pixmap(const QImage &qtImg)
{
// Stolen from virtuality
    XImage ximage;
    ximage.width            = qtImg.width();
    ximage.height           = qtImg.height();
    ximage.xoffset          = 0;
    ximage.format           = ZPixmap;
    // This is a hack to prevent the image data from detaching
    ximage.data             = (char*) const_cast<const QImage*>(&qtImg)->bits();
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    ximage.byte_order       = MSBFirst;
#else
    ximage.byte_order       = LSBFirst;
#endif
    ximage.bitmap_unit      = 32;
    ximage.bitmap_bit_order = ximage.byte_order;
    ximage.bitmap_pad       = 32;
    ximage.depth            = 32;
    ximage.bytes_per_line   = qtImg.bytesPerLine();
    ximage.bits_per_pixel   = 32;
    ximage.red_mask         = 0x00ff0000;
    ximage.green_mask       = 0x0000ff00;
    ximage.blue_mask        = 0x000000ff;
    ximage.obdata           = 0;
    XInitImage(&ximage);
    Pixmap pix = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), qtImg.width(), qtImg.height(), 32);
    GC gc = XCreateGC(QX11Info::display(), pix, 0, 0);
    XPutImage(QX11Info::display(), pix, gc, &ximage, 0, 0, 0, 0, qtImg.width(), qtImg.height());
    XFreeGC(QX11Info::display(), gc);
    XFlush(QX11Info::display());
    return pix;
}

QImage
XHandler::fromX11Pix(unsigned long x11Pix, const QSize &sz)
{
    if (XImage *xi = XGetImage(QX11Info::display(), x11Pix, 0, 0, sz.width(), sz.height(), AllPlanes, ZPixmap))
    {
        QImage img(sz.width(), sz.height(), QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        unsigned int size(sz.width() * sz.height());
        QRgb *rgb = reinterpret_cast<QRgb *>(xi->data);
        QRgb *newRgb = reinterpret_cast<QRgb *>(img.bits());
        for (int i = 0; i < size; ++i)
            newRgb[i] = rgb[i];
        freeData(xi->data);
        freeData(xi);
        return img;
    }
    return QImage();
}

void
XHandler::freePix(const XPixmap pixmap)
{
    XFreePixmap(QX11Info::display(), pixmap);
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
