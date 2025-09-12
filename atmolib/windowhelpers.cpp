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
#include "windowhelpers.h"
#include "windowdata.h"
#include "color.h"
#include "ops.h"
#include "macmenu.h"
#include "xhandler.h"
#include "fx.h"
#include "titlewidget.h"
#include <QMainWindow>
#include <QToolBar>
#include <QMenuBar>
#include <QImage>
#include <QPainter>

#if HASDBUS
#include <QDBusInterface>
#endif

using namespace NSE;

WindowHelpers *WindowHelpers::s_instance(0);

WindowHelpers
*WindowHelpers::instance()
{
    if (!s_instance)
        s_instance = new WindowHelpers();
    return s_instance;
}

WindowHelpers::WindowHelpers(QObject *parent)
    : QObject(parent)
{
#if HASDBUS
//    static const QString service("com.syndromatic.atmo.kwindeco"),
//            interface("com.syndromatic.atmo.deco"),
//            path("/NSEDecoAdaptor");
//    QDBusInterface *iface = new QDBusInterface(service, path, interface);
//    iface->connection().connect(service, path, interface, "dataChanged", this, SLOT(dataChanged(QDBusMessage)));
#endif
}

bool
WindowHelpers::inUno(const QWidget *w)
{
    return w&&w->mapTo(w->window(), QPoint()).y() < unoHeight(w->window());
}

bool
WindowHelpers::scheduleWindow(const qulonglong w)
{
    if (instance()->m_scheduled.contains(w))
        return false;
    instance()->m_scheduled << w;
    return true;
}

void
WindowHelpers::updateWindowDataLater(QWidget *win)
{
    const qulonglong window = (qulonglong)win;
//    qDebug() << "WindowHelpers::updateWindowDataLater" << window;
    if (scheduleWindow(window))
        QMetaObject::invokeMethod(instance(), "updateWindowData", Qt::QueuedConnection, Q_ARG(qulonglong, window));
}

void
WindowHelpers::updateWindowData(qulonglong window)
{
    m_scheduled.removeOne(window);
    QWidget *win = Ops::getChild<QWidget *>(window);
    if (!win || !win->isWindow())
        return;

    WindowData data = WindowData::memory(XHandler::windowId(win), win, true);
    if (data && data.lock())
    {
        QPalette pal(win->palette());
        if (!Color::contrast(pal.color(win->backgroundRole()), pal.color(win->foregroundRole()))) //im looking at you spotify
            pal = QApplication::palette();
        if (dConf.uno.enabled)
        {
            bool separator(true);
            const unsigned int height = getHeadHeight(win, separator, data->titleHeight);
            if (dConf.differentInactive)
            {
                if (isActiveWindow(win))
                {
                    pal.setColor(QPalette::Active, win->backgroundRole(), Color::mid(pal.color(win->backgroundRole()), Qt::black, 10, 1));
                    pal.setColor(QPalette::Active, win->foregroundRole(), Color::mid(pal.color(win->foregroundRole()), Qt::black, 10, 1));
                    pal.setCurrentColorGroup(QPalette::Active);
                }
                else
                {
                    pal.setColor(QPalette::Inactive, win->backgroundRole(), Color::mid(pal.color(win->backgroundRole()), Qt::white, 20, 1));
                    pal.setColor(QPalette::Inactive, win->foregroundRole(), Color::mid(pal.color(win->foregroundRole()), Qt::white, 20, 1));
                    pal.setCurrentColorGroup(QPalette::Inactive);
                }
            }
            int width(0);
            unoBg(win, width, height, pal, data->imageData);
            data->imageWidth = width;
            data->imageHeight = height;
            data->separator = separator;
            data->unoHeight = height;
        }
        else
        {
            QImage img(data->imageData, GFX::noise(true).width(), GFX::noise(true).height(), QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::transparent);
            QPainter p(&img);
            p.drawTiledPixmap(img.rect(), GFX::noise(true));
            p.end();
            data->imageWidth = img.width();
            data->imageHeight = img.height();
            data->separator = false;
            data.setWindowGradient(dConf.windows.gradient);
        }
        data->winIcon           = dConf.deco.icon;
        data->contAware         = dConf.uno.enabled&&dConf.uno.contAware;
        data->uno               = dConf.uno.enabled;
        data->hor               = dConf.uno.enabled?dConf.uno.hor:dConf.windows.hor;
        data->opacity           = XHandler::opacity();
        data->buttons           = dConf.deco.buttons;
        data->frame             = dConf.deco.frameSize;
        data->shadowOpacity     = dConf.shadows.opacity;
        data->illumination      = dConf.shadows.illumination;
        data->textBevelOpacity  = dConf.shadows.onTextOpacity;
        data->followDecoShadow  = dConf.toolbtn.shadow;
        data->closeColor        = dConf.deco.close;
        data->maxColor          = dConf.deco.max;
        data->minColor          = dConf.deco.min;
        data->fgColor           = pal.color(win->foregroundRole()).rgba();
        data->bgColor           = pal.color(win->backgroundRole()).rgba();
//        if (win->findChild<TitleWidget *>())
//            data->titleHeight = 6;
        data.setButtonGradient(dConf.toolbtn.gradient);
        data.unlock();
        QMetaObject::invokeMethod(win, "update", Qt::QueuedConnection);
        emit instance()->windowDataChanged(win);
        data.sync();
    }
}


