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

const QImage
WindowData::image() const
{
    const DataStruct *d = dataStruct();
    return QImage(d->imageData, d->imageWidth, d->imageHeight, QImage::Format_ARGB32_Premultiplied);
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
    DataStruct *d = dataStruct();
    d->toolBtnGradientStopCount = qMin(32, g.count());
    gradientToData(g, d->toolBtnGradientStops);
}

void
WindowData::setWindowGradient(const Gradient g)
{
    DataStruct *d = dataStruct();
    d->windowGradientStopCount = qMin(32, g.count());
    gradientToData(g, d->windowGradientStops);
}

Gradient
WindowData::dataToGradient(const quint32 *data, const int stops)
{
    Gradient g;
    for (int i = 0; i < stops; ++i)
        g << GradientStop((float)((data[i]&0xffff0000)>>16)/10000.0f, (int)(data[i]&0x0000ffff)-100);
    return g;
}

void
WindowData::sync()
{
#if HASDBUS
    if (!winId())
        return;
    static const QString destination("org.kde.dsp.kwindeco");
    static const QString path("/DSPDecoAdaptor");
    static const QString interface("org.kde.dsp.deco");
    static const QString method("updateData");
    QDBusMessage msg = QDBusMessage::createMethodCall(destination, path, interface, method);
    msg << winId();
    QDBusConnection::sessionBus().send(msg);
#endif
}

WindowData
WindowData::memory(const quint32 wid, QObject *parent, const bool create)
{
    if (!wid || !parent)
        return WindowData();

    DataStructSharedMemory *m = parent->findChild<DataStructSharedMemory *>();
    if (!m)
        m = new DataStructSharedMemory(QString::number(wid), parent);

//    qDebug() << "WindowData::memory(const quint32 wid, QObject *parent, const bool create)";
    WindowData data(m);
    if (data.isAttached() || data.attach())
        return data;
    if (create && data.create())
        return data;
    return WindowData();
}
