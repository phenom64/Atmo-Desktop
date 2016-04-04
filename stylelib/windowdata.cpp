#include "windowdata.h"
#include <QObject>
#include <QDebug>
#include <QWidget>
//#include <QImage>

#include "defines.h"
#if HASDBUS
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#endif

using namespace DSP;

WindowData
*WindowData::memory(const quint32 wid, QObject *parent, const bool create)
{
    static const int s_memSize = (sizeof(quint32)*(ImageHeight))+(2048*256*4);
    if (!wid || !parent)
        return 0;
    const QString keyName = QString("dspwindowdata%1").arg(QString::number(wid));
    WindowData *m = parent->findChild<WindowData *>(keyName);
    if (!m)
        m = new WindowData(keyName, parent, wid);
    if (m->isAttached() || m->attach())
        return m;
    if (create && m->create(s_memSize) && m->lock())
    {
        memset(m->data(), 0, m->size());
        m->unlock();
        return m;
    }
    return 0;
}

WindowData::WindowData(const QString &key, QObject *parent, quint32 id)
    : QSharedMemory(key, parent)
    , m_winId(id)
{
    setObjectName(key);
}

WindowData::~WindowData()
{
    if (isAttached())
        detach();
}

void
WindowData::setImageSize(const int w, const int h)
{
    quint32 *size = static_cast<quint32 *>(data());
    size[ImageWidth] = w;
    size[ImageHeight] = h;
//    m_image = QImage(constImageData(), w, h, QImage::Format_ARGB32_Premultiplied);
}

uchar
*WindowData::imageData()
{
    quint32 *d = static_cast<quint32 *>(data());
    return reinterpret_cast<uchar *>(&d[ImageData]);
}

const uchar
*WindowData::constImageData() const
{
    const quint32 *d = static_cast<const quint32 *>(constData());
    return reinterpret_cast<const uchar *>(&d[ImageData]);
}

const QSize
WindowData::imageSize() const
{
    const quint32 *d = static_cast<const quint32 *>(constData());
    return QSize(d[ImageWidth], d[ImageHeight]);
}

const QImage
WindowData::image() const
{
//#if 1
//    return m_image;
//#else
    const QSize &sz = imageSize();
    return QImage(constImageData(), sz.width(), sz.height(), QImage::Format_ARGB32_Premultiplied);
//#endif
}

const QColor
WindowData::bg()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        const quint32 *d = static_cast<const quint32 *>(constData());
        return QColor::fromRgba(d[BgColor]);
    }
    return QColor();
}

void
WindowData::setBg(const QColor &c)
{
    if (lock())
    {
        quint32 *d = static_cast<quint32 *>(data());
        d[BgColor] = c.rgba();
        unlock();
    }
}

const QColor
WindowData::fg()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        const quint32 *d = static_cast<const quint32 *>(constData());
        return QColor::fromRgba(d[FgColor]);
    }
    return QColor();
}

void
WindowData::setFg(const QColor &c)
{
    if (lock())
    {
        quint32 *d = static_cast<quint32 *>(data());
        d[FgColor] = c.rgba();
        unlock();
    }
}

#define GETCOLOR(_ENUM_, _FUNC_) const QColor \
WindowData::_FUNC_##Color() \
{ \
    MemoryLocker locker(this); \
    if (locker.lockObtained()) \
    { \
        const quint32 *d = static_cast<const quint32 *>(constData()); \
        return QColor::fromRgba(d[_ENUM_##Color]); \
    } \
    return QColor(); \
}

GETCOLOR(Min, min)
GETCOLOR(Max, max)
GETCOLOR(Close, close)

#define SETCOLOR(_VAR_) void \
WindowData::set##_VAR_##Color(const QColor &c) \
{ \
    if (lock()) \
    { \
        quint32 *d = static_cast<quint32 *>(data()); \
        d[_VAR_##Color] = c.rgba(); \
        unlock(); \
    } \
}

SETCOLOR(Min)
SETCOLOR(Max)
SETCOLOR(Close)

void
WindowData::setDecoId(const quint32 id)
{
    if (lock())
    {
        quint32 *d = static_cast<quint32 *>(data());
        d[DecoID] = id;
        unlock();
    }
}

const quint32
WindowData::decoId()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        const quint32 *d = static_cast<const quint32 *>(constData());
        return d[DecoID];
    }
    return 0;
}

static void gradientToData(const Gradient g, quint32 *d)
{
    for (int i = 0; i < g.count(); ++i)
    {
        GradientStop stop(g.at(i));
        d[i] = (d[i] & ~0xffff0000) | (((int)(stop.first*10000) << 16) & 0xffff0000);
        d[i] = (d[i] & ~0x0000ffff) | ((stop.second+100) & 0x0000ffff);
    }
}

void
WindowData::setButtonGradient(const Gradient g)
{
    if (lock())
    {
        quint32 *d = static_cast<quint32 *>(data());
        d[ToolBtnGradientSize] = qMin(32, g.count());
        d = &d[ToolBtnGradient];
        gradientToData(g, d);
        unlock();
    }
}

void
WindowData::setWindowGradient(const Gradient g)
{
    if (lock())
    {
        quint32 *d = static_cast<quint32 *>(data());
        d[WindowGradientSize] = qMin(32, g.count());
        d = &d[WindowGradient];
        gradientToData(g, d);
        unlock();
    }
}

static Gradient dataToGradient(const quint32 *data, const int stops)
{
    Gradient g;
    for (int i = 0; i < stops; ++i)
        g << GradientStop((float)((data[i]&0xffff0000)>>16)/10000.0f, (int)(data[i]&0x0000ffff)-100);
    return g;
}

const Gradient
WindowData::buttonGradient()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        const quint32 *d = static_cast<const quint32 *>(constData());
        const int stops = d[ToolBtnGradientSize];
        const quint32 *data = &d[ToolBtnGradient];
        return dataToGradient(data, stops);
    }
    return Gradient();
}

const Gradient
WindowData::windowGradient()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        const quint32 *d = static_cast<const quint32 *>(constData());
        const int stops = d[WindowGradientSize];
        const quint32 *data = &d[WindowGradient];
        return dataToGradient(data, stops);
    }
    return Gradient();
}

bool
WindowData::isEmpty()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        const quint32 *d = static_cast<const quint32 *>(constData());
        return !d[0] && !d[1] && !d[2] && !d[3] && !d[4] && !d[5];
    }
    return true;
}

void
WindowData::sync()
{
#if HASDBUS
    static const QString destination("org.kde.dsp.kwindeco");
    static const QString path("/DSPDecoAdaptor");
    static const QString interface("org.kde.dsp.deco");
    static const QString method("updateData");
    QDBusMessage msg = QDBusMessage::createMethodCall(destination, path, interface, method);
    msg << m_winId;
    QDBusConnection::sessionBus().send(msg);
#endif
}

//----------------------------------------------------------------------------------------------------------

WindowData::MemoryLocker::MemoryLocker(QSharedMemory *m)
    : m_mem(m)
    , m_lock(false)
{
    if (m_mem)
        m_lock = m_mem->lock();
}

WindowData::MemoryLocker::~MemoryLocker()
{
    if (m_mem && m_lock)
        m_mem->unlock();
}
