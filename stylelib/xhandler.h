#ifndef XHANDLER_H
#define XHANDLER_H

#include <QMetaType>
#include <QDataStream>

class WindowData
{
public:
    enum DataType {     //data is a 32 bit integer
        Separator =     1<<0,       //first 8 bits reserved for booleans...
        ContAware =     1<<1,
        Uno =           1<<2,
        Horizontal =    1<<3,
        Opacity =       0x0000ff00,
        UnoHeight =     0x00ff0000, //the height of the head is never more then 255 right? ..right?
        Buttons =       0x0f000000, //enough long as we dont have more then 15 buttons styles
        Frame =         0xf0000000  //same here... long as we dont set framesize over 15, unsure if this is enough....
    };
    typedef unsigned int Type;
    unsigned int data, fg, bg;
    WindowData() : data(0), fg(0), bg(0) {}
//    explicit WindowData(const WindowData &wd) : data(wd.data), fg(wd.fg), bg(wd.bg) {}
    template<typename T> void setValue(const Type type, const T value = T())
    {
        switch (type)
        {
        case Separator:
        case ContAware:
        case Uno:
        case Horizontal: data |= (type*value); break; //just boolean vals...
        case Opacity: data |= ((value << 8) & Opacity); break;
        case UnoHeight: data |= ((value << 16) & UnoHeight); break;
        case Buttons: data |= ((value + 1) << 24); break;
        case Frame: data |= (value << 28); break;
        default: break;
        }
    }
    template<typename T> const T value(const Type type) const
    {
        switch (type)
        {
        case Separator:
        case ContAware:
        case Uno:
        case Horizontal: return T(data & type);
        case Opacity: return T((data & Opacity) >> 8);
        case UnoHeight: return T((data & UnoHeight) >> 16);
        case Buttons: return T(((data & Buttons) >> 24) - 1);
        case Frame: return T((data & Frame) >> 28);
        default: return T();
        }
    }
    inline void operator=(const WindowData &wd) { this->data = wd.data; this->fg = wd.fg; this->bg = wd.bg; }
    inline bool operator==(const WindowData &wd) const { return (this->data == wd.data && this->fg == wd.fg && this->bg == wd.bg); }
    inline bool operator!=(const WindowData &wd) const { return !operator==(wd); }
};

static int _n = 0;
class QPixmap;
class QPoint;
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
                 DecoBgPix,
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
    typedef unsigned long XWindow;
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
    static bool compositingActive();
    static float opacity();
    static QPixmap x11Pix(const QPixmap &pix, XWindow &handle, const XWindow winId = 0);
    static QPixmap x11Pix(const QPixmap &pix);
    static void freePix(QPixmap pix);
    static void freePix(const XWindow handle);
    static void updateDeco(const XWindow w);
    static QPoint strutTopLeft();
    static void getDecoBorders(int &left, int &right, int &top, int &bottom, const XWindow id);

protected:
    static void changeProperty(const XWindow w, const Value v, const TypeSize size, const unsigned char *data, const unsigned int nitems);
    static unsigned char *fetchProperty(const XWindow w, const Value v, int &n, unsigned long offset, unsigned long length);
};

#endif //XHANDLER_H
