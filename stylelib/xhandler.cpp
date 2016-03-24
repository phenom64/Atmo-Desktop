#include <qmetatype.h>
#include <QDataStream>
#include <QDebug>
#include <QWidget>
#include <QX11Info>
#include <QPainter>
#include "xhandler.h"
#include "../config/settings.h"

#if HASXCB //we only have xcb when compiled against qt5.
#include <xcb/xproto.h>
//#include <xcb/xcb_image.h>
#include <xcb/xcb.h>
#elif HASX11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
//#include "fixx11h.h"
#endif

using namespace DSP;

static char *atoms[XHandler::ValueCount] =
{
    "_NET_WORKAREA",
    "_NET_CURRENT_DESKTOP",
    "_NET_WM_ICON",
    "_KDE_NET_WM_SHADOW",
    "_KDE_NET_WM_BLUR_BEHIND_REGION",
    "_NET_FRAME_EXTENTS",
    "_STYLEPROJECT_STOREACTIVESHADOW",
    "_STYLEPROJECT_STOREINACTIVESHADOW"
};

#if HASXCB
static xcb_atom_t xcb_atom[XHandler::ValueCount];
#elif HASX11
static Atom atom[XHandler::ValueCount];
#endif



void
XHandler::init()
{
    static bool isInited(false);
    if (isInited)
        return;

    for (int i = 0; i < XHandler::ValueCount; ++i)
    {
#if HASXCB
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), false, strlen(atoms[i]), atoms[i]);
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(QX11Info::connection(), cookie, 0);
        xcb_atom[i] = reply ? reply->atom : 0;
#elif HASX11
        atom[i] = XInternAtom(QX11Info::display(), atoms[i], False);
#endif
    }
    isInited = true;
}

void
XHandler::changeProperty(const XWindow w, const Value v, const TypeSize size, const unsigned char *data, const unsigned int nitems)
{
    if (!data)
        return;

#if HASXCB
    if (xcb_atom[v])
    {
        xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, w, xcb_atom[v], XCB_ATOM_CARDINAL, size, nitems, data);
        xcb_flush(QX11Info::connection());
    }
#elif HASX11
    XChangeProperty(QX11Info::display(), w, atom[v], XA_CARDINAL, size, PropModeReplace, data, nitems);
    XSync(QX11Info::display(), False);
#endif
}

unsigned char
*XHandler::fetchProperty(const XWindow w, const Value v, int &n, quint32 offset, quint32 length)
{
#if HASXCB
    if (xcb_atom[v])
    {
        xcb_get_property_cookie_t cookie = xcb_get_property(QX11Info::connection(), false, w, xcb_atom[v], XCB_ATOM_CARDINAL, offset, length);
        xcb_get_property_reply_t *reply =  xcb_get_property_reply(QX11Info::connection(), cookie, NULL);
        if (reply)
        {
            n = xcb_get_property_value_length(reply);
            if (!n)
            {
                freeData(reply);
                return 0;
            }
            n = reply->value_len;
            //meh... cant free only the data from outside...
            //so we copy it to another container and return that :((((((((((((((
            //fix
            const int bytes = n*sizeof(quint32);
            quint32 *data = (quint32*)malloc(bytes);
            memcpy(data, xcb_get_property_value(reply), bytes);
            freeData(reply);
            return (uchar*)data;
        }
    }
#elif HASX11
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char *d(0);
    XGetWindowProperty(QX11Info::display(), w, atom[v], offset, length, False, XA_CARDINAL, &type, &format, &nitems, &after, &d);
    n = nitems;
    return d;
#endif
    return 0;
}

void
XHandler::deleteXProperty(const XWindow w, const Value v)
{
#if HASXCB
    if (xcb_atom[v])
    {
        xcb_delete_property(QX11Info::connection(), w, xcb_atom[v]);
        xcb_flush(QX11Info::connection());
    }
#elif HASX11
    XDeleteProperty(QX11Info::display(), w, atom[v]);
    XSync(QX11Info::display(), False);
#endif
}

void
XHandler::freeData(void *data)
{
#if HASXCB
    free(data);
#elif HASX11
    XFree(data);
#endif
}

