#include <QWidget>
#include <QTabBar>
#include <QMainWindow>
#include <QToolBar>
#include <QDebug>
#include <QColor>
#include <QMap>
#include <QPainter>
#include <QResizeEvent>
#include <QApplication>
#include <QAbstractScrollArea>
#include <QToolButton>
#include <QStyle>
#include <QStyleOptionToolButton>
#include <QTimer>
#include <QMenuBar>
#include <QLabel>
#include <QToolBox>

#include "ops.h"
#include "xhandler.h"
#include "macros.h"
#include "color.h"
#include "render.h"
#include "../config/settings.h"
#include "handlers.h"

using namespace DSP;

Q_DECL_EXPORT Ops *Ops::s_instance = 0;
Q_DECL_EXPORT QQueue<QueueItem> Ops::m_laterQueue;

Ops
*Ops::instance()
{
    if (!s_instance)
        s_instance = new Ops();
    return s_instance;
}

void
Ops::deleteInstance()
{
    if (s_instance)
    {
        delete s_instance;
        s_instance = 0;
    }
}

QWidget
*Ops::window(QWidget *w)
{
    QWidget *widget = w;
    while (widget->parentWidget())
        widget = widget->parentWidget();
    return widget;
}

bool
Ops::isSafariTabBar(const QTabBar *tabBar)
{
    if (!dConf.uno.enabled || !dConf.tabs.safari)
        return false;
    if (!tabBar || !(tabBar->shape() == QTabBar::RoundedNorth || tabBar->shape() == QTabBar::TriangularNorth) || !tabBar->documentMode())
        return false;

    QWidget *win(tabBar->window());
    if (tabBar->mapTo(win, QPoint(0, 0)).y() <= 1)
        return true;
    const QPoint &checkPoint(tabBar->mapTo(win, QPoint(1, -1)));
    return isOrInsideA<const QToolBar *>(win->childAt(checkPoint));
}

QPalette::ColorRole
Ops::opposingRole(const QPalette::ColorRole &role)
{
    switch (role)
    {
    case QPalette::Window: return QPalette::WindowText;
    case QPalette::WindowText: return QPalette::Window;
    case QPalette::AlternateBase:
    case QPalette::Base: return QPalette::Text;
    case QPalette::Text: return QPalette::Base;
    case QPalette::ToolTipBase: return QPalette::ToolTipText;
    case QPalette::ToolTipText: return QPalette::ToolTipBase;
    case QPalette::Button: return QPalette::ButtonText;
    case QPalette::ButtonText: return QPalette::Button;
    case QPalette::Highlight: return QPalette::HighlightedText;
    case QPalette::HighlightedText: return QPalette::Highlight;
    default: return QPalette::NoRole;
    }
}

void
Ops::updateGeoFromSender()
{
    QWidget *w = static_cast<QWidget *>(sender());
    QResizeEvent e(w->size(), w->size());
    qApp->sendEvent(w, &e);
}

QPalette::ColorRole
Ops::bgRole(const QWidget *w, const QPalette::ColorRole fallBack)
{
    if (!w)
        return fallBack;
    if (const QAbstractScrollArea *area = qobject_cast<const QAbstractScrollArea *>(w))
        if (area->viewport()->autoFillBackground() && area->viewport()->palette().color(area->viewport()->backgroundRole()).alpha() == 0xff)
            return area->viewport()->backgroundRole();
    return w->backgroundRole();
}

QPalette::ColorRole
Ops::fgRole(const QWidget *w, const QPalette::ColorRole fallBack)
{
    if (!w)
        return fallBack;
    if (const QAbstractScrollArea *area = qobject_cast<const QAbstractScrollArea *>(w))
        if (area->viewport()->autoFillBackground() && area->viewport()->palette().color(area->viewport()->foregroundRole()).alpha() == 0xff)
            return area->viewport()->foregroundRole();
    return w->foregroundRole();
}

void
Ops::callLater(QWidget *w, Function func, int time)
{
    QueueItem i;
    i.w = w;
    i.func = func;
    m_laterQueue.enqueue(i);
    QTimer::singleShot(time, instance(), SLOT(later()));
}

void
Ops::later()
{
    QueueItem i = m_laterQueue.dequeue();
    QWidget *w = i.w;
    Function func = i.func;
    (w->*func)();
}

bool
Ops::hasMenu(const QToolButton *tb, const QStyleOptionToolButton *stb)
{
#define QSTB QStyleOptionToolButton
    if (stb && stb->features & (/*QSTB::Menu|QSTB::HasMenu*/QSTB::MenuButtonPopup))
        return true;
#undef QSTB
    if (tb && (/*tb->menu()||*/tb->popupMode()==QToolButton::MenuButtonPopup))
        return true;
    return false;
}

void
Ops::toolButtonData(const QToolButton *tbtn, bool &nextsel, bool &prevsel, bool &isintop, Sides &sides)
{
    if (!tbtn)
        return;
    const QToolBar *bar = qobject_cast<const QToolBar *>(tbtn->parentWidget());
    if (!bar)
        return;

    const QList<QAction *> actions(bar->actions());
    int action(-1);
    while (++action < actions.count())
        if (bar->widgetForAction(actions.at(action)) == tbtn)
            break;

    sides = Handlers::ToolBar::sides(tbtn);
    if (!(sides & Left)||!(sides & Top))
    {
        if (const QToolButton *prev = qobject_cast<const QToolButton *>(bar->widgetForAction(actions.at(action-1))))
            prevsel = prev->isChecked();
    }
    if (!(sides & Right)||!(sides & Bottom))
    {
        if (const QToolButton *next = qobject_cast<const QToolButton *>(bar->widgetForAction(actions.at(action+1))))
            nextsel = next->isChecked();
    }
    if (tbtn->isChecked())
        nextsel = true;
    if (const QMainWindow *win = qobject_cast<const QMainWindow *>(bar->parentWidget()))
        if (win->toolBarArea(const_cast<QToolBar *>(bar)) == Qt::TopToolBarArea)
            isintop = true;
}

void
Ops::swap(int &t1, int &t2)
{
    const int tmp(t1);
    t1 = t2;
    t2 = tmp;
}