unsigned int
WindowHelpers::getHeadHeight(QWidget *win, bool &separator, const int dataHeight)
{
    int h = dataHeight;
    if (!h)
        h = 25;
    if (!h)
    {
        separator = false;
        return 0;
    }
    if (!dConf.uno.enabled)
    {
        separator = false;
        if (h)
            return h;
        return 0;
    }
    int height(0);
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(win))
    {
        if (QMenuBar *menuBar = mw->findChild<QMenuBar *>())
#if HASDBUS
            if (!BE::MacMenu::isActive() && !BE::MacMenu::manages(menuBar))
#endif
        {
            if (menuBar->isVisible())
                height += menuBar->height();
        }

        QList<QToolBar *> tbs(mw->findChildren<QToolBar *>());
        for (int i = 0; i<tbs.count(); ++i)
        {
            QToolBar *tb(tbs.at(i));
            if (!tb->isFloating() && tb->isVisible())
                if (!tb->findChild<QTabBar *>()) //ah eiskalt...
                    if (mw->toolBarArea(tb) == Qt::TopToolBarArea)
                    {
                        if (tb->geometry().bottom() > height)
                            height = tb->geometry().bottom()+1;
                        separator = false;
                    }
        }
    }
    QList<QTabBar *> tabbars = win->findChildren<QTabBar *>();
    for (int i = 0; i < tabbars.count(); ++i)
    {
        const QTabBar *tb(tabbars.at(i));
        if (tb && tb->isVisible() && Ops::isUnoTabBar(tb))
        {
            separator = false;
            const int y(tb->mapTo(win, tb->rect().bottomLeft()).y()+1);
            if (y > height)
                height = y;
        }
    }
    instance()->m_uno.insert(win, height);
    return height+h;
}

int
WindowHelpers::unoHeight(QWidget *win, bool includeClientPart, bool includeTitleBar)
{
    int height(0);
    if (includeClientPart)
        height += instance()->m_uno.value(win, 0);
    if (!includeTitleBar)
        return height;
    WindowData data = WindowData::memory(XHandler::windowId(win), win);
    if (data && data.lock())
    {
//        WindowDataLocker locker(data);
        int h = height + data->titleHeight;
        data.unlock();
        return h;
    }
    return height;
}

static const char *s_active("NSE_activeWindow");

bool
WindowHelpers::isActiveWindow(const QWidget *w)
{
    if (!w)
        return true;
    QWidget *win = w->window();
    return win->isActiveWindow();
//    if (!win->property(s_active).isValid())
//        return true;
//    return (win->property(s_active).toBool());
}

#if HASDBUS
void
WindowHelpers::dataChanged(QDBusMessage msg)
{
//    uint winId = msg.arguments().at(0).toUInt();
//    QList<QWidget *> widgets = qApp->allWidgets();
//    for (int i = 0; i < widgets.count(); ++i)
//    {
//        QWidget *w(widgets.at(i));
//        if (w->isWindow() && XHandler::windowId(w) == winId)
//        {
//            WindowData data = WindowData::memory(winId, w);
//            if (data && data.lock())
//            {
//                w->setProperty(s_active, data->isActiveWin);
//                data.unlock();
//            }
//            updateWindowDataLater(w);
//            break;
//        }
//    }
}
#endif

void
WindowHelpers::unoBg(QWidget *win, int &w, int h, const QPalette &pal, uchar *data)
{
    if (!data || !win || !h)
        return;
    const bool hor(dConf.uno.hor);
    QColor bc(pal.color(win->backgroundRole()));
    if (dConf.uno.tint.first.isValid())
        bc = Color::mid(bc, dConf.uno.tint.first, 100-dConf.uno.tint.second, dConf.uno.tint.second);
    QBrush b(bc);
    if (!dConf.uno.gradient.isEmpty())
    {
        QLinearGradient lg(0, 0, hor?win->width():0, hor?0:h);
        if (dConf.differentInactive && !isActiveWindow(win))
            lg.setStops(QGradientStops()
                        << NSE::Settings::pairToStop(NSE::GradientStop(0.0f, 0), bc)
                        << NSE::Settings::pairToStop(NSE::GradientStop(1.0f, 0), bc));
        else
            lg.setStops(NSE::Settings::gradientStops(dConf.uno.gradient, bc));
        b = QBrush(lg);
    }
    const unsigned int n(dConf.uno.noise);
    w = (hor?win->width():(n?GFX::noise().width():1));
    QImage img(data, w, h, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter pt(&img);
    pt.fillRect(img.rect(), b);
    if (n)
    {
        const QPixmap noise = FX::mid(GFX::noise(), b, n, 100-n, img.size());
//        QPixmap noise(GFX::noise().size());
//        noise.fill(Qt::transparent);
//        QPainter ptt(&noise);
//        ptt.drawTiledPixmap(noise.rect(), GFX::noise());
//        ptt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
//        ptt.fillRect(noise.rect(), QColor(0, 0, 0, n*2.55f));
//        ptt.end();
//        pt.setCompositionMode(QPainter::CompositionMode_Overlay);
        pt.drawTiledPixmap(img.rect(), noise);
    }
    if (XHandler::opacity() < 0xff)
    {
        pt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pt.fillRect(img.rect(), QColor(0, 0, 0, XHandler::opacity()));
    }
    pt.end();
}
