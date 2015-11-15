#ifndef WINDOWDATA_H
#define WINDOWDATA_H

#include "../config/settings.h"
#include <QSharedMemory>
#include <QDebug>

#if HASDBUS
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusMessage>
#endif

//class QImage;

namespace DSP
{

class WindowData;
class ClientData;
class DecoData;
class SharedDataAdaptorManager;
class SharedDataAdaptor;
#if 0

class SharedDataAdaptorManager : public QObject
{
    Q_OBJECT
public:
    static SharedDataAdaptorManager *instance();
    void updateData(uint win);
    void emitWindowActiveChanged(uint win, bool active);
    void emitDataChanged(uint win);

protected:
    SharedDataAdaptorManager(QObject *parent = 0);
    ~SharedDataAdaptorManager();

private:
    static SharedDataAdaptorManager *s_instance;
    SharedDataAdaptor *m_adaptor;
};

class SharedDataAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.dsp.sharedmemory")

public:
    SharedDataAdaptor(SharedDataAdaptorManager *parent);

public slots:
    Q_NOREPLY void updateData(uint win) { m_manager->updateData(win); }

signals:
     void windowActiveChanged(uint win, bool active);
     void dataChanged(uint win);

private:
     SharedDataAdaptorManager *m_manager;
};

#endif

class Q_DECL_EXPORT WindowData : public QSharedMemory
{
    Q_OBJECT
    friend class DecoData;
    friend class ClientData;
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
        EmbeddedButtons=1<<5,
        IsActiveWindow =1<<6,
        Opacity =       0x0000f00,
        UnoHeight =     0x00ff0000, //the height of the head is never more then 255 right? ..right?
        Buttons =       0x0f000000, //enough long as we dont have more then 15 buttons styles
        Frame =         0xf0000000,  //same here... long as we dont set framesize over 15, unsure if this is enough....
        TitleHeight =   0x000000ff,
        ShadowOpacity = 0x0000fe00,
        LeftEmbedSize = 0x00fe0000,
        RightEmbedSize= 0xff000000
    };
    enum SharedValue { //when data cast to unsigned int ptr
        BgColor = 2,
        FgColor,
        DecoID,
        ImageWidth,
        ImageHeight,
        ImageData
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
            case WindowIcon:
            case EmbeddedButtons:
            case IsActiveWindow:    d[0] ^= (-value ^ d[0]) & type; break; //just boolean vals...
            case Opacity:           d[0] = (d[0] & ~Opacity) | ((value << 8) & Opacity); break;
            case UnoHeight:         d[0] = (d[0] & ~UnoHeight) | ((value << 16) & UnoHeight); break;
            case Buttons:           d[0] = (d[0] & ~Buttons) | (((value + 1) << 24) & Buttons); break;
            case Frame:             d[0] = (d[0] & ~Frame) | ((value << 28) & Frame); break;
            case TitleHeight:       d[1] = (d[1] & ~TitleHeight) | (value & TitleHeight); break;
            case ShadowOpacity:     d[1] = (d[1] & ~ShadowOpacity) | ((value << 8) & ShadowOpacity); break;
            case LeftEmbedSize:     d[1] = (d[1] & ~LeftEmbedSize) | ((value << 16) & LeftEmbedSize); break;
            case RightEmbedSize:    d[1] = (d[1] & ~RightEmbedSize) | ((value << 24) & RightEmbedSize); break;
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
            case WindowIcon:
            case EmbeddedButtons:
            case IsActiveWindow:    return T(d[0] & type);
            case Opacity:           return T((d[0] & Opacity) >> 8);
            case UnoHeight:         return T((d[0] & UnoHeight) >> 16);
            case Buttons:           return T(((d[0] & Buttons) >> 24) - 1);
            case Frame:             return T((d[0] & Frame) >> 28);
            case TitleHeight:       return T(d[1] & TitleHeight);
            case ShadowOpacity:     return T((d[1] & ShadowOpacity) >> 8);
            case LeftEmbedSize:     return T((d[1] & LeftEmbedSize) >> 16);
            case RightEmbedSize:    return T((d[1] & RightEmbedSize) >> 24);
            default:                return T();
            }
        }
        return defaultVal;
    }
    static WindowData *memory(const unsigned int wid, QObject *parent, const bool create = false);

    bool isEmpty();
    bool sync();

    const QColor bg();
    void setBg(const QColor &c);

    const QColor fg();
    void setFg(const QColor &c);

    const uint decoId();
    void setDecoId(const uint id);

    const QImage image() const;

    //this should be passed to the QImage constructor...
    uchar *imageData();
    const uchar *constImageData() const;

    void setImageSize(const int w, const int h);
    const QSize imageSize() const;

    inline const uint winId() const { return m_winId; }
    static QList<WindowData *> instances();

protected:
    WindowData(const QString &key, QObject *parent, uint id = 0);

private:
    unsigned int m_winId;
};

#if 0
class ClientData : public WindowData
{
    Q_OBJECT
    friend class WindowData;
public:
    bool sync(uint win = 0);
//    void update(uint win = 0);

protected:
    ClientData(const QString &key, QObject *parent, uint id = 0);

protected slots:
//    void decoActiveChanged(QDBusMessage);
    void dataChangedSlot(QDBusMessage);
};

class DecoData : public WindowData
{
    friend class WindowData;
public:
    ~DecoData();
    bool sync(uint win = 0);
//    static void update(uint win);

protected:
    DecoData(const QString &key, QObject *parent, uint id = 0);
};
#endif

} //namespace

#endif //WINDOWDATA_H
