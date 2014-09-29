#ifndef XHANDLER_H
#define XHANDLER_H

#include <QWidget>
#include <QDebug>

typedef struct _WindowData
{
    QRgb text;
    bool separator;
    unsigned int height, opacity;
} WindowData;

static int _n = 0;
class Q_DECL_EXPORT XHandler
{
public:
    enum Value { WindowData = 0, KwinShadows, KwinBlur, StoreShadow, MenuShadowUp, MenuShadowDown, DecoData, DecoBgPix, ContPix, ValueCount };
    enum Size { Byte = 8, Short = 16, Long = 32, LongLong = 64 };
    typedef unsigned int TypeSize;
    template<typename T> static void setXProperty(const WId w, const Value v, const TypeSize size, T *d, unsigned int n = 1)
    {
        const TypeSize realSize(sizeof(T));
        if (realSize > size/8)
            n *= realSize;

        changeProperty(w, v, qMin<TypeSize>(size, Long), reinterpret_cast<unsigned char *>(d), n);
    }
    template<typename T> static T *getXProperty(const WId w, const Value v, int &n = _n)
    {
        return reinterpret_cast<T *>(fetchProperty(w, v, n));
    }
    static void deleteXProperty(const WId w, const Value v);
    static void mwRes(const QPoint &globalPoint, const WId &win, bool resize = false);
    static bool compositingActive();
    static float opacity();

    static QPixmap x11Pix(const QPixmap &pix, const Qt::HANDLE handle = 0);
    static void freePix(QPixmap pix);
    static void freePix(const Qt::HANDLE handle);
protected:
    static void changeProperty(const WId w, const Value v, const TypeSize size, const unsigned char *data, const unsigned int nitems);
    static unsigned char *fetchProperty(const WId w, const Value v, int &n);
//private:
//    static Atom atom[ValueCount];
};

#endif //XHANDLER_H
