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
#include "gfx.h"
#include "../config/settings.h"
#include "handlers.h"

using namespace DSP;

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
    if (dConf.tabs.regular || !dConf.uno.enabled || !dConf.tabs.safari)
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

//void
//Ops::updateGeoFromSender()
//{
//    QWidget *w = static_cast<QWidget *>(sender());
//    QResizeEvent e(w->size(), w->size());
//    qApp->sendEvent(w, &e);
//}

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
Ops::swap(int &t1, int &t2)
{
    const int tmp(t1);
    t1 = t2;
    t2 = tmp;
}
