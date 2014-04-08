
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QStyleOptionToolButton>

#include "styleproject.h"
#include "render.h"

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
    QColor bc(option->palette.color(QPalette::Button));
    if (option->isHovered)
        bc = bc.lighter();
    if (option->isSunken)
        bc = bc.darker();
    Render::renderMask(option->rect, painter, bc);
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
    if (!widget->parentWidget())
        return false;
    const QToolBar *bar = qobject_cast<const QToolBar *>(widget->parentWidget());
    if (!bar)
        return false;

    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return false;

    const QRect geo = widget->geometry();
    int x, y, r, b, h = widget->height(), hc = y+h/2, w = widget->width(), wc = x+w/2;
    int margin = pixelMetric(PM_ToolBarSeparatorExtent, opt, widget);
    geo.getCoords(&x, &y, &r, &b);
    int sides = Render::All;
    if (qobject_cast<QToolButton *>(bar->childAt(r+margin, hc)))
        sides &= ~Render::Right;
    if (qobject_cast<QToolButton *>(bar->childAt(x-margin, hc)))
        sides &= ~Render::Left;
    if (qobject_cast<QToolButton *>(bar->childAt(wc, b+margin)))
        sides &= ~Render::Bottom;
    if (qobject_cast<QToolButton *>(bar->childAt(wc, y-margin)))
        sides &= ~Render::Top;

    QColor bc(option->palette.color(QPalette::Button));
    if (option->isEnabled)
    {
        if (option->isHovered)
            bc = bc.lighter();
        if (option->isSunken)
            bc = bc.darker();
    }
    Render::renderMask(option->rect, painter, bc, 31, (Render::Sides)sides);
    const QPixmap pix = opt->icon.pixmap(opt->iconSize, opt->isEnabled ? QIcon::Normal : QIcon::Disabled);
    Qt::Alignment iAlign = Qt::AlignCenter, tAlign = Qt::AlignCenter;
    QRect rect(option->rect);
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
