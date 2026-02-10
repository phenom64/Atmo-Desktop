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


#include "menubar.h"
#include "kwinclient2.h"
#include "../atmolib/ops.h"

#if !defined(ATMO_USE_KDECORATION3)
#include <KWindowInfo>
#endif

#include <QEvent>
#include <QMouseEvent>
#include <QTimer>


#include <QDBusConnection>
#include <dbusmenuimporter.h>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

#include <QPixmap>

#include <QSettings>
#include <QDir>
#include <QWindow>
#include <qmath.h>
#include <QMenu>
#include <QTimer>
#include <QRect>
#include <QPainter>

using namespace NSE;

MenuBar::MenuBar(Deco *deco, const QString &service, const QString &path)
    : KDecoration2::DecorationButtonGroup(deco)
    , deco(deco)
    , importer(new DBusMenuImporter(service, path, ASYNCHRONOUS, this))
    , timer(new QTimer(this))
    , prevHovered(0)
    , active(0)
{
    setSpacing(Ops::dpiScaled(nullptr, 8));
    connect(importer, &DBusMenuImporter::menuUpdated, this, &MenuBar::menuUpdated);
//    connect(importer, &DBusMenuImporter::menuReadyToBeShown, this, &MenuBar::menuUpdated);
    QMetaObject::invokeMethod(importer, "updateMenu", Qt::QueuedConnection);
    connect(timer, &QTimer::timeout, this, &MenuBar::pollMouse);
}

void
MenuBar::menuUpdated()
{
    const QList<QAction *> list = importer->menu()->actions();
    while (list.count() < buttons().count())
        removeButton(buttons().at(buttons().count()-1));
    while (list.count() > buttons().count())
        addButton(new MenuBarItem(this, buttons().count()));
    if (active >= buttons().count())
        active = 0;
    QMetaObject::invokeMethod(deco, "updateLayout", Qt::QueuedConnection);
}

void
MenuBar::paint(QPainter *painter, const AtmoDecoRect &repaintArea)
{
    const QPen pen = painter->pen();
    const QBrush brush = painter->brush();

    painter->setPen(QColor(0,0,0,63));
    QLinearGradient grad(0, geometry().top(), 0, geometry().height());
    grad.setStops(QGradientStops() << QGradientStop(0.0, QColor(0,0,0,31)) << QGradientStop(1.0, Qt::transparent));
    painter->setBrush(grad);
    const qreal radius = Ops::dpiScaled(nullptr, 2);
    painter->drawRoundedRect(geometry().translated(0.5, 0.5).adjusted(0,0,-1,-1), radius, radius);
    painter->setPen(pen);
    painter->setBrush(brush);

    KDecoration2::DecorationButtonGroup::paint(painter, repaintArea);
}

static void findActsRecursive(QList<QAction *> &acts, QMenu *menu, const QString matchText = QString())
{
    for (int i = 0; i < menu->actions().count(); ++i)
    {
        QAction *a = menu->actions().at(i);
        if (matchText == a->text() && !matchText.isEmpty())
        {
            acts << a;
            return;
        }
        else if (matchText.isEmpty())
            acts << a;
        if (a->menu())
            findActsRecursive(acts, a->menu(), matchText);
    }
}

static QAction *fromText(const QString text, QMenu *menu)
{
    QList<QAction *> acts;
    findActsRecursive(acts, menu, text);
    if (!acts.isEmpty())
        return acts.first();
    return 0;
}

QAction
*MenuBar::actionFromText(const QString &text) const
{
    return fromText(text, importer->menu());
}

MenuBar::~MenuBar()
{
    qDebug() << "MenuBar::~MenuBar()";
}

void
MenuBar::pollMouse()
{
    for (int i = 0; i < buttons().count(); ++i)
    {
        MenuBarItem *item = static_cast<MenuBarItem *>(buttons().at(i).data());
        if (item->topLevelGeo().contains(QCursor::pos()))
        {
            if (prevHovered != item)
            {
                prevHovered = item;
                prevHovered->hoverEnter();
                return;
            }
        }
    }
}

void
MenuBar::startMousePolling()
{
    timer->start(50);
}

void
MenuBar::stopMousePolling()
{
    timer->stop();
}

void
MenuBar::updateMenu()
{
    importer->updateMenu();
}

bool
MenuBar::hasShownMenues() const
{
    for (int i = 0; i < buttons().count(); ++i)
    {
        MenuBarItem *item = static_cast<MenuBarItem *>(buttons().at(i).data());
        if (QMenu *mn = item->menu())
            if (mn->isVisible())
                return true;
    }
    return false;
}

