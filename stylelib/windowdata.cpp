#include "windowdata.h"
#include "xhandler.h"
#include <QSharedMemory>
#include <QObject>
#include <QDebug>

#if !defined(QT_NO_DBUS)
#include <QDBusMessage>
#include <QDBusConnection>
#endif

bool
WindowData::hasData(QObject *parent)
{
    return parent->findChild<QSharedMemory *>("dsp_windowdata_memory");
}

QSharedMemory
*WindowData::sharedMemory(const unsigned int wid, QObject *parent, const bool create)
{
    if (!wid)
    {
        qDebug() << "DSP: cant get windowdata w/o window id";
        return 0;
    }
    QSharedMemory *m = parent->findChild<QSharedMemory *>("dsp_windowdata_memory");
    if (!m)
    {
        m = new QSharedMemory(QString("dsp_windowdata-%1").arg(QString::number(wid)), parent);
        m->setObjectName("dsp_windowdata_memory");
    }
    if (!m->isAttached() && !m->attach() && create)
    {
        if (m->create(sizeof(WindowData)))
            return m;
        else
        {
            qDebug() << "DSP: unable to create shared memory... cannot talk to deco.\n" << "error:\n" << m->errorString();
            return 0;
        }
    }
    return m;
}

void
WindowData::setData(const unsigned int wid, QObject *parent, WindowData *wd, const bool force)
{
    QSharedMemory *m = WindowData::sharedMemory(wid, parent, force);
    if (m && m->isAttached() && m->lock())
    {
        memcpy(m->data(), wd, m->size());
        m->unlock();
    }

#if !defined(QT_NO_DBUS)
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.dsp.kdecoration2", "/DSPDecoAdaptor", "org.kde.dsp.deco", "updateData");
    msg << wid;
    QDBusConnection::sessionBus().send(msg);
#endif
}

//---------------------------------------------------------------------------------------------------

bool
SharedBgImage::hasData(QObject *parent)
{
    return parent->findChild<QSharedMemory *>("dsp_imagedata_memory");
}

QSharedMemory
*SharedBgImage::sharedMemory(const unsigned int wid, QObject *parent, const bool create)
{
    if (!wid)
    {
        qDebug() << "DSP: cant get imageData w/o window id";
        return 0;
    }
    QSharedMemory *m = parent->findChild<QSharedMemory *>("dsp_imagedata_memory");
    if (!m)
    {
        m = new QSharedMemory(QString("dsp_imagedata-%1").arg(QString::number(wid)), parent);
        m->setObjectName("dsp_imagedata_memory");
    }
    if (!m->isAttached() && !m->attach() && create)
    {
        if (m->create(256*256*4+(sizeof(unsigned int)*2))) //2 32 bit ints at start for width and height...
            return m;
        else
        {
            qDebug() << "DSP: unable to create shared image memory... cannot set bg for deco.\n" << "error:\n" << m->errorString();
            return 0;
        }
    }
    return m;
}

uchar
*SharedBgImage::data(void *fromData)
{
    return &reinterpret_cast<uchar *>(fromData)[8];
}

QSize
SharedBgImage::size(void *fromData)
{
    unsigned int *data = reinterpret_cast<unsigned int *>(fromData);
    return QSize(data[0], data[1]);
}
