#ifndef XHANDLER_H
#define XHANDLER_H

#include <qglobal.h>

static int _n = 0;
class QPixmap;
class QImage;
class QPoint;
class QWidget;
class Q_DECL_EXPORT XHandler
{
public:
    enum Value { _NET_WORKAREA = 0,
                 _NET_CURRENT_DESKTOP,
                 _NET_WM_ICON,
                 _KDE_NET_WM_SHADOW,
                 _KDE_NET_WM_BLUR_BEHIND_REGION,
                 _NET_FRAME_EXTENTS,
                 WindowData,
                 StoreActiveShadow,
                 StoreInActiveShadow,
                 DecoTitleHeight,
                 ContPix,
                 Repaint,
                 ValueCount };
    enum Size { Byte = 8, Short = 16, Long = 32 };
    enum Operation {
        _NET_WM_MOVERESIZE_SIZE_TOPLEFT      =0,
        _NET_WM_MOVERESIZE_SIZE_TOP          =1,
        _NET_WM_MOVERESIZE_SIZE_TOPRIGHT     =2,
        _NET_WM_MOVERESIZE_SIZE_RIGHT        =3,
        _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT  =4,
        _NET_WM_MOVERESIZE_SIZE_BOTTOM       =5,
        _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT   =6,
        _NET_WM_MOVERESIZE_SIZE_LEFT         =7,
        _NET_WM_MOVERESIZE_MOVE              =8,   /* movement only */
        _NET_WM_MOVERESIZE_SIZE_KEYBOARD     =9,   /* size via keyboard */
        _NET_WM_MOVERESIZE_MOVE_KEYBOARD     =10,   /* move via keyboard */
        _NET_WM_MOVERESIZE_CANCEL            =11   /* cancel operation */
    };
    typedef unsigned int TypeSize;
    typedef unsigned long XWindow, XPixmap;
    template<typename T> static void setXProperty(const XWindow w, const Value v, const TypeSize size, T *d, unsigned int n = 1)
    {
        //reminder to self, the realByteSize is dependent on the type submitten to this method, no magic involved
        const TypeSize byteSize(size/8), realByteSize(sizeof(T));
        if (realByteSize > byteSize)
            n *= realByteSize/byteSize;

        changeProperty(w, v, size, reinterpret_cast<unsigned char *>(d), n);
    }
    template<typename T> static T *getXProperty(const XWindow w, const Value v, int &n = _n, unsigned long offset = 0L, unsigned long length = 0xffffffff)
    {
        return reinterpret_cast<T *>(fetchProperty(w, v, n, offset, length));
    }
    static unsigned long xAtom(Value v);
    static void freeData(void *data);
    static void deleteXProperty(const XWindow w, const Value v);
    static void mwRes(const QPoint &globalPoint, const XWindow &win, bool resize = false);
    static void move(QWidget *w, const QPoint &pt);
    static bool compositingActive();
    static float opacity();
    static QImage fromX11Pix(unsigned long x11Pix, const QSize &sz);
    static XPixmap x11Pixmap(const QImage &qtImg);
    static void freePix(const XPixmap pixmap);
    static void updateDeco(const XWindow w);
    static void getDecoBorders(int &left, int &right, int &top, int &bottom, const XWindow id);

protected:
    static void changeProperty(const XWindow w, const Value v, const TypeSize size, const unsigned char *data, const unsigned int nitems);
    static unsigned char *fetchProperty(const XWindow w, const Value v, int &n, unsigned long offset, unsigned long length);
};

#endif //XHANDLER_H