#if HASXCB
static void xcbIndexMask(const Qt::MouseButton b, xcb_button_index_t &i, xcb_button_mask_t &m)
{
    //this is rather confusing, xcb says *_2 shoudl be right click, but *_3 is rightclick....
    switch (b)
    {
    case 1: i = XCB_BUTTON_INDEX_1; m = XCB_BUTTON_MASK_1; break;
    case 2: i = XCB_BUTTON_INDEX_3; m = XCB_BUTTON_MASK_3; break;
    case 4: i = XCB_BUTTON_INDEX_2; m = XCB_BUTTON_MASK_2; break;
    default: break;
    }
}

#endif

void
XHandler::doubleClickEvent(const QPoint &globalPos, const XWindow win, const Qt::MouseButton button)
{
    pressEvent(globalPos, win, button);
    releaseEvent(globalPos, win, button);
    pressEvent(globalPos, win, button);
    releaseEvent(globalPos, win, button);
}

void
XHandler::releaseEvent(const QPoint &globalPos, const XWindow win, const Qt::MouseButton button)
{
#if HASXCB
    xcb_button_mask_t mask;
    xcb_button_index_t index;
    xcbIndexMask(button, index, mask);
    xcb_connection_t *connection = QX11Info::connection();
    xcb_button_release_event_t releaseEvent;
    memset(&releaseEvent, 0, sizeof(releaseEvent));
    releaseEvent.response_type = XCB_BUTTON_RELEASE;
    releaseEvent.event =  win;
    releaseEvent.child = XCB_WINDOW_NONE;
    releaseEvent.root = QX11Info::appRootWindow();
    releaseEvent.event_x = 0;
    releaseEvent.event_y = 0;
    releaseEvent.root_x = globalPos.x();
    releaseEvent.root_y = globalPos.y();
    releaseEvent.detail = index;
    releaseEvent.state = mask;
    releaseEvent.time = QX11Info::appTime();
    releaseEvent.same_screen = true;
    xcb_send_event(connection, false, win, XCB_EVENT_MASK_BUTTON_RELEASE, reinterpret_cast<const char*>(&releaseEvent));
    xcb_ungrab_pointer(connection, XCB_TIME_CURRENT_TIME);
    xcb_flush(connection);
#endif
}

void
XHandler::pressEvent(const QPoint &globalPos, const XWindow win, const Qt::MouseButton button)
{
#if HASXCB
    xcb_button_mask_t mask;
    xcb_button_index_t index;
    xcbIndexMask(button, index, mask);
    xcb_connection_t *connection = QX11Info::connection();
    xcb_button_press_event_t pressEvent;
    memset(&pressEvent, 0, sizeof(pressEvent));
    pressEvent.response_type = XCB_BUTTON_PRESS;
    pressEvent.event =  win;
    pressEvent.child = XCB_WINDOW_NONE;
    pressEvent.root = QX11Info::appRootWindow();
    pressEvent.event_x = 0;
    pressEvent.event_y = 0;
    pressEvent.root_x = globalPos.x();
    pressEvent.root_y = globalPos.y();
    pressEvent.detail = index;
    pressEvent.state = mask;
    pressEvent.time = XCB_CURRENT_TIME /*QX11Info::appTime()*/;
    pressEvent.same_screen = true;
    xcb_send_event(connection, false, win, XCB_EVENT_MASK_BUTTON_PRESS, reinterpret_cast<const char*>(&pressEvent));
    xcb_ungrab_pointer(connection, XCB_TIME_CURRENT_TIME);
    xcb_flush(connection);
#endif
}

void
XHandler::wheelEvent(const XHandler::XWindow win, const bool up)
{
#if HASXCB
    xcb_connection_t *connection = QX11Info::connection();
    xcb_button_press_event_t wheelEvent;
    memset(&wheelEvent, 0, sizeof(wheelEvent));
    wheelEvent.response_type = XCB_BUTTON_PRESS;
    wheelEvent.event =  win;
    wheelEvent.child = XCB_WINDOW_NONE;
    wheelEvent.root = QX11Info::appRootWindow();
    wheelEvent.detail = up?XCB_BUTTON_INDEX_4:XCB_BUTTON_INDEX_5;
    wheelEvent.state = up?XCB_BUTTON_MASK_4:XCB_BUTTON_MASK_5;
    wheelEvent.time = XCB_CURRENT_TIME;
    wheelEvent.same_screen = true;
    xcb_send_event(connection, false, win, XCB_EVENT_MASK_BUTTON_PRESS, reinterpret_cast<const char*>(&wheelEvent));
    xcb_flush(connection);
#endif
}

