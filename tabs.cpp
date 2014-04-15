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

#include "styleproject.h"
#include "stylelib/ops.h"
#include "render.h"

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
    QPalette::ColorRole fg(QPalette::ButtonText), bg(QPalette::Button);
    //    if (widget)
    //    {
    //        fg = widget->foregroundRole();
    //        bg = widget->backgroundRole();
    //    }
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
    QColor bgc(opt->palette.color(bg));
    if (Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget)))
    {
        if (opt->state & State_Selected)
            bgc = m_specialColor[1];
        else
            return true;
        sides = Render::All & ~Render::Top;
    }
    painter->save();
    painter->setOpacity(0.5f);
    const int m(3);
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
    return false;
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
            sides &= ~Render::Top;
            Render::renderMask(widget->rect(), painter, Ops::mid(m_specialColor[1], Qt::black, 4, 1), 32, sides);
            painter->save();
            painter->setOpacity(0.5f);
            Render::renderShadow(Render::Sunken, widget->rect().adjusted(0, 0, 0, 1), painter, 32, sides);
            if (!(sides & Render::Top))
                Render::renderShadow(Render::Sunken, widget->rect().adjusted(0, 0, 0, 1), painter, 32, Render::Top);
            painter->restore();
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
    const int m(3);
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
            sides &= ~Render::Top;
            Render::renderMask(barRect.adjusted(0, 0, 0, -1), painter, Ops::mid(m_specialColor[1], Qt::black, 2, 1), 32, sides);
            painter->save();
            painter->setOpacity(0.5f);
            Render::renderShadow(Render::Sunken, barRect, painter, 32, sides);
            if (!(sides & Render::Top))
                Render::renderShadow(Render::Sunken, barRect, painter, 32, Render::Top);
            painter->restore();
            return true;
        }
    }
    painter->save();
    painter->setOpacity(0.5f);
    Render::renderShadow(Render::Raised, rect, painter, 7, sides);
    painter->restore();
    Render::renderMask(r, painter, opt->palette.color(QPalette::Button), 4, sides);
    return true;
}

bool
StyleProject::drawTabCloser(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}
