#include "windowdata.h"
#include <QObject>
#include <QDebug>

bool
WindowData::hasData(const QObject *parent)
{
    return parent->findChild<QSharedMemory *>("dsp_windowdata_memory");
}

WindowData
*WindowData::memory(const unsigned int wid, QObject *parent, const bool create)
{
    if (!wid)
    {
        qDebug() << "DSP: cant get windowdata w/o window id";
        return 0;
    }
    WindowData *m = parent->findChild<WindowData *>("dsp_windowdata_memory");
    if (!m)
    {
        m = new WindowData(QString("dsp_windowdata-%1").arg(QString::number(wid)), parent);
        m->setObjectName("dsp_windowdata_memory");
    }
    if (m->isAttached() || m->attach())
        return m;
    if (create && m->QSharedMemory::create((sizeof(unsigned int)*6)+(256*256*4)))
    {
        if (m->lock())
        {
            unsigned char *d = reinterpret_cast<unsigned char *>(m->data());
            memset(d, 0, m->size());
            m->unlock();
        }
        return m;
    }
    qDebug() << "DSP: unable to create shared memory.\n" << "error:\n" << m->errorString();
    if (m)
        m->deleteLater();
    return 0;
}

//QSharedMemory
//*WindowData::constSharedMemory(const unsigned int wid, const QObject *parent)
//{
//    if (!wid)
//    {
//        qDebug() << "DSP: cant get windowdata w/o window id";
//        return 0;
//    }
//    QSharedMemory *m = parent->findChild<QSharedMemory *>("dsp_windowdata_memory");
//    if (m && (m->isAttached() || m->attach()))
//        return m;
//    return 0;
//}

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