void
XHandler::mwRes(const QPoint &localPos, const QPoint &globalPos, const XWindow win, bool resize, XWindow dest)
{
#if HASXCB
    if (!QX11Info::isPlatformX11())
        return;

    if (!dest)
        dest = win;
    xcb_connection_t *connection = QX11Info::connection();

    static xcb_atom_t atom = 0;
    if (!atom)
    {
        const QString atomName("_NET_WM_MOVERESIZE");
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, false, atomName.size(), qPrintable(atomName));
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, 0);
        atom = reply ? reply->atom : 0;
    }
    if (!atom)
        return;

    // button release event
    xcb_button_release_event_t releaseEvent;
    memset(&releaseEvent, 0, sizeof(releaseEvent));
    releaseEvent.response_type = XCB_BUTTON_RELEASE;
    releaseEvent.event =  win;
    releaseEvent.child = XCB_WINDOW_NONE;
    releaseEvent.root = QX11Info::appRootWindow();
    releaseEvent.event_x = localPos.x();
    releaseEvent.event_y = localPos.y();
    releaseEvent.root_x = globalPos.x();
    releaseEvent.root_y = globalPos.y();
    releaseEvent.detail = XCB_BUTTON_INDEX_1;
    releaseEvent.state = XCB_BUTTON_MASK_1;
    releaseEvent.time = XCB_CURRENT_TIME;
    releaseEvent.same_screen = true;
    xcb_send_event(connection, false, win, XCB_EVENT_MASK_BUTTON_RELEASE, reinterpret_cast<const char*>(&releaseEvent));
    xcb_ungrab_pointer(connection, XCB_TIME_CURRENT_TIME);

    // move resize event
    xcb_client_message_event_t clientMessageEvent;
    memset(&clientMessageEvent, 0, sizeof(clientMessageEvent));

    clientMessageEvent.response_type = XCB_CLIENT_MESSAGE;
    clientMessageEvent.type = atom;
    clientMessageEvent.format = 32;
    clientMessageEvent.window = dest;
    clientMessageEvent.data.data32[0] = globalPos.x();
    clientMessageEvent.data.data32[1] = globalPos.y();
    clientMessageEvent.data.data32[2] = resize?_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:_NET_WM_MOVERESIZE_MOVE;
    clientMessageEvent.data.data32[3] = Qt::LeftButton;
    clientMessageEvent.data.data32[4] = 0;

    xcb_send_event(connection, false, QX11Info::appRootWindow(),
                   XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                   reinterpret_cast<const char*>(&clientMessageEvent));

    xcb_flush(connection);
#elif HASX11
    XEvent xrev;
    xrev.xbutton.serial = 0; //?????
    xrev.xbutton.button = Button1;
    xrev.xbutton.display = QX11Info::display();
    xrev.xbutton.root = QX11Info::appRootWindow();
    xrev.xbutton.same_screen = True;
    xrev.xbutton.send_event = False;
    xrev.xbutton.state = Button1Mask;
    xrev.xbutton.window = win;
    xrev.xbutton.time = QX11Info::appTime();
    xrev.xbutton.type = ButtonRelease;
    xrev.xbutton.x_root = globalPos.x();
    xrev.xbutton.y_root = globalPos.y();
    xrev.xbutton.x = localPos.x();
    xrev.xbutton.y = localPos.y();
    XSendEvent(QX11Info::display(), win, False, Button1Mask|ButtonReleaseMask, &xrev);

    static Atom netWmMoveResize = XInternAtom(QX11Info::display(), "_NET_WM_MOVERESIZE", False);
    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = netWmMoveResize;
    xev.xclient.display = QX11Info::display();
    xev.xclient.window = win;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = globalPos.x();
    xev.xclient.data.l[1] = globalPos.y();
    xev.xclient.data.l[2] = resize?_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT:_NET_WM_MOVERESIZE_MOVE;
    xev.xclient.data.l[3] = Button1;
    xev.xclient.data.l[4] = 0;
    XUngrabPointer(QX11Info::display(), QX11Info::appTime()); //is this necessary? ...oh well, sizegrip does it...
    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    XSync(QX11Info::display(), False);
#endif
}

