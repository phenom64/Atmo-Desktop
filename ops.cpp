#include <QWidget>
#include <QTabBar>
#include <QMainWindow>
#include <QToolBar>
#include <QDebug>

#include "ops.h"

Ops::Ops()
{
}

QColor
Ops::mid(const QColor &c1, const QColor c2, int i1, int i2)
{
    const int i3 = i1+i2;
    int r,g,b,a;
    r = qMin(255,(i1*c1.red() + i2*c2.red())/i3);
    g = qMin(255,(i1*c1.green() + i2*c2.green())/i3);
    b = qMin(255,(i1*c1.blue() + i2*c2.blue())/i3);
    a = qMin(255,(i1*c1.alpha() + i2*c2.alpha())/i3);
    return QColor(r,g,b,a);
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
    if (!tabBar)
        return false;
    QMainWindow *mainWin = qobject_cast<QMainWindow *>(tabBar->window());
    if (!mainWin)
        return false;
    const bool s(tabBar->shape() == QTabBar::RoundedNorth || tabBar->shape() == QTabBar::TriangularNorth);
    if (!s)
        return false;
    const QPoint &checkPoint(tabBar->mapTo(mainWin, tabBar->rect().translated(1, -6).topLeft()));
    if (isOrInsideA<const QToolBar *>(mainWin->childAt(checkPoint)))
        return true;
    return false;
}
