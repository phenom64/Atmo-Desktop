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
#include "titlewidget.h"
#include <QAction>
#include <QToolBar>
#include <QPainter>
#include <QEvent>
#include "windowdata.h"
#include "xhandler.h"
#include <QX11Info>
#include "ops.h"
#include "macmenu.h"
#include <QLayout>
#include "toolbarhelpers.h"
#include "windowhelpers.h"
#include <QWidgetAction>
#include <QToolButton>
#if HASDBUS
#include <QDBusInterface>
#endif

using namespace NSE;

static const char *s_spacerName("NSE_TOOLBARSPACER");

TitleWidget::TitleWidget(QToolBar *parent)
    : QWidget(parent)
    , m_time(0)
    , m_toolBar(parent)
    , m_unembedQueued(false)
    , m_embedQueued(false)
    , m_action(0)
    , m_visibleCount(0)
    , m_logicalQueued(false)
{
    m_toolBar->installEventFilter(this);
    setContentsMargins(8, 0, 0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_NoMousePropagation);
#if HASDBUS
    if (BE::MacMenu::instance())
        connect(BE::MacMenu::instance(), SIGNAL(activeChanged()), this, SLOT(macMenuChanged()));
    static const QString service("com.syndromatic.atmo.kwindeco"),
            interface("com.syndromatic.atmo.deco"),
            path("/NSEDecoAdaptor");
    QDBusInterface *iface = new QDBusInterface(service, path, interface);
    iface->connection().connect(service, path, interface, "dataChanged", this, SLOT(dataChanged(QDBusMessage)));
#endif
    connect(parent, SIGNAL(topLevelChanged(bool)), this, SLOT(toolBarFloatingChagned(bool)), Qt::QueuedConnection);
    connect(parent, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(toolBarOrientationChagned(Qt::Orientation)));
    connect(parent, SIGNAL(movableChanged(bool)), this, SLOT(toolBarMovableChanged(bool)), Qt::QueuedConnection);
    connect(parent, SIGNAL(visibilityChanged(bool)), this, SLOT(toolBarVisibilityChanged(bool)), Qt::QueuedConnection);
//    connect(parent, SIGNAL(destroyed(QObject*)), instance(), SLOT(toolBarDeleted(QObject*)));
    hide();
}

void
TitleWidget::manage(QToolBar *toolBar)
{
    if (!toolBar->findChild<TitleWidget *>())
        new TitleWidget(toolBar);
}

bool
TitleWidget::isManaging(const QToolBar *toolBar)
{
    TitleWidget *tw = toolBar->findChild<TitleWidget *>();
    if (!tw)
        return false;
    return tw && tw->m_action && tw->m_action->isVisible();
}

void
TitleWidget::paintEvent(QPaintEvent *)
{
    QString title(window()->windowTitle());
    const QRect mappedRect(mapFrom(parentWidget(), QPoint()), parentWidget()->contentsRect().size());
    QPainter p(this);
    QFont f(p.font());
    const bool active(WindowHelpers::isActiveWindow(this));
    QPalette pal(palette());
    pal.setCurrentColorGroup(active?QPalette::Active:QPalette::Disabled);
    f.setBold(active);
//    f.setPointSize(f.pointSize()-1.5f);
    p.setFont(f);
    int align(Qt::AlignVCenter);
    switch (dConf.titlePos)
    {
    case Left: align |= Qt::AlignLeft; break;
    case Center: align |= Qt::AlignHCenter; break;
    case Right: align |= Qt::AlignRight; break;
    default: break;
    }
//    Render::renderShadow(dConf.toolbtn.shadow, rect(), &p, dConf.toolbtn.rnd, All, dConf.shadows.opacity);
#if QT_VERSION > 0x050000
//    align = Qt::AlignBottom|Qt::AlignHCenter;
//    QFont fb(p.font());
//    fb.setPointSize(fb.pointSize()-3);
//    fb.setBold(false);
//    QColor c(pal.color(foregroundRole()));
//    c.setAlpha(127);
//    p.setPen(c);
//    p.setFont(fb);
//    p.drawText(rect, Qt::AlignTop|Qt::AlignHCenter, qApp->applicationDisplayName());
//    p.setFont(f);
    if (!title.isEmpty())
        title.append(QString(" - "));
    title.append(qApp->applicationDisplayName());
#endif
    const QString &elidedTitle = p.fontMetrics().elidedText(title, Qt::ElideRight, rect().width()-20, Qt::TextShowMnemonic);
    QRect tr(p.fontMetrics().boundingRect(mappedRect, align, elidedTitle));
    if (tr.x() < 20)
        tr.moveLeft(20);
    style()->drawItemText(&p, tr, align, pal, true, elidedTitle, foregroundRole());
    if (dConf.deco.icon && !title.isEmpty())
    {
        const QPixmap icon(window()->windowIcon().pixmap(16, active?QIcon::Normal:QIcon::Disabled));
        QRect ir(QPoint(0, mappedRect.height()/2-qMax(1, icon.height())/2), icon.size());
        ir.moveRight(tr.left()-4);
        if (icon.width() && ir.right() >= icon.width())
            style()->drawItemPixmap(&p, ir, Qt::AlignVCenter|Qt::AlignRight, icon);
    }
    p.end();
}

