#include <QWidget>
#include <QTabBar>
#include <QMainWindow>
#include <QToolBar>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QColor>
#include <QMap>
#include <QPainter>

#include "ops.h"

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

int
Ops::luminosity(const QColor &c)
{
    int r, g, b, a;
    c.getRgb(&r, &g, &b, &a);
    return (r*299 + g*587 + b*114)/1000;
}

bool
Ops::contrast(const QColor &c1, const QColor &c2)
{
    int lum1 = luminosity(c1), lum2 = luminosity(c2);
    if (qAbs(lum2-lum1)<125)
        return false;

    int r(qAbs(c1.red()-c2.red())),
            g(qAbs(c1.green()-c2.green())),
            b(qAbs(c1.blue()-c2.blue()));
    return (r+g+b>500);
}

void
Ops::setValue(const int value, QColor &c)
{
    c.setHsv(c.hue(), c.saturation(), qBound(0, value, 255), c.alpha());
}

static QMap<QColor, QColor> contrastMap[2];

void
Ops::ensureContrast(QColor &c1, QColor &c2)
{
    if (contrast(c1, c2))
        return;
    QColor dark = c1;
    QColor light = c2;
    bool inv(false);
    if (luminosity(c2)<luminosity(c1))
    {
        dark = c2;
        light = c1;
        inv = true;
    }
    while (!contrast(dark, light))
    {
        setValue(dark.value()-1, dark);
        setValue(light.value()+1, light);
    }
    c1 = inv ? light : dark;
    c2 = inv ? dark : light;
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
    QPoint topLeft = tabBar->mapTo(mainWin, tabBar->rect().topLeft());
    QRect winRect = mainWin->rect();
    QRect tabBarRect = QRect(topLeft, tabBar->size());
    if (tabBarRect.top() <= winRect.top())
        return true;
    const QPoint &checkPoint(tabBar->mapTo(mainWin, tabBar->rect().translated(1, -6).topLeft()));
    if (isOrInsideA<const QToolBar *>(mainWin->childAt(checkPoint)))
        return true;
    return false;
}

static QDBusMessage methodCall(const QString &method)
{
    return QDBusMessage::createMethodCall("org.kde.kwin", "/StyleProjectFactory", "org.kde.DBus.StyleProjectFactory", method);
}

void
Ops::updateWindow(WId window)
{
    QDBusMessage msg = methodCall("update");
    msg << QVariant::fromValue((unsigned int)window);
    QDBusConnection::sessionBus().send(msg);
}

void
Ops::drawCheckMark(QPainter *p, const QColor &c, const QRect &r)
{
    p->save();
    p->translate(r.topLeft());

    const int size = qMin(r.width(), r.height());
    const int third = size/3, thirds = third*2, sixth=third/2;
    const int points[] = { third,third+sixth, third+sixth,thirds+sixth, thirds+sixth,third-sixth };

    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(QPen(c, third*0.66f));
    p->setBrush(Qt::NoBrush);

    QPainterPath path;

    path.addPolygon(QPolygon(3, points));
    p->drawPath(path);
    p->restore();
}

void
Ops::drawArrow(QPainter *p, const QColor &c, const QRect &r, const Direction &d)
{
    p->save();
    p->translate(r.topLeft());
    p->setPen(Qt::NoPen);
    p->setBrush(c);

    const int size = qMin(r.width(), r.height());
    const int points[]  = { 0,0, size,size/2, 0,size };

    if (d != Right)
    {
        p->translate(r.center());
        switch (d)
        {
        case Down: p->rotate(90); break;
        case Left: p->rotate(180);
        case Up: p->rotate(270);
        }
        p->translate(-r.center());
    }
    p->drawPolygon(QPolygon(3, points));

    p->restore();
}
