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
#include <QResizeEvent>
#include <QApplication>
#include <QAbstractScrollArea>
#include <QToolButton>
#include <QStyle>
#include <QStyleOptionToolButton>

#include "ops.h"
#include "xhandler.h"
#include "macros.h"
#include "color.h"
#include "render.h"

Q_DECL_EXPORT Ops *Ops::s_instance = 0;

Ops
*Ops::instance()
{
    if (!s_instance)
        s_instance = new Ops();
    return s_instance;
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
    if (!tabBar || !(tabBar->shape() == QTabBar::RoundedNorth || tabBar->shape() == QTabBar::TriangularNorth) || !tabBar->window())
        return false;
//    QMainWindow *mainWin = qobject_cast<QMainWindow *>(tabBar->window()); // gnarffffff! rekonq tabbar isnt in a mainwin... sighs
    QWidget *win = tabBar->window();
    QPoint topLeft = tabBar->mapTo(win, tabBar->rect().topLeft());
    QRect winRect = win->rect();
    QRect tabBarRect = QRect(topLeft, tabBar->size());
    if (tabBarRect.top() <= winRect.top())
        return true;
    const QPoint &checkPoint(tabBar->mapTo(win, tabBar->rect().translated(1, -6).topLeft()));
    if (isOrInsideA<const QToolBar *>(win->childAt(checkPoint)))
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
    unsigned char *h = XHandler::getXProperty<unsigned char>(win->winId(), XHandler::DecoData);
    if (!h)
        return 0;
    unsigned int height(*h);
    win->setProperty("titleHeight", height);
    castObj(QMainWindow *, mw, win);
    if (!mw)
        return height;
    int tbheight(0);
    if (QToolBar *tb = mw->findChild<QToolBar *>())
    {
        if (tb->isVisible())
        if (mw->toolBarArea(tb) == Qt::TopToolBarArea && tb->geometry().top() <= 0)
        {
            tbheight = tb->height();
            needSeparator = 0;
        }
    }
    height += tbheight;
    if (QTabBar *tb = mw->findChild<QTabBar *>())
        if (tb->isVisible())
        if (tb->geometry().top() <= 0)
            needSeparator = 0;
    return height;
}

void
Ops::fixWindowTitleBar(QWidget *win)
{
    unsigned int ns(1);
    WindowData wd;
    wd.height = getHeadHeight(win, ns);
    wd.separator = ns;
    wd.top = Color::titleBarColors[0].rgba();
    wd.bottom = Color::titleBarColors[1].rgba();
    wd.text = win->palette().color(win->foregroundRole()).rgba();
    if (!wd.height)
        return;
    XHandler::setXProperty<unsigned int>(win->winId(), XHandler::WindowData, XHandler::Short, reinterpret_cast<unsigned int *>(&wd), 5);
//    qDebug() << ((c & 0xff000000) >> 24) << ((c & 0xff0000) >> 16) << ((c & 0xff00) >> 8) << (c & 0xff);
//    qDebug() << QColor(c).alpha() << QColor(c).red() << QColor(c).green() << QColor(c).blue();
    updateWindow(win->winId());
    const QList<QToolBar *> toolBars = win->findChildren<QToolBar *>();
    for (int i = 0; i < toolBars.size(); ++i)
        toolBars.at(i)->update();
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
    if (castObj(const QAbstractScrollArea *, area, w))
        if (area->viewport()->autoFillBackground())
            return area->viewport()->backgroundRole();
    return w->backgroundRole();
}

QPalette::ColorRole
Ops::fgRole(const QWidget *w, const QPalette::ColorRole fallBack)
{
    if (!w)
        return fallBack;
    if (castObj(const QAbstractScrollArea *, area, w))
        if (area->viewport()->autoFillBackground())
            return area->viewport()->foregroundRole();
    return w->foregroundRole();
}

ToolButtonData
Ops::toolButtonData(const QToolButton *tbtn, const QStyle *s, bool &ok, const QStyleOption *opt)
{
    ToolButtonData data;
    ok = false;
    if (!tbtn)
        return data;
    castObj(const QToolBar *, bar, tbtn->parentWidget());
    if (!bar)
        return data;
    const QRect geo = tbtn->geometry();
    int x, y, r, b, h = tbtn->height(), hc = y+h/2, w = tbtn->width(), wc = x+w/2;
    int margin = s->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, opt, tbtn);
    geo.getCoords(&x, &y, &r, &b);
    Render::Sides sides = Render::All;
    bool nextSelected(false), prevSelected(false), isInTopToolBar(false);

    if (const QToolButton *btn = qobject_cast<const QToolButton *>(bar->childAt(r+margin, hc)))
    {
        sides &= ~Render::Right;
        if (btn->isChecked())
            nextSelected = true;
    }
    if (const QToolButton *btn = qobject_cast<const QToolButton *>(bar->childAt(x-margin, hc)))
    {
        sides &= ~Render::Left;
        if (btn->isChecked())
            prevSelected = true;
    }
    if (const QToolButton *btn = qobject_cast<const QToolButton *>(bar->childAt(wc, b+margin)))
    {
        sides &= ~Render::Bottom;
        if (btn->isChecked())
            nextSelected = true;
    }
    if (const QToolButton *btn = qobject_cast<const QToolButton *>(bar->childAt(wc, y-margin)))
    {
        sides &= ~Render::Top;
        if (btn->isChecked())
            prevSelected = true;
    }
//    if (const QToolButton *btn = qobject_cast<const QToolButton *>(widget))
        if (tbtn->isChecked())
            nextSelected = true;
    if (const QMainWindow *win = qobject_cast<const QMainWindow *>(bar->parentWidget()))
        if (win->toolBarArea(const_cast<QToolBar *>(bar)) == Qt::TopToolBarArea)
            isInTopToolBar = true;

    data.sides = sides;
    data.isInTopToolBar = isInTopToolBar;
    data.prevSelected = prevSelected;
    data.nextSelected = nextSelected;
    ok = true;
    return data;
}
