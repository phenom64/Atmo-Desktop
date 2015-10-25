#ifndef XHANDLER_H
#define XHANDLER_H

#include <qglobal.h>
#include <qwindowdefs.h>
#include "../defines.h"

static int _n = 0;
class QPixmap;
class QImage;
class QPoint;
class QWidget;
namespace DSP
{
class Q_DECL_EXPORT XHandler
{
public:
    enum Value { _NET_WORKAREA = 0,
                 _NET_CURRENT_DESKTOP,
                 _NET_WM_ICON,
                 _KDE_NET_WM_SHADOW,
                 _KDE_NET_WM_BLUR_BEHIND_REGION,
                 _NET_FRAME_EXTENTS,
                 StoreActiveShadow,
                 StoreInActiveShadow,
                 ValueCount
               };
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

#if HASXCB
    typedef quint32 XPixmap;
#elif HASX11
    typedef unsigned long XPixmap;
#else
    typedef void XPixmap;
#endif
    typedef WId XWindow;

    template<typename T> static inline void setXProperty(const XWindow w, const Value v, const TypeSize size, T *d, unsigned int n = 1)
    {
        const TypeSize byteSize(size/8), realByteSize(sizeof(T));
        if (realByteSize > byteSize)
            n *= realByteSize/byteSize;

        changeProperty(w, v, size, reinterpret_cast<unsigned char *>(d), n);
    }
    template<typename T> static inline T *getXProperty(const XWindow w, const Value v, int &n = _n, quint32 offset = 0, quint32 length = 0xffffffff)
    {
        return reinterpret_cast<T *>(fetchProperty(w, v, n, offset, length));
    }
    static void freeData(void *data);
    static void deleteXProperty(const XWindow w, const Value v);
    static void mwRes(const QPoint &localPos, const QPoint &globalPos, const XWindow win, bool resize = false, XWindow dest = 0);
    static bool compositingActive();
    static float opacity();
    static XPixmap x11Pixmap(const QImage &qtImg);
    static void freePix(const XPixmap pixmap);
    static void getDecoBorders(int &left, int &right, int &top, int &bottom, const XWindow id);
    static void init();
    static void restack(const XWindow win, const XWindow parent);
    static void move(const XWindow win, const QPoint &p);
    static void wheelEvent(const XHandler::XWindow win, const bool up);
    static void pressEvent(const QPoint &globalPos, const XWindow win, const Qt::MouseButton button);
    static void releaseEvent(const QPoint &globalPos, const XWindow win, const Qt::MouseButton button);
    static void doubleClickEvent(const QPoint &globalPos, const XWindow win, const Qt::MouseButton button);

protected:
    static void changeProperty(const XWindow w, const Value v, const TypeSize size, const unsigned char *data, const unsigned int nitems);
    static unsigned char *fetchProperty(const XWindow w, const Value v, int &n, quint32 offset, quint32 length);
};
} //namespace

#endif //XHANDLER_H
