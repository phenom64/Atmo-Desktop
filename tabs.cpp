#include <QDebug>
#include <QTabBar>
#include <QTabWidget>
#include <QStyleOption>
#include <QStyleOptionTab>
#include <QStyleOptionTabBarBase>
#include <QStyleOptionTabBarBaseV2>
#include <QStyleOptionTabV2>
#include <QStyleOptionTabV3>
#include <QStyleOptionTabWidgetFrame>
#include <QStyleOptionTabWidgetFrameV2>
#include <QMainWindow>
#include <QToolBar>
#include <QPaintDevice>
#include <QPainter>

#include "styleproject.h"
#include "stylelib/ops.h"
#include "stylelib/render.h"
#include "stylelib/color.h"

bool
StyleProject::drawTab(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}

bool
StyleProject::drawTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Tab, opt, option);
    if (!opt)
        return true;
    Render::Sides sides = Render::All;
    QPalette::ColorRole fg(QPalette::WindowText), bg(QPalette::Window);
    if (widget)
    {
        fg = widget->foregroundRole();
        bg = widget->backgroundRole();
    }
    QColor bgc(opt->palette.color(bg));
    castObj(const QTabBar *, bar, widget);
    if (Ops::isSafariTabBar(bar))
    {
        const bool isLeftOf(bar->tabAt(opt->rect.center()) < bar->currentIndex());
        const bool isFirst(opt->position == QStyleOptionTab::Beginning || opt->position == QStyleOptionTab::OnlyOneTab);
//        const bool isLast(opt->position == QStyleOptionTab::End);
        const bool isSelected(opt->state & State_Selected || opt->position == QStyleOptionTab::OnlyOneTab);
        const int o(pixelMetric(PM_TabBarTabOverlap));
        QRect r(opt->rect.adjusted(0, 0, 0, !isSelected));
//        if (isLeftOf)
        if (painter->device() == widget)
        {
            r.setLeft(r.left()-(isFirst?o/2:o));
            r.setRight(r.right()+o);
        }
//        else
        QPainterPath p;
        Render::renderTab(r, painter, isLeftOf ? Render::BeforeSelected : isSelected ? Render::Selected : Render::AfterSelected, &p);
        if (isSelected)
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setBrush(Color::titleBarColors[1]);
            painter->setPen(Qt::NoPen);
            painter->drawPath(p);
            painter->restore();
        }
        return true;
    }
    switch (opt->shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
        sides &= ~Render::Bottom;
        break;
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        sides &= ~Render::Top;
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        sides &= ~Render::Right;
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        sides &= ~Render::Left;
        break;
    default: break;
    }
    painter->save();
//    painter->setOpacity(0.5f);
    const int m(2);
    QRect r(opt->rect);
    if (!(opt->state & State_Selected))
        r.sAdjust(m, m, -m, -m);
    Render::renderShadow(Render::Raised, r, painter, 6, sides);
    painter->restore();

    r.sAdjust(m, m, -m, -m);
    Render::renderMask(r, painter, bgc, 3, sides);
    return true;
}

bool
StyleProject::drawTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
#if 0
    castOpt(TabV3, opt, option);
    if (!Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget)) || !opt)
        return false;

    const QRect tr(subElementRect(SE_TabBarTabText, option, widget));
    drawItemText(painter, tr, Qt::AlignCenter, option->palette, option->ENABLED, opt->text, widget?widget->foregroundRole():QPalette::WindowText);
//    castObj(const QTabBar *, bar, widget);
    const QTabBar *bar = static_cast<const QTabBar *>(widget); //tabbar guaranteed by qobject_cast above
    QRect ir(QPoint(), bar->iconSize());
    ir.moveCenter(opt->rect.center());
    int d(8);
    int index(bar->tabAt(opt->rect.center()));
    int right(tr.left());
//    if (index > bar->currentIndex())
//        right+=d;
    ir.moveRight(right);

    drawItemPixmap(painter, ir, Qt::AlignCenter, opt->icon.pixmap(bar->iconSize()));
    return true;
#endif
    return false;
}

static void renderSafariBar(QPainter *p, const QTabBar *bar, const QColor &c, Render::Sides sides, QRect rect = QRect())
{
    QRect r(rect.isValid()?rect:bar->rect());
    p->fillRect(r, c);
    r.setBottom(r.bottom()+1);
    Render::renderShadow(Render::Sunken, r, p, 8, Render::Top|Render::Bottom, 0.5f);
}

