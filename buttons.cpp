
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QStyleOptionToolButton>

#include "styleproject.h"
#include "render.h"
#include "ops.h"

/*
 * This here paints the button, in order to override
 * QCommonStyle painting, simply return true here
 * and paint what you want yourself.
 */

bool
StyleProject::drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    /* uncomment the following lines and remove the
     * return false in order to fill *all* pushbuttons
     * w/ a beautiful red color and nothing else
     */
//    painter->fillRect(option->rect, Qt::red);
//    return true;
    return false;
}

/* not sure if these 2 are needed at all */

bool
StyleProject::drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    if (!painter->isActive())
        return false;
    QColor bc(option->palette.color(QPalette::Button));
    if (option->HOVER)
        bc = bc.lighter();
    if (option->SUNKEN)
        bc = bc.darker();

    painter->setOpacity(0.75f);
    Render::renderShadow(Render::Raised, option->rect, painter);
    painter->setOpacity(1.0f);
    QRect r(option->rect.adjusted(3, 3, -3, -3));
    QLinearGradient lg(r.topLeft(), r.bottomLeft());
    lg.setColorAt(0.0f, Ops::mid(bc, Qt::white, 5, 1));
    lg.setColorAt(1.0f, Ops::mid(bc, Qt::black, 7, 1));
    Render::renderMask(r, painter, lg);
    painter->restore();
    return true;
}

bool
StyleProject::drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}


/* checkboxes */

bool
StyleProject::drawCheckBox(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}

bool
StyleProject::drawCheckBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}

/* radiobuttons */

bool
StyleProject::drawRadioButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}

bool
StyleProject::drawRadioButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}

bool
StyleProject::drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}

bool
StyleProject::drawToolButton(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && !widget->parentWidget())
        return false;

    castOpt(ToolButton, opt, option);
    if (!opt || !painter->isActive())
        return false;


    const QRect geo = widget->geometry();
    int x, y, r, b, h = widget->height(), hc = y+h/2, w = widget->width(), wc = x+w/2;
    int margin = pixelMetric(PM_ToolBarSeparatorExtent, opt, widget);
    geo.getCoords(&x, &y, &r, &b);
    const QToolBar *bar = qobject_cast<const QToolBar *>(widget->parentWidget());
    Render::Sides sides = Render::All;
    bool nextSelected(false), prevSelected(false);
    if (bar)
    {
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
        if (const QToolButton *btn = qobject_cast<const QToolButton *>(widget))
            if (btn->isChecked())
                nextSelected = true;
    }
    Render::Shadow shadow = Render::Etched;
    QColor bc(option->palette.color(QPalette::Button));
    if (option->ENABLED)
    {
        if (option->HOVER)
            bc = bc.lighter(110);
        if (option->SUNKEN)
        {
            bc = bc.darker(110);
            shadow = Render::Sunken;
        }
    }

    QRect rect(opt->rect);
    QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
    const QColor &start = Ops::mid(bc, Qt::white, 2, 1), &end = Ops::mid(bc, Qt::black, 2, 1);
    lg.setColorAt(0.0f, opt->SUNKEN ? end : start);
    lg.setColorAt(1.0f, opt->SUNKEN ? Ops::mid(bc, end) : end);
    Render::renderMask(rect.sAdjusted(1, 1, -1, -2), painter, lg, 3, sides);
    painter->setOpacity(0.5f);
    Render::renderShadow(shadow, opt->rect, painter, 4, sides);
    if (!(sides&Render::Right) && !nextSelected)
    {
        painter->setPen(Qt::black);
        painter->drawLine(rect.adjusted(0, 3, 0, -4).topRight(), rect.adjusted(0, 3, 0, -4).bottomRight());
    }
    if (option->SUNKEN)
        Render::renderShadow(shadow, rect.sAdjusted(1, 1, -1, -2), painter, 4, Render::All-sides);
    painter->setOpacity(1.0f);
    const QPixmap pix = opt->icon.pixmap(opt->iconSize, opt->ENABLED ? QIcon::Normal : QIcon::Disabled);
    Qt::Alignment iAlign = Qt::AlignCenter, tAlign = Qt::AlignCenter;

    switch (opt->toolButtonStyle)
    {
    case Qt::ToolButtonTextBesideIcon:
        iAlign = Qt::AlignLeft|Qt::AlignVCenter;
        tAlign = Qt::AlignLeft|Qt::AlignVCenter;
        rect.setLeft(rect.left()+opt->iconSize.width());
        break;
    case Qt::ToolButtonTextUnderIcon:
        iAlign = Qt::AlignTop|Qt::AlignHCenter;
        tAlign = Qt::AlignBottom|Qt::AlignHCenter;
        rect.setTop(rect.top()+opt->iconSize.height());
        break;
    }

    if (opt->toolButtonStyle != Qt::ToolButtonTextOnly)
        drawItemPixmap(painter, option->rect, iAlign, pix);
    if (opt->toolButtonStyle)
        drawItemText(painter, rect, tAlign, opt->palette, opt->ENABLED, opt->text, QPalette::ButtonText);
    return true;
}
