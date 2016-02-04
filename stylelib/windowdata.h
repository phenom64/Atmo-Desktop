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
    enum ValueMask {     //data is a 32 bit integer
        SeparatorMask =                 1<<0,       //first 8 bits reserved for booleans...
        ContAwareMask =                 1<<1,
        UnoMask =                       1<<2,
        HorizontalMask =                1<<3,
        WindowIconMask =                1<<4,
        EmbeddedButtonsMask=            1<<5,
        IsActiveWindowMask =            1<<6,
        OpacityMask =                   0x0000f000,
        UnoHeightMask =                 0x00ff0000, //the height of the head is never more then 255 right? ..right?
        ButtonsMask =                   0x0f000000, //enough long as we dont have more then 15 buttons styles
        FrameMask =                     0xf0000000,  //same here... long as we dont set framesize over 15, unsure if this is enough....
        TitleHeightMask =               0x000000ff,
        ShadowOpacityMask =             0x0000fe00,
        LeftEmbedSizeMask =             0x00fe0000,
        RightEmbedSizeMask=             0xff000000,
        FollowDecoShadowMask=           0x0000000f
    };
    enum ValueType {
        Separator = 0,
        ContAware,
        Uno,
        Horizontal,
        WindowIcon,
        EmbeddedButtons,
        IsActiveWindow,
        Opacity,
        UnoHeight,
        Buttons,
        Frame,
        TitleHeight,
        ShadowOpacity,
        LeftEmbedSize,
        RightEmbedSize,
        FollowDecoShadow
    };
    enum SharedValue { //when data cast to unsigned int ptr
        BgColor = 3, //first ones reserved for data
        FgColor,
        MinColor,
        MaxColor,
        CloseColor,
        ToolBtnGradientSize,
        ToolBtnGradient,
        ToolBtnGradientEnd = ToolBtnGradient+32,
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
            case IsActiveWindow:    d[0] ^= (-value ^ d[0]) & (1 << type); break; //just boolean vals...
            case Opacity:           d[0] = (d[0] & ~OpacityMask) | ((value << 8) & OpacityMask); break;
            case UnoHeight:         d[0] = (d[0] & ~UnoHeightMask) | ((value << 16) & UnoHeightMask); break;
            case Buttons:           d[0] = (d[0] & ~ButtonsMask) | (((value + 1) << 24) & ButtonsMask); break;
            case Frame:             d[0] = (d[0] & ~FrameMask) | ((value << 28) & FrameMask); break;
            case TitleHeight:       d[1] = (d[1] & ~TitleHeightMask) | (value & TitleHeightMask); break;
            case ShadowOpacity:     d[1] = (d[1] & ~ShadowOpacityMask) | ((value << 8) & ShadowOpacityMask); break;
            case LeftEmbedSize:     d[1] = (d[1] & ~LeftEmbedSizeMask) | ((value << 16) & LeftEmbedSizeMask); break;
            case RightEmbedSize:    d[1] = (d[1] & ~RightEmbedSizeMask) | ((value << 24) & RightEmbedSizeMask); break;
            case FollowDecoShadow:  d[2] = (d[2] & ~FollowDecoShadowMask) | (value & FollowDecoShadowMask);
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
            case IsActiveWindow:    return (T)(d[0] & (1 << type));
            case Opacity:           return (T)((d[0] & OpacityMask) >> 8);
            case UnoHeight:         return (T)((d[0] & UnoHeightMask) >> 16);
            case Buttons:           return (T)(((d[0] & ButtonsMask) >> 24) - 1);
            case Frame:             return (T)((d[0] & FrameMask) >> 28);
            case TitleHeight:       return (T)(d[1] & TitleHeightMask);
            case ShadowOpacity:     return (T)((d[1] & ShadowOpacityMask) >> 8);
            case LeftEmbedSize:     return (T)((d[1] & LeftEmbedSizeMask) >> 16);
            case RightEmbedSize:    return (T)((d[1] & RightEmbedSizeMask) >> 24);
            case FollowDecoShadow:  return (T)(d[2] & FollowDecoShadowMask);
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

    const QColor minColor();
    void setMinColor(const QColor &c);

    const QColor maxColor();
    void setMaxColor(const QColor &c);

    const QColor closeColor();
    void setCloseColor(const QColor &c);

    const uint decoId();
    void setDecoId(const uint id);

    const Gradient gradient();
    void setGradient(const Gradient g);

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

} //namespace

#endif //WINDOWDATA_H
