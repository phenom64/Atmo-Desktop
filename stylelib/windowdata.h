#ifndef WINDOWDATA_H
#define WINDOWDATA_H

#include "../config/settings.h"
#include <QSharedMemory>
#include <QDebug>

class Q_DECL_EXPORT WindowData : public QSharedMemory
{
    /// Convenience class for functions returning values
    /// so one doesnt have to store the return val in a var
    /// and then return that
    class MemoryLocker
    {
    public:
        MemoryLocker(QSharedMemory *m);
        ~MemoryLocker();
        inline const bool lockObtained() const { return m_lock; }
    private:
        QSharedMemory *m_mem;
        bool m_lock;
    };

public:
    enum ValueType {     //data is a 32 bit integer
        Separator =     1<<0,       //first 8 bits reserved for booleans...
        ContAware =     1<<1,
        Uno =           1<<2,
        Horizontal =    1<<3,
        WindowIcon =    1<<4,
        Opacity =       0x0000ff00,
        UnoHeight =     0x00ff0000, //the height of the head is never more then 255 right? ..right?
        Buttons =       0x0f000000, //enough long as we dont have more then 15 buttons styles
        Frame =         0xf0000000,  //same here... long as we dont set framesize over 15, unsure if this is enough....
        TitleHeight =   0x0000ffff,
        ShadowOpacity = 0xffff0000
    };
    enum SharedValue { //when data cast to unsigned int ptr
        ImageWidth = 4,
        ImageHeight = 5
    };

    typedef unsigned int Type;
    template<typename T> void setValue(const Type type, const T value)
    {
        if (lock())
        {
            unsigned int *d = reinterpret_cast<unsigned int *>(data());
            switch (type)
            {
            case Separator:
            case ContAware:
            case Uno:
            case Horizontal:
            case WindowIcon:    d[0] ^= (-value ^ d[0]) & type; break; //just boolean vals...
            case Opacity:       d[0] = (d[0] & ~Opacity) | ((value << 8) & Opacity); break;
            case UnoHeight:     d[0] = (d[0] & ~UnoHeight) | ((value << 16) & UnoHeight); break;
            case Buttons:       d[0] = (d[0] & ~Buttons) | (((value + 1) << 24) & Buttons); break;
            case Frame:         d[0] = (d[0] & ~Frame) | ((value << 28) & Frame); break;
            case TitleHeight:   d[1] = (d[1] & ~TitleHeight) | (value & TitleHeight); break;
            case ShadowOpacity: d[1] = (d[1] & ~ShadowOpacity) | ((value << 16) & ShadowOpacity); break;
            default: break;
            }
            unlock();
        }
    }

    template<typename T> T value(const Type type, const T defaultVal = T())
    {
        MemoryLocker locker(this);
        if (locker.lockObtained())
        {
            unsigned int *d = reinterpret_cast<unsigned int *>(data());
            switch (type)
            {
            case Separator:
            case ContAware:
            case Uno:
            case Horizontal:
            case WindowIcon:    return T(d[0] & type);
            case Opacity:       return T((d[0] & Opacity) >> 8);
            case UnoHeight:     return T((d[0] & UnoHeight) >> 16);
            case Buttons:       return T(((d[0] & Buttons) >> 24) - 1);
            case Frame:         return T((d[0] & Frame) >> 28);
            case TitleHeight:   return T(d[1] & TitleHeight);
            case ShadowOpacity: return T((d[1] & ShadowOpacity) >> 16);
            default:            return T();
            }
        }
        return defaultVal;
    }
    static WindowData *memory(const unsigned int wid, QObject *parent, const bool create = false);

    bool isEmpty();
    bool sync();

    QColor bg();
    void setBg(const QColor &c);

    QColor fg();
    void setFg(const QColor &c);

    QImage image();

    //this should be passed to the QImage constructor...
    uchar *imageData();

    void setImageSize(const int w, const int h);
    QSize imageSize();

protected:
    WindowData(const QString &key, QObject *parent, uint id = 0):QSharedMemory(key, parent),m_winId(id){setObjectName(key);}

private:
    unsigned int m_winId;
};

#endif //WINDOWDATA_H
