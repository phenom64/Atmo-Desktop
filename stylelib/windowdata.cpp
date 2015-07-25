#include "windowdata.h"
#include <QObject>
#include <QDebug>

#include "defines.h"
#if HASDBUS
#include <QDBusMessage>
#include <QDBusConnection>
#endif

static const int s_memSize = (sizeof(unsigned int)*6)+(256*256*4);

WindowData
*WindowData::memory(const unsigned int wid, QObject *parent, const bool create)
{
    if (!wid)
    {
        qDebug() << "DSP: cant get windowdata w/o window id";
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
    qDebug() << "DSP: unable to get/create shared memory.\n" << "error:\n" << m->errorString() << "memory key:" << m->key();
    if (m)
        m->deleteLater();
    return 0;
}

void
WindowData::setImageSize(const int w, const int h)
{
    uint *size = reinterpret_cast<uint *>(data());
    size[ImageWidth] = w;
    size[ImageHeight] = h;
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
    const QSize &sz = imageSize();
    return QImage(constImageData(), sz.width(), sz.height(), QImage::Format_ARGB32_Premultiplied);
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

bool
WindowData::sync()
{
#if HASDBUS
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.dsp.kwindeco", "/DSPDecoAdaptor", "org.kde.dsp.deco", "updateData");
    msg << m_winId;
    QDBusConnection::sessionBus().send(msg);
    return true;
#endif
    return false;
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
