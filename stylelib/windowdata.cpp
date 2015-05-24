#include "windowdata.h"
#include "xhandler.h"
#include <QSharedMemory>
#include <QMap>
#include <QObject>
#include <QDebug>

#if !defined(QT_NO_DBUS)
#include <QDBusMessage>
#include <QDBusConnection>
#endif

QMap<unsigned int, QSharedMemory *> WindowData::s_windowData;

bool
WindowData::hasData(const unsigned int wid)
{
    return s_windowData.contains(wid);
}

QSharedMemory
*WindowData::sharedMemory(const unsigned int wid, QObject *parent, const bool create)
{
    if (!wid)
    {
        qDebug() << "DSP: cant get windowdata w/o window id";
        return 0;
    }
    if (!s_windowData.contains(wid))
        s_windowData.insert(wid, new QSharedMemory(QString("dsp_windowdata-%1").arg(QString::number(wid)), parent));

    QSharedMemory *m = s_windowData.value(wid);
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
WindowData::detach(const unsigned int wid)
{
    if (!wid || !s_windowData.contains(wid))
    {
        qDebug() << "DSP: cant detach shared memory segment from window id" << wid;
        return;
    }

    if (QSharedMemory *m = s_windowData.value(wid))
    if (m->isAttached())
    if (m->lock())
    {
        if (WindowData *wd = reinterpret_cast<WindowData *>(m->data()))
        if (wd->pix)
            XHandler::freePix(wd->pix);
        m->unlock();
        m->detach();
        s_windowData.remove(wid);
    }
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

QMap<unsigned int, QSharedMemory *> SharedBgImage::s_imageData;

bool
SharedBgImage::hasData(const unsigned int wid)
{
    return s_imageData.contains(wid);
}

QSharedMemory
*SharedBgImage::sharedMemory(const unsigned int wid, QObject *parent, const bool create)
{
    if (!wid)
    {
        qDebug() << "DSP: cant get imageData w/o window id";
        return 0;
    }
    if (!s_imageData.contains(wid))
        s_imageData.insert(wid, new QSharedMemory(QString("dsp_imagedata-%1").arg(QString::number(wid)), parent));

    QSharedMemory *m = s_imageData.value(wid);
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

void
SharedBgImage::detach(const unsigned int wid)
{
    if (!wid || !s_imageData.contains(wid))
    {
        qDebug() << "DSP: cant detach shared image memory segment from window id" << wid;
        return;
    }

    if (QSharedMemory *m = s_imageData.value(wid))
    if (m->isAttached())
    {
        m->detach();
        s_imageData.remove(wid);
    }
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