void
TitleWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->accept();
//    QWidget::mouseDoubleClickEvent(e);
    const int clickSpeed = QX11Info::appTime()/*e->timestamp()*/-m_time;
    //sometimes if the window is inactive, and we click the titlebar in order
    //to move the window, instead for whatever reason we get a doubleclick event,
    //these mystical 2 clicks are usually closer together then any human (me)
    //is able to perform (I managed to do a doubleclick in 120 ms, not faster, so
    //you probably need some 30 years of lol if youre clicking faster then 75 ms)
    if (clickSpeed > 75 && clickSpeed < 300)
    {
        WindowData data = WindowData::memory(XHandler::windowId(window()), window());
        if (data && data.lock())
        {
            const uint deco = data->decoId;
            data.unlock();
            XHandler::doubleClickEvent(e->globalPos(), deco, e->button());
        }
        m_time = QX11Info::appTime()/*e->timestamp()*/;
    }
}

void
TitleWidget::mouseReleaseEvent(QMouseEvent *e)
{
    e->accept();
}

void
TitleWidget::mousePressEvent(QMouseEvent *e)
{
    e->accept();
//#if QT_VERSION >= 0x050000
//    m_time = e->timestamp();
//#else
    m_time = QX11Info::appTime();
//#endif
    if (e->button() == Qt::LeftButton)
    {
        XHandler::mwRes(e->pos(), e->globalPos(), XHandler::windowId(window()));
        return;
    }
    WindowData data = WindowData::memory(XHandler::windowId(window()), window());
    if (data && data.lock())
    {
        const uint deco = data->decoId;
        data.unlock();
        XHandler::pressEvent(e->globalPos(), deco, e->button());
    }
}

void
TitleWidget::wheelEvent(QWheelEvent *e)
{
    e->accept();
    WindowData data = WindowData::memory(XHandler::windowId(window()), window());
    if (data && data.lock())
    {
        const uint deco = data->decoId;
        data.unlock();
        XHandler::wheelEvent(deco, e->delta()>0);
    }
}

