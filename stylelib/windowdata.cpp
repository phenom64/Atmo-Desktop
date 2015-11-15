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

#if 0

SharedDataAdaptorManager *SharedDataAdaptorManager::s_instance = 0;

SharedDataAdaptorManager
*SharedDataAdaptorManager::instance()
{
    if (!s_instance)
        s_instance = new SharedDataAdaptorManager();
    return s_instance;
}

SharedDataAdaptorManager::SharedDataAdaptorManager(QObject *parent)
    : QObject(parent)
{
    m_adaptor = new SharedDataAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.kde.dsp.dspsharedmemory");
    QDBusConnection::sessionBus().registerObject("/DSPSharedMemAdaptor", this);
}

SharedDataAdaptorManager::~SharedDataAdaptorManager()
{
    QDBusConnection::sessionBus().unregisterService("org.kde.dsp.dspsharedmemory");
    QDBusConnection::sessionBus().unregisterObject("/DSPSharedMemAdaptor");
}

void
SharedDataAdaptorManager::updateData(uint win)
{
    qDebug() << "SharedDataAdaptorManager::updateData(uint win)";
//    WindowData::update(win);
}

void
SharedDataAdaptorManager::emitWindowActiveChanged(uint win, bool active)
{
    emit m_adaptor->windowActiveChanged(win, active);
}
void
SharedDataAdaptorManager::emitDataChanged(uint win)
{
    emit m_adaptor->dataChanged(win);
}

//----------------------------------------------------------------------------------------------------------

SharedDataAdaptor::SharedDataAdaptor(SharedDataAdaptorManager *parent)
    : QDBusAbstractAdaptor(parent),
      m_manager(parent)
{

}

#endif

//----------------------------------------------------------------------------------------------------------

WindowData
*WindowData::memory(const unsigned int wid, QObject *parent, const bool create)
{
    static const int s_memSize = (sizeof(unsigned int)*6)+(256*256*4);
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

//void
//WindowData::update(uint win)
//{
//    qDebug() << "WindowData::update(uint win)" << win << instances().count();
//    for (int i = 0; i < instances().count(); ++i)
//        if (instances().at(i)->m_winId == win)
//        {
//            qDebug() << "updateing data for" << win;
//            emit instances().at(i)->dataChanged();
//        }
//}

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

#if 0

//----------------------------------------------------------------------------------------------------------

ClientData::ClientData(const QString &key, QObject *parent, uint id)
    : WindowData(key, parent, id)
{
#if HASDBUS
    static const QString service("org.kde.dsp.dspsharedmemory"),
            interface("org.kde.dsp.sharedmemory"),
            path("/DSPSharedMemAdaptor");
    QDBusInterface *iface = new QDBusInterface(service, path, interface);
//    iface->connection().connect(service, path, interface, "windowActiveChanged", this, SLOT(decoActiveChanged(QDBusMessage)));
    iface->connection().connect(service, path, interface, "dataChanged", this, SLOT(dataChangedSlot(QDBusMessage)));
#endif
}

void
ClientData::dataChangedSlot(QDBusMessage)
{
    emit dataChanged();
}

bool
ClientData::sync(uint win)
{
#if HASDBUS
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.dsp.dspsharedmemory", "/DSPSharedMemAdaptor", "org.kde.dsp.sharedmemory", "updateData");
    if (win)
        msg << win;
    else
        msg << m_winId;
    QDBusConnection::sessionBus().send(msg);
    return true;
#endif
    return false;
}

//----------------------------------------------------------------------------------------------------------

//static QList<DecoData *> s_ddList;

DecoData::DecoData(const QString &key, QObject *parent, uint id)
    : WindowData(key, parent, id)
{
//    s_ddList << this;
//    SharedDataAdaptorManager::instance();
}

DecoData::~DecoData()
{
//    s_ddList.removeOne(this);
}

bool
DecoData::sync(uint win)
{
//    emit SharedDataAdaptorManager::instance()->dataC
#if HASDBUS
    SharedDataAdaptorManager::instance()->emitDataChanged(win);
    return true;
#else
    qDebug() << "sync failed! requires dbus!";
    return false;
#endif
}
#endif
