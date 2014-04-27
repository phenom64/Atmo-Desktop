
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QStyleOptionToolButton>

#include "styleproject.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"

/*
 * This here paints the button, in order to override
 * QCommonStyle painting, simply return true here
 * and paint what you want yourself.
 */

bool
StyleProject::drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Button, opt, option);
    if (!opt)
        return true;

    QPalette::ColorRole bg(QPalette::Button), fg(QPalette::ButtonText);
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
    }

    if (!(opt->features & QStyleOptionButton::Flat))
    {
        QColor bc(option->palette.color(bg));
        if (option->HOVER)
            bc = bc.lighter();
        if (option->SUNKEN)
            bc = bc.darker();

        const int o(painter->opacity());
        painter->setOpacity(0.75f);
        Render::renderShadow(Render::Raised, option->rect, painter);
        painter->setOpacity(o);
        QRect r(option->rect.adjusted(3, 3, -3, -3));
        QLinearGradient lg(r.topLeft(), r.bottomLeft());
        lg.setColorAt(0.0f, Color::mid(bc, Qt::white, 5, 1));
        lg.setColorAt(1.0f, Color::mid(bc, Qt::black, 7, 1));
        Render::renderMask(r, painter, lg);
    }

    drawItemText(painter, opt->rect, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, fg);
    return true;
}

/* not sure if these 2 are needed at all */

bool
StyleProject::drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return true;
}

bool
StyleProject::drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return true;
}


/* checkboxes */

bool
StyleProject::drawCheckBox(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Button, opt, option);
    if (!opt)
        return true;

    QPalette::ColorRole bg(QPalette::Button), fg(QPalette::ButtonText);
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
    }
    QRect checkRect(subElementRect(SE_CheckBoxIndicator, opt, widget));
    QRect textRect(subElementRect(SE_CheckBoxContents, opt, widget));
    const float o(painter->opacity());
    painter->setOpacity(0.5f*o);
    Render::renderShadow(Render::Raised, checkRect, painter, 5);
    painter->setOpacity(o);


    const QColor bgc(opt->palette.color(bg));
    QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
    lg.setColorAt(0.0f, Color::mid(bgc, Qt::white, 5, 1));
    lg.setColorAt(1.0f, Color::mid(bgc, Qt::black, 7, 1));

    Render::renderMask(checkRect.adjusted(3, 3, -3, -3), painter, lg, 2);

    if (opt->state & (State_On|State_NoChange))
        Ops::drawCheckMark(painter, opt->palette.color(fg), checkRect, opt->state & State_NoChange);

    int hor(opt->direction==Qt::LeftToRight?Qt::AlignLeft:Qt::AlignRight);
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->text, fg);
    return true;
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
    castOpt(Button, opt, option);
    if (!opt)
        return true;

    QPalette::ColorRole bg(QPalette::Button), fg(QPalette::ButtonText);
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
    }
    int size(opt->rect.height());
    QRect checkRect(subElementRect(SE_RadioButtonIndicator, opt, widget));
    QRect textRect(subElementRect(SE_RadioButtonContents, opt, widget));
    Render::renderShadow(Render::Raised, checkRect, painter);

    const QColor bgc(opt->palette.color(bg));
    QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
    lg.setColorAt(0.0f, Color::mid(bgc, Qt::white, 5, 1));
    lg.setColorAt(1.0f, Color::mid(bgc, Qt::black, 7, 1));

    Render::renderMask(checkRect.adjusted(3, 3, -3, -3), painter, lg);

    if (opt->state & State_On)
    {
        painter->save();
        painter->setBrush(opt->palette.color(fg));
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawEllipse(checkRect.adjusted(6, 6, -6, -6));
        painter->restore();
    }
    int hor(opt->direction==Qt::LeftToRight?Qt::AlignLeft:Qt::AlignRight);
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->text, fg);
    return true;
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
    const QColor &start = Color::mid(bc, Qt::white, 1, 2), &end = Color::mid(bc, Qt::black, 3, 1);
    lg.setColorAt(0.0f, opt->SUNKEN ? end : start);
    lg.setColorAt(1.0f, opt->SUNKEN ? Color::mid(bc, end) : end);
    Render::renderMask(rect.sAdjusted(1, 1, -1, -2), painter, lg, 3, sides);
    painter->setOpacity(0.4f);
    Render::renderShadow(shadow, opt->rect, painter, 4, sides);
    if (!(sides&Render::Right) && !nextSelected)
    {
        painter->setPen(QColor(0, 0, 0, 192));
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