bool
StyleProject::drawTabBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && widget->parentWidget() && widget->parentWidget()->inherits("KTabWidget"))
        return true;

    Render::Sides sides = Render::checkedForWindowEdges(widget);
    if (const QTabBar *tabBar = qobject_cast<const QTabBar *>(widget))
    {
        if (Ops::isSafariTabBar(tabBar))
        {
            renderSafariBar(painter, tabBar, Color::mid(Color::titleBarColors[1], Qt::black, 8, 1), sides);
            return true;
        }
    }
    return true;
}

bool
StyleProject::drawTabWidget(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    Render::Sides sides = Render::checkedForWindowEdges(widget);
    castObj(const QTabWidget *, tabWidget, widget);
    castOpt(TabWidgetFrame, opt, option);

    if (!tabWidget || !opt)
        return true;
    const int m(2);
    QRect r(opt->rect.adjusted(m, m, -m, -m));
    QRect rect(opt->rect);
    if (const QTabBar *tabBar = tabWidget->findChild<const QTabBar *>())
    {
        QRect barRect(tabBar->geometry());
        switch (opt->shape)
        {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth:
        {
            barRect.setLeft(tabWidget->rect().left());
            barRect.setRight(tabWidget->rect().right());
            const int b(barRect.bottom()+1);
            r.setTop(b);
            rect.setTop(b-m);
            break;
        }
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
        {
            const int t(barRect.top()-1);
            r.setBottom(t);
            rect.setBottom(t+m);
            break;
        }
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
        {
            const int right(barRect.right()+1);
            r.setLeft(right);
            rect.setLeft(right-m);
            break;
        }
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
        {
            const int l(barRect.left()-1);
            r.setRight(l);
            rect.setRight(l+m);
            break;
        }
        default: break;
        }
        if (Ops::isSafariTabBar(tabBar))
        {
            renderSafariBar(painter, tabBar, Color::mid(Color::titleBarColors[1], Qt::black, 8, 1), sides, barRect);
            return true;
        }
    }
    Render::renderShadow(Render::Raised, rect, painter, 7, sides);
    Render::renderMask(r, painter, opt->palette.color(QPalette::Window), 4, sides);
    return true;
}

static QPixmap closer[2];

bool
StyleProject::drawTabCloser(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const int size(qMin(option->rect.width(), option->rect.height())), _2_(size/2),_4_(size/4), _8_(size/8), _16_(size/16);
    const QRect line(_2_-_16_, _4_, _8_, size-(_4_*2));


    const bool hover(option->HOVER);
    if (closer[hover].isNull())
    {
        QPixmap pix = QPixmap(option->rect.size());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.setRenderHint(QPainter::Antialiasing);
        QRect l(pix.rect().adjusted(_16_, _16_, -_16_, -_16_));
        p.drawEllipse(l);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        const int r[2] = { 45, 90 };
        for (int i = 0; i < 2; ++i)
        {
            p.translate(_2_, _2_);
            p.rotate(r[i]);
            p.translate(-_2_, -_2_);
            p.drawRect(line);
        }
        p.end();

        bool isDark(false);
        if (widget)
            isDark = Color::luminosity(widget->palette().color(widget->foregroundRole())) > Color::luminosity(widget->palette().color(widget->backgroundRole()));
        int cc(isDark?0:255);
        QPixmap tmp(pix);
        Render::colorizePixmap(tmp, QColor(cc, cc, cc, 127));
        QPixmap tmp2(pix);
        QPalette::ColorRole role(widget?widget->foregroundRole():QPalette::WindowText);
        if (hover)
            role = QPalette::Highlight;
        Render::colorizePixmap(tmp2, option->palette.color(role));

        closer[hover] = QPixmap(option->rect.size());
        closer[hover].fill(Qt::transparent);
        p.begin(&closer[hover]);
        p.drawTiledPixmap(closer[hover].rect().translated(0, 1), tmp);
        p.drawTiledPixmap(closer[hover].rect(), tmp2);
        p.end();
    }
    painter->drawTiledPixmap(option->rect, closer[hover]);
    return true;
}