bool
TitleWidget::shouldEmbed()
{
    bool hasMacMenu(false);
#if HASDBUS
        hasMacMenu = BE::MacMenu::isActive();
#endif
    if (!hasMacMenu
            || dConf.app == Settings::SystemSettings
            || m_toolBar->isFloating()
            || !m_toolBar->isVisible()
            || !m_toolBar->isMovable()
            || !qobject_cast<QMainWindow *>(m_toolBar->parentWidget())
            || static_cast<QMainWindow *>(m_toolBar->parentWidget())->toolBarArea(m_toolBar) != Qt::TopToolBarArea
            || m_toolBar->width() != m_toolBar->parentWidget()->width()
            || m_toolBar->toolButtonStyle() != Qt::ToolButtonIconOnly
            || m_toolBar->iconSize().height() != 16)
        return false;

#if QT_VERSION >= 0x050000
    const QList<QWidget *> kids(m_toolBar->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
#else
    const QList<QWidget *> kids(m_toolBar->findChildren<QWidget *>());
#endif
    for (int i = 0; i < kids.count(); ++i)
    {
        QWidget *w(kids.at(i));
        if (qobject_cast<TitleWidget *>(w) || w->width() < 128)
            continue;
        if (w->isVisible() && !qobject_cast<QToolButton *>(w))
            return false;
    }
    return true;
}

void
TitleWidget::toolBarOrientationChagned(const Qt::Orientation o)
{
    QToolBar *toolBar = static_cast<QToolBar *>(sender());
//    qDebug() << "NSE::TitleWidget::toolBarOrientationChagned";
    ToolbarHelpers::adjustMarginsLater(toolBar);
}

void
TitleWidget::toolBarFloatingChagned(const bool floating)
{
    QToolBar *toolBar = static_cast<QToolBar *>(sender());
//    qDebug() << "NSE::TitleWidget::toolBarFloatingChagned" << floating;
    if (floating)
        unembedLater();
    ToolbarHelpers::adjustMarginsLater(toolBar);
}

void
TitleWidget::toolBarMovableChanged(const bool movable)
{
    QToolBar *toolBar = static_cast<QToolBar *>(sender());
//    qDebug() << "NSE::TitleWidget::toolBarMovableChanged" << movable;
    if (!movable)
        unembedLater();
    else
        embedLater();
    ToolbarHelpers::adjustMarginsLater(toolBar);
    WindowHelpers::updateWindowDataLater(toolBar->window());
}

void
TitleWidget::toolBarVisibilityChanged(const bool visible)
{
//    QToolBar *toolBar = static_cast<QToolBar *>(sender());
//    qDebug() << "NSE::TitleWidget::toolBarVisibilityChanged";
    if (!m_logicalQueued)
    {
        m_logicalQueued =true;
        QMetaObject::invokeMethod(this, "logicalToggle", Qt::QueuedConnection);
    }
}

void
TitleWidget::logicalToggle()
{
//    qDebug() << "NSE::TitleWidget::logicalToggle()" << m_toolBar->isVisible() << shouldEmbed();
    m_logicalQueued = false;
    if (shouldEmbed())
        embedLater();
    else
        unembedLater();
    WindowHelpers::updateWindowDataLater(m_toolBar->parentWidget());
}


void
TitleWidget::embedLater()
{
    if (!m_embedQueued)
    {
        m_embedQueued = true;
        QMetaObject::invokeMethod(this, "embed", Qt::QueuedConnection);
    }
}

void
TitleWidget::embed()
{
    m_embedQueued = false;
//    qDebug() << "NSE::TitleWidget::embed()";

    if (!shouldEmbed() || m_toolBar->layout()->count()<3)
        return;

    WindowData data = WindowData::memory(XHandler::windowId(m_toolBar->window()), m_toolBar->window());
    if (!data)
        return;

    m_toolBar->removeEventFilter(this);
    QAction *a(0);
    if (dConf.titlePos == TitleWidget::Center)
    {
        if (QWidget *w = getCenterWidget())
            a = m_toolBar->actionAt(w->geometry().center());
        else
            a = m_toolBar->actions().at(1);
    }
    else if (dConf.titlePos == TitleWidget::Left)
        a = m_toolBar->actions().at(1);

    bool isAdded(false);
    if (QWidgetAction *action = qobject_cast<QWidgetAction *>(m_toolBar->actionAt(geometry().center())))
    if (action == m_action)
    {
        isAdded = true;
        m_toolBar->removeAction(m_action);
        m_toolBar->insertAction(a, m_action);
    }
    if (!isAdded)
        m_action = m_toolBar->insertWidget(a, this);
    if (m_action)
        m_action->setVisible(true);
    bool dataChanged(false);
    if (data.lock())
    {
        ToolbarHelpers::fixSpacerLater(m_toolBar, data->leftEmbedSize);

        ToolbarHelpers::adjustMarginsLater(m_toolBar);
        if (!data->embedButtons)
        {
            data->embedButtons = true;
            dataChanged = true;
        }
        if (data->titleHeight != 6)
        {
            data->titleHeight = 6;
            dataChanged = true;
        }
        data.unlock();
    }
    if (dataChanged)
        data.sync();
    m_toolBar->installEventFilter(this);
}

void
TitleWidget::unembedLater()
{
    if (!m_unembedQueued)
    {
        m_unembedQueued = true;
        QMetaObject::invokeMethod(this, "unembed", Qt::QueuedConnection);
    }
}

void
TitleWidget::unembed()
{
    m_unembedQueued = false;
    QMainWindow *win = qobject_cast<QMainWindow *>(m_toolBar->parentWidget());
//    qDebug() << "NSE::TitleWidget::unembed" << win;
    if (!win)
    {
        deleteLater();
        return;
    }
    WindowData data = WindowData::memory(XHandler::windowId(win), win);
    if (data && data.lock())
    {
        data->embedButtons = false;
        data->titleHeight = 25;
        data.unlock();
        data.sync();
    }
    if (m_action)
        m_action->setVisible(false);
    if (!m_toolBar->isMovable())
        ToolbarHelpers::fixSpacerLater(m_toolBar);
}

int
TitleWidget::visibleKids()
{
    const QObjectList kids = children();
    int count(0);
    for (int i = 0; i < kids.size(); ++i)
    {
        const QObject *o = kids.at(i);
        if (!o->isWidgetType())
            continue;
        if (static_cast<const QWidget *>(o)->isVisibleTo(m_toolBar))
            ++count;
    }
    return count;
}

bool
TitleWidget::eventFilter(QObject *o, QEvent *e)
{
    if (o != static_cast<QObject *>(m_toolBar) || !e)
        return false;

    switch (e->type())
    {
    case QEvent::ShowToParent:
    case QEvent::HideToParent:
    {
        const int newCount = visibleKids();
        if (m_visibleCount != newCount)
        {
            embedLater();
            m_visibleCount = newCount;
        }
        return false;
    }
    case QEvent::WindowTitleChange:
    {
        update();
        return false;
    }
    default: return false;
    }
    return false;
}

QWidget
*TitleWidget::getCenterWidget()
{
    QLayout *l = m_toolBar->layout();
    const int center(qMin(l->count()-1, qCeil(l->count()/2.0f)+2));
    QLayoutItem *centerItem = l->itemAt(center);
    if (centerItem->spacerItem())
        centerItem = l->itemAt(center+1);
    QWidget *w = centerItem->widget();

    for (int i = center; i; --i)
    {
        QWidget *wi = l->itemAt(i)->widget();
        if (wi->isHidden())
            continue;
        if (wi->inherits("QToolBarSeparator") && qobject_cast<QToolButton *>(w))
            break;
        else if (qobject_cast<QToolButton *>(wi))
            w = wi;
    }
    return w;
}

#if HASDBUS
void
TitleWidget::macMenuChanged()
{
    const QList<QWidget *> allWidgets(qApp->allWidgets());
    for (int i = 0; i < allWidgets.count(); ++i)
        if (QMainWindow *win = qobject_cast<QMainWindow *>(allWidgets.at(i)))
        {
            const QList<QToolBar *> toolBars = win->findChildren<QToolBar *>();
            for (int t = 0; t < toolBars.count(); ++t)
            {
                if (toolBars.at(i) == m_toolBar)
                {
                    if (BE::MacMenu::isActive())
                        embedLater();
                    else
                        unembedLater();
                }
            }
        }
}

void
TitleWidget::dataChanged(QDBusMessage msg)
{
    uint winId = msg.arguments().at(0).toUInt();
    QList<QWidget *> widgets = qApp->allWidgets();
    for (int i = 0; i < widgets.count(); ++i)
    {
        QWidget *w(widgets.at(i));
        if (w->isWindow() && XHandler::windowId(w) == winId)
        {
            QList<QToolBar *> toolBars(w->findChildren<QToolBar *>());
            for (int i = 0; i < toolBars.count(); ++i)
            {
                if (toolBars.at(i) == m_toolBar && shouldEmbed())
                    embedLater();
            }
            break;
        }
    }
}
#endif
