#include "windowdata.h"
#include <QObject>
#include <QDebug>
//#include <QImage>

#include "defines.h"
#if HASDBUS
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#endif

using namespace DSP;

WindowData
*WindowData::memory(const unsigned int wid, QObject *parent, const bool create)
{
    static const int s_memSize = (sizeof(unsigned int)*(ImageHeight))+(2048*256*4);
    if (!wid)
    {
//        qDebug() << "DSP: cant get windowdata w/o window id";
        return 0;
    }
    const QString &keyName = QString("dsp_windowdata-%1").arg(QString::number(wid));
    WindowData *m = parent->findChild<WindowData *>(keyName);
    if (!m)
        m = new WindowData(keyName, parent, wid);
    if (m->isAttached() || m->attach())
    {
        if (m->size() == s_memSize)
            return m;
        else
        {
            m->detach();
            m->deleteLater();
            m = new WindowData(keyName, parent, wid);
            if (m->isAttached() || m->attach())
                return m;
        }
    }
    if (create && m->create(s_memSize))
    {
        if (m->lock())
        {
            unsigned char *d = reinterpret_cast<unsigned char *>(m->data());
            memset(d, 0, m->size());
            m->unlock();
        }
        return m;
    }
//    qDebug() << "DSP: unable to get/create shared memory.\n" << "error:\n" << m->errorString() << "memory key:" << m->key();
    if (m)
        m->deleteLater();
    return 0;
}

WindowData::WindowData(const QString &key, QObject *parent, uint id)
    : QSharedMemory(key, parent)
    , m_winId(id)
{
    setObjectName(key);
}

void
WindowData::setImageSize(const int w, const int h)
{
    uint *size = reinterpret_cast<uint *>(data());
    size[ImageWidth] = w;
    size[ImageHeight] = h;
//    m_image = QImage(constImageData(), w, h, QImage::Format_ARGB32_Premultiplied);
}

uchar
*WindowData::imageData()
{
    uint *d = reinterpret_cast<uint *>(data());
    return reinterpret_cast<uchar *>(&d[ImageData]);
}

const uchar
*WindowData::constImageData() const
{
    const uint *d = reinterpret_cast<const uint *>(constData());
    return reinterpret_cast<const uchar *>(&d[ImageData]);
}

const QSize
WindowData::imageSize() const
{
    const uint *d = reinterpret_cast<const uint *>(constData());
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
        const uint *d = reinterpret_cast<const uint *>(constData());
        return QColor::fromRgba(d[BgColor]);
    }
    return QColor();
}

void
WindowData::setBg(const QColor &c)
{
    if (lock())
    {
        uint *d = reinterpret_cast<uint *>(data());
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
        const uint *d = reinterpret_cast<const uint *>(constData());
        return QColor::fromRgba(d[FgColor]);
    }
    return QColor();
}

void
WindowData::setFg(const QColor &c)
{
    if (lock())
    {
        uint *d = reinterpret_cast<uint *>(data());
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
        const uint *d = reinterpret_cast<const uint *>(constData()); \
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
        uint *d = reinterpret_cast<uint *>(data()); \
        d[_VAR_##Color] = c.rgba(); \
        unlock(); \
    } \
}

SETCOLOR(Min)
SETCOLOR(Max)
SETCOLOR(Close)

void
WindowData::setDecoId(const uint id)
{
    if (lock())
    {
        uint *d = reinterpret_cast<uint *>(data());
        d[DecoID] = id;
        unlock();
    }
}

const uint
WindowData::decoId()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        const uint *d = reinterpret_cast<const uint *>(constData());
        return d[DecoID];
    }
    return 0;
}

void
WindowData::setGradient(const Gradient g)
{
    if (lock())
    {
        uint *d = reinterpret_cast<uint *>(data());
        d[ToolBtnGradientSize] = qMin(32, g.count());
        d = &d[ToolBtnGradient];
        for (int i = 0; i < g.count(); ++i)
        {
            GradientStop stop(g.at(i));
            d[i] = (d[i] & ~0xffff0000) | (((int)(stop.first*10000) << 16) & 0xffff0000);
            d[i] = (d[i] & ~0x0000ffff) | ((stop.second+100) & 0x0000ffff);
        }
        unlock();
    }
}

const Gradient
WindowData::gradient()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        const uint *d = reinterpret_cast<const uint *>(constData());
        const int stops = d[ToolBtnGradientSize];
        const uint *data = &d[ToolBtnGradient];
        Gradient g;
        for (int i = 0; i < stops; ++i)
            g << GradientStop((float)((data[i]&0xffff0000)>>16)/10000.0f, (int)(data[i]&0x0000ffff)-100);
        return g;
    }
    return Gradient();
}

bool
WindowData::isEmpty()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        const uint *d = reinterpret_cast<const uint *>(data());
        return !d[0] && !d[1] && !d[2] && !d[3] && !d[4] && !d[5];
    }
    return true;
}

void
WindowData::sync()
{
#if HASDBUS
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.dsp.kwindeco", "/DSPDecoAdaptor", "org.kde.dsp.deco", "updateData");
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
