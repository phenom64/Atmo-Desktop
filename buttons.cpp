
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
 * QPlastiqueStyle painting, simply return true here
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
    QColor bc(option->palette.color(QPalette::Button));
    if (option->isHovered)
        bc = bc.lighter();
    if (option->isSunken)
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

    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return false;


    const QRect geo = widget->geometry();
    int x, y, r, b, h = widget->height(), hc = y+h/2, w = widget->width(), wc = x+w/2;
    int margin = pixelMetric(PM_ToolBarSeparatorExtent, opt, widget);
    geo.getCoords(&x, &y, &r, &b);
    const QToolBar *bar = qobject_cast<const QToolBar *>(widget->parentWidget());
    int sides = Render::All;
    if (bar)
    {
        if (qobject_cast<QToolButton *>(bar->childAt(r+margin, hc)))
            sides &= ~Render::Right;
        if (qobject_cast<QToolButton *>(bar->childAt(x-margin, hc)))
            sides &= ~Render::Left;
        if (qobject_cast<QToolButton *>(bar->childAt(wc, b+margin)))
            sides &= ~Render::Bottom;
        if (qobject_cast<QToolButton *>(bar->childAt(wc, y-margin)))
            sides &= ~Render::Top;
    }
    QColor bc(option->palette.color(QPalette::Button));
    if (option->isEnabled)
    {
        if (option->isHovered)
            bc = bc.lighter(110);
        if (option->isSunken)
            bc = bc.darker(110);
    }
#define sAdjusted(_X1_, _Y1_, _X2_, _Y2_) adjusted(bool(sides&Render::Left)*_X1_, bool(sides&Render::Top)*_Y1_, bool(sides&Render::Right)*_X2_, bool(sides&Render::Bottom)*_Y2_)
    QRect rect(option->rect);
    QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
    lg.setColorAt(0.0f, Ops::mid(bc, Qt::white, 2, 1));
    lg.setColorAt(1.0f, Ops::mid(bc, Qt::black, 7, 1));
    Render::renderMask(rect.sAdjusted(1, 1, -1, -2), painter, lg, 3, (Render::Sides)sides);
    painter->setOpacity(0.5f);
    Render::renderShadow(Render::Etched, option->rect, painter, 4, (Render::Sides)sides);
    if (!(sides&Render::Right))
    {
        painter->setPen(Qt::black);
        painter->drawLine(rect.adjusted(0, 3, 0, -4).topRight(), rect.adjusted(0, 3, 0, -4).bottomRight());
    }
    painter->setOpacity(1.0f);
    const QPixmap pix = opt->icon.pixmap(opt->iconSize, opt->isEnabled ? QIcon::Normal : QIcon::Disabled);
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
        drawItemText(painter, rect, tAlign, option->palette, opt->isEnabled, opt->text, QPalette::ButtonText);
    return true;
}
