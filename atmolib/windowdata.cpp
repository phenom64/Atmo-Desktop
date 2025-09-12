/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**************************************************************************
*   Based on styleproject, Copyright (C) 2013 by Robert Metsaranta        *
*   therealestrob@gmail.com                                              *
***************************************************************************/
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

using namespace NSE;

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
    static const QString destination("com.syndromatic.atmo.kwindeco");
    static const QString path("/NSEDecoAdaptor");
    static const QString interface("com.syndromatic.atmo.deco");
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
