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
#include "xhandler.h"
#include "macros.h"
#include "color.h"
#include "../styleproject.h"

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
Ops::drawCheckMark(QPainter *p, const QColor &c, const QRect &r, const bool tristate)
{
    p->save();
    p->translate(r.topLeft());

    const int size = qMin(r.width(), r.height());
    const int third = size/3, thirds = third*2, sixth=third/2;
    const int points[] = { third,third+sixth, third+sixth,thirds+sixth, thirds+sixth,third-sixth };

    p->setRenderHint(QPainter::Antialiasing);
    QPen pen(c, third*(tristate?0.33f:0.66f), tristate?Qt::DashLine:Qt::SolidLine);
    pen.setDashPattern(QVector<qreal>() << 0.05f << 1.5f);
    pen.setStyle(tristate?Qt::CustomDashLine:Qt::SolidLine);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    QPainterPath path;
    path.addPolygon(QPolygon(3, points));
    p->drawPath(path);

    p->restore();
}

void
Ops::drawArrow(QPainter *p, const QColor &c, const QRect &r, const Direction d, const Qt::Alignment align, int size)
{
    p->save();
    p->setPen(Qt::NoPen);
    p->setRenderHint(QPainter::Antialiasing);
    p->setBrush(c);

    if (!size)
        size = qMin(r.width(), r.height());
    size &= ~1;

    QRect rect(0, 0, size, size);
    if (align & (Qt::AlignVCenter|Qt::AlignHCenter))
        rect.moveCenter(r.center());
    if (align & Qt::AlignLeft)
        rect.moveLeft(r.left());
    if (align & Qt::AlignRight)
        rect.moveRight(r.right());
    if (align & Qt::AlignTop)
        rect.moveTop(r.top());
    if (align & Qt::AlignBottom)
        rect.moveBottom(r.bottom());

    p->translate(rect.topLeft());

    const int half(size >> 1);
    const int points[]  = { 0,0, size,half, 0,size };

    if (d != Right)
    {
        p->translate(half, half);
        switch (d)
        {
        case Down: p->rotate(90); break;
        case Left: p->rotate(180); break;
        case Up: p->rotate(-90); break;
        }
        p->translate(-half, -half);
    }
    p->drawPolygon(QPolygon(3, points));
    p->restore();
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

static unsigned int getHeadHeight(QWidget *win, unsigned int &needSeparator)
{
    unsigned int *h = XHandler::getXProperty<unsigned int>(win->winId(), XHandler::DecoData);
    if (!h)
        return 0;
    unsigned int height(*h);
    win->setProperty("titleHeight", height);
    QWidget *first(win->childAt(1, 1));
    if (castObj(QToolBar *, tb, first))
    {
        height += tb->height();
        needSeparator = 0;
    }
    else if (castObj(QTabBar *, bar, first))
        needSeparator = 0;
    return height;
}

void
Ops::fixWindowTitleBar(QWidget *win)
{
    unsigned int ns(1);
//    WindowData wd;
//    wd.height = getHeadHeight(win, ns);
//    wd.separator = ns;
//    wd.top = Color::titleBarColors[0].rgba();
//    wd.bottom = Color::titleBarColors[1].rgba();
    unsigned int height(getHeadHeight(win, ns));
    if (!height)
        return;
    unsigned int d[4] = { Color::titleBarColors[0].rgba(), Color::titleBarColors[1].rgba(), height, ns };
    const int n(4);
    qDebug() << "sending data to deco, colors:" << Color::titleBarColors[0] << Color::titleBarColors[1] << "and height:" << height;
    XHandler::setXProperty<unsigned int>(win->winId(), XHandler::WindowData, d, n);
//            qDebug() << ((c & 0xff000000) >> 24) << ((c & 0xff0000) >> 16) << ((c & 0xff00) >> 8) << (c & 0xff);
//            qDebug() << QColor(c).alpha() << QColor(c).red() << QColor(c).green() << QColor(c).blue();
    updateWindow(win->winId());
    const QList<QToolBar *> toolBars = win->findChildren<QToolBar *>();
    for (int i = 0; i < toolBars.size(); ++i)
        toolBars.at(i)->update();
}