bool
XHandler::compositingActive()
{
#if QT_VERSION < 0x050000
    return QX11Info::isCompositingManagerRunning();
#elif defined(HASXCB)
    static xcb_atom_t atom = 0;
    xcb_connection_t *c = QX11Info::connection();
    if (!atom)
    {
        const QString &net_wm_cm_name = QString("_NET_WM_CM_S%1").arg(QString::number(QX11Info::appScreen()));
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(c, false, net_wm_cm_name.size(), qPrintable(net_wm_cm_name));
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(c, cookie, 0);
        atom = reply ? reply->atom : 0;
    }
    if (!atom)
        return false;

    xcb_get_selection_owner_cookie_t cookie = xcb_get_selection_owner(c, atom);
    xcb_get_selection_owner_reply_t *reply = xcb_get_selection_owner_reply(c, cookie, NULL);
    return reply && reply->owner;
#elif HASX11
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

XHandler::XPixmap
XHandler::x11Pixmap(const QImage &qtImg)
{
    if (qtImg.isNull())
        return 0;
#if HASXCB
    xcb_connection_t *c = QX11Info::connection();
    //    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
    xcb_pixmap_t pixmapId = xcb_generate_id(c);
    xcb_create_pixmap(c, qtImg.depth(), pixmapId, QX11Info::appRootWindow(), qtImg.width(), qtImg.height());
    xcb_gcontext_t gcId = xcb_generate_id(c);
    xcb_create_gc(c, gcId, pixmapId, 0, 0);
    xcb_put_image(c, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmapId, gcId, qtImg.width(), qtImg.height(), 0, 0, 0, qtImg.depth(), qtImg.byteCount(), qtImg.bits());
    xcb_free_gc(c, gcId);
    return pixmapId;
#elif HASX11
    //Stolen from virtuality
    XImage ximage;
    memset(&ximage, 0, sizeof(XImage));
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
    return pix;
#endif
    return 0;
}

void
XHandler::freePix(const XPixmap pixmap)
{
#if HASXCB
    xcb_free_pixmap(QX11Info::connection(), pixmap);
#elif HASX11
    XFreePixmap(QX11Info::display(), pixmap);
#endif
}

void
XHandler::reparent(const XWindow child, const XWindow parent)
{
#if HASXCB
    xcb_connection_t *c = QX11Info::connection();
    xcb_reparent_window(c, child, parent, 0, 0);
//    static const quint32 value[] = {XCB_STACK_MODE_ABOVE};
//    xcb_configure_window(c, child, XCB_CONFIG_WINDOW_STACK_MODE, value);
//    xcb_map_window(c, child);
#endif
}

void
XHandler::restack(const XWindow win, const XWindow parent)
{
#if HASXCB
    xcb_connection_t *c = QX11Info::connection();
    xcb_window_t current = parent;
    xcb_query_tree_cookie_t cookie = xcb_query_tree_unchecked(c, current);
    xcb_query_tree_reply_t *tree = xcb_query_tree_reply(c, cookie, 0);
    if (tree && tree->parent)
    {
        current = tree->parent;
//        xcb_query_tree_cookie_t cookie2 = xcb_query_tree_unchecked(c, current);
//        xcb_query_tree_reply_t *tree2 = xcb_query_tree_reply(c, cookie2, 0);
//        if (tree2 && tree2->parent != tree2->root)
//            current = tree2->parent;
    }
    // reparent
    xcb_reparent_window(c, win, current, 0, 0);
    static const quint32 value[] = {XCB_STACK_MODE_ABOVE};
    xcb_configure_window(c, win, XCB_CONFIG_WINDOW_STACK_MODE, value);
    xcb_map_window(c, win);
#endif
}

void
XHandler::move(const XWindow win, const QPoint &p)
{
#if HASXCB
    unsigned int values[2] = {p.x(), p.y()};
    xcb_configure_window(QX11Info::connection(), win, XCB_CONFIG_WINDOW_X|XCB_CONFIG_WINDOW_Y, values);
#endif
}

void
XHandler::getDecoBorders(int &left, int &right, int &top, int &bottom, const XWindow id)
{
    int n(0);
    dlong *data = getXProperty<dlong>(id, _NET_FRAME_EXTENTS, n);
    if (n == 4)
    {
        left = data[0];
        right = data[1];
        top = data[2];
        bottom = data[3];
    }
    else
        left = right = top = bottom = 0;
    if (data)
        freeData(data);
}
