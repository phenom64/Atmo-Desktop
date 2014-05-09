#ifndef XHANDLER_H
#define XHANDLER_H

#include <QWidget>
#include <QDebug>

typedef struct _WindowData
{
    QRgb top, bottom, text;
    bool separator;
    unsigned int height;
} WindowData;

static int _n = 0;
class Q_DECL_EXPORT XHandler
{
public:
    enum Value { WindowData = 0, KwinShadows, StoreShadow, DecoData, ValueCount };
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
protected:
    static void changeProperty(const WId w, const Value v, const TypeSize size, const unsigned char *data, const unsigned int nitems);
    static unsigned char *fetchProperty(const WId w, const Value v, int &n);
//private:
//    static Atom atom[ValueCount];
};

#endif //XHANDLER_H
