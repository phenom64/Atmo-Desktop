#ifndef XHANDLER_H
#define XHANDLER_H

#include <QWidget>
#include <QDebug>

class WindowData
{
public:
    enum DataType { Separator = 0x1, ContAware = 0x2, Uno = 0x4, Horizontal = 0x8, Opacity = 0xff00, UnoHeight = 0xff0000, Buttons = 0xf000000, Frame = 0xf0000000 };
    QRgb text, bg;
    unsigned int data;
};

static int _n = 0;
class Q_DECL_EXPORT XHandler : public QObject
{
    Q_OBJECT
public:
    enum Value { WindowIcon = 0, KwinShadows, KwinBlur, WindowData, StoreActiveShadow, StoreInActiveShadow, DecoTitleHeight, DecoBgPix, ContPix, ValueCount };
    enum Size { Byte = 8, Short = 16, Long = 32, LongLong = 64 };
    typedef unsigned int TypeSize;
    template<typename T> static void setXProperty(const WId w, const Value v, const TypeSize size, T *d, unsigned int n = 1)
    {
        const TypeSize realSize(sizeof(T));
        if (realSize > size/8)
            n *= realSize;

        changeProperty(w, v, qMin<TypeSize>(size, Long), reinterpret_cast<unsigned char *>(d), n);
    }
    template<typename T> static T *getXProperty(const WId w, const Value v, int &n = _n, unsigned long offset = 0L, unsigned long length = 0xffffffff)
    {
        return reinterpret_cast<T *>(fetchProperty(w, v, n, offset, length));
    }
    static unsigned long xAtom(Value v);
    static void freeData(void *data);
    static void deleteXProperty(const WId w, const Value v);
    static void mwRes(const QPoint &globalPoint, const WId &win, bool resize = false);
    static bool compositingActive();
    static float opacity();
    static QPixmap x11Pix(const QPixmap &pix, Qt::HANDLE &handle, const QWidget *win = 0);
    static QPixmap x11Pix(const QPixmap &pix);
    static void freePix(QPixmap pix);
    static void freePix(const Qt::HANDLE handle);
    static void updateBgPix(const WId w);
    static QPixmap emptyX11Pix(const QSize &sz, const WId w);
    static XHandler *instance();

    XHandler(QObject *parent = 0);
    ~XHandler();

protected:
    static void changeProperty(const WId w, const Value v, const TypeSize size, const unsigned char *data, const unsigned int nitems);
    static unsigned char *fetchProperty(const WId w, const Value v, int &n, unsigned long offset, unsigned long length);

private:
    static XHandler s_instance;
//    QTimer *m_timer;
};

#endif //XHANDLER_H