void
MenuBar::hideAllMenues()
{
    for (int i = 0; i < buttons().count(); ++i)
    {
        MenuBarItem *item = static_cast<MenuBarItem *>(buttons().at(i).data());
        if (QMenu *mn = item->menu())
            if (mn->isVisible())
                mn->hide();
    }
}

MenuBarItem::MenuBarItem(MenuBar *menu, int index)
    : KDecoration2::DecorationButton(KDecoration2::DecorationButtonType::Custom, menu->deco, menu->deco)
    , m(menu)
    , hasGeo(false)
    , idx(index)
{
    connect(this, &MenuBarItem::clicked, this, &MenuBarItem::click);
    if (this->menu())
    {
        connect(this->menu(), &QMenu::aboutToHide, this, [this](){m->stopMousePolling();});
        connect(this->menu(), &QMenu::aboutToShow, this, [this](){m->updateMenu();});
    }
}

MenuBarItem::~MenuBarItem()
{
    qDebug() << "MenuBarItem::~MenuBarItem()";
}

QString
MenuBarItem::text() const
{
    if (QAction *a = action())
    {
        if (a->isSeparator())
            return " | ";
        return a->text();
    }
    return QString();
}

QAction
*MenuBarItem::action() const
{
    if (idx < m->importer->menu()->actions().count())
        return m->importer->menu()->actions().at(idx);
    return 0;
}

void
MenuBarItem::paint(QPainter *painter, const AtmoDecoRect &repaintArea)
{
    if (!hasGeo)
    {
        const int textPad = Ops::dpiScaled(nullptr, 2);
        QRect geo = painter->fontMetrics().boundingRect(QRect(), Qt::TextHideMnemonic, text()).adjusted(-textPad, 0, textPad, 0);
        geo.setHeight(m->deco->titleBar().height() - Ops::dpiScaled(nullptr, 4));
        setGeometry(geo);
        hasGeo = true;
        QMetaObject::invokeMethod(m->deco, "updateLayout", Qt::QueuedConnection);
    }
    const QPen pen = painter->pen();
    if (isHovered())
    {
        QColor bg = m->deco->window()->palette().color(QPalette::WindowText);
        bg.setAlpha(31);
        const QBrush brush = painter->brush();
        painter->setBrush(bg);
        painter->setPen(Qt::NoPen);
        const qreal radius = Ops::dpiScaled(nullptr, 2);
        painter->drawRoundedRect(geometry(), radius, radius);
        painter->setBrush(brush);
    }
    if (m->deco->window()->isActive() && m->deco->m_textBevOpacity)
    {
        const int rgb = m->deco->m_isDark ? 0 : 255;
        const QColor bevel(rgb,rgb,rgb, m->deco->m_textBevOpacity);
        painter->setPen(bevel);
        painter->drawText(geometry().translated(0, Ops::dpiScaled(nullptr, 1)), Qt::TextHideMnemonic|Qt::AlignCenter, text());
    }
    painter->setPen(m->deco->m_fg);
    painter->drawText(geometry(), Qt::TextHideMnemonic|Qt::AlignCenter, text());
    painter->setPen(pen);
}

void
MenuBarItem::hoverEnter()
{
    if (m->hasShownMenues())
    {
        m->hideAllMenues();
        click();
    }
}

void
MenuBarItem::hoverEnterEvent(QHoverEvent *event)
{
    KDecoration2::DecorationButton::hoverEnterEvent(event);
//    QMetaObject::invokeMethod(m, "updateMenu", Qt::QueuedConnection);
    hoverEnter();
}

void
MenuBarItem::hoverLeave()
{
//    if (!m->hasShownMenues())
//        m->stopMousePolling();

//    update();
}

void
MenuBarItem::hoverLeaveEvent(QHoverEvent *event)
{
    KDecoration2::DecorationButton::hoverLeaveEvent(event);
    hoverLeave();
}

QMenu
*MenuBarItem::menu() const
{
    if (QAction *a = action())
        if (QMenu *m = a->menu())
            return m;
    return 0;
}

QRect
MenuBarItem::topLevelGeo() const
{
#if defined(ATMO_USE_KDECORATION3)
    const QPointF topLeft = m->deco->window()->position() + geometry().topLeft();
    return QRect(topLeft.toPoint(), geometry().size().toSize());
#else
    KWindowInfo info(m->deco->winId(), NET::WMGeometry);
    return QRect(info.geometry().topLeft() + geometry().topLeft(), info.geometry().topLeft() + geometry().bottomRight() + QPoint(1, 1));
#endif
}

void
MenuBarItem::click()
{
    QMenu *mn = menu();
    if (!mn)
        return;
    QPoint pos = topLevelGeo().bottomLeft();
    m->startMousePolling();
    mn->popup(pos);
}
