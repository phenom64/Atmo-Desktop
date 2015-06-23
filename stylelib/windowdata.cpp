#include "windowdata.h"
#include <QObject>
#include <QDebug>

#if !defined(QT_NO_DBUS)
#include <QDBusMessage>
#include <QDBusConnection>
#endif

static int s_memSize = (sizeof(unsigned int)*6)+(256*256*4);

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
    unsigned int *size = reinterpret_cast<unsigned int *>(data());
    size[ImageWidth] = w;
    size[ImageHeight] = h;
}

uchar
*WindowData::imageData()
{
    uchar *d = reinterpret_cast<uchar *>(data());
    d = &d[sizeof(unsigned int)*4];
    unsigned int *size = reinterpret_cast<unsigned int *>(d);
    d = reinterpret_cast<uchar *>(&size[2]);
    return d;
}

QSize
WindowData::imageSize()
{
    uchar *d = reinterpret_cast<uchar *>(data());
    d = &d[sizeof(unsigned int)*4];
    unsigned int *size = reinterpret_cast<unsigned int *>(d);
    return QSize(size[0], size[1]);
}

QImage
WindowData::image()
{
    const QSize &sz = imageSize();
    return QImage(imageData(), sz.width(), sz.height(), QImage::Format_ARGB32_Premultiplied);
}

QColor
WindowData::bg()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        unsigned int *d = reinterpret_cast<unsigned int *>(data());
        return QColor::fromRgba(d[2]);
    }
    return QColor();
}

void
WindowData::setBg(const QColor &c)
{
    if (lock())
    {
        unsigned int *d = reinterpret_cast<unsigned int *>(data());
        d[2] = c.rgba();
        unlock();
    }
}

QColor
WindowData::fg()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        unsigned int *d = reinterpret_cast<unsigned int *>(data());
        return QColor::fromRgba(d[3]);
    }
    return QColor();
}

void
WindowData::setFg(const QColor &c)
{
    if (lock())
    {
        unsigned int *d = reinterpret_cast<unsigned int *>(data());
        d[3] = c.rgba();
        unlock();
    }
}

bool
WindowData::isEmpty()
{
    MemoryLocker locker(this);
    if (locker.lockObtained())
    {
        unsigned int *d = reinterpret_cast<unsigned int *>(data());
        return !d[0] && !d[1] && !d[2] && !d[3] && !d[4] && !d[5];
    }
    return true;
}

bool
WindowData::sync()
{
#if !defined(QT_NO_DBUS)
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
