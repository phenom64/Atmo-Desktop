
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QStyleOptionToolButton>
#include <QMainWindow>
#include <QDebug>
#include <QBrush>
#include <QDockWidget>

#include "styleproject.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/animhandler.h"
#include "stylelib/settings.h"

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

    drawPushButtonBevel(option, painter, widget);
    drawPushButtonLabel(option, painter, widget);
    return true;
}

/* not sure if these 2 are needed at all */

bool
StyleProject::drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Button, opt, option);
    if (!opt)
        return true;
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::ButtonText)), fg(Ops::fgRole(widget, QPalette::ButtonText));
    if (!(opt->features & QStyleOptionButton::Flat))
    {
        QColor bc(option->palette.color(bg));
        QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);

        if (option->SUNKEN || opt->features & QStyleOptionButton::DefaultButton)
            bc = sc;
        else if (option->ENABLED)
        {
            int hl(Anim::Basic::level(widget));
            bc = Color::mid(bc, sc, STEPS-hl, hl);
        }
        QLinearGradient lg(0, 0, 0, opt->rect.height());
        lg.setStops(Settings::gradientStops(Settings::conf.pushbtn.gradient, bc));
        QBrush m(lg);
        Render::drawClickable(Settings::conf.pushbtn.shadow, opt->rect, painter, Render::All, Settings::conf.pushbtn.rnd, Settings::conf.shadows.opacity, widget, &m);
    }
    return true;
}

bool
StyleProject::drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Button, opt, option);
    if (!opt)
        return true;
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::ButtonText)), fg(Ops::fgRole(widget, QPalette::ButtonText));
    QRect r(opt->rect);
    if (!opt->text.isEmpty())
        drawItemText(painter, r, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, fg);
    else
        drawItemPixmap(painter, r, Qt::AlignCenter, opt->icon.pixmap(opt->iconSize, opt->ENABLED?QIcon::Normal:QIcon::Disabled));
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
    QRect checkRect(subElementRect(SE_CheckBoxIndicator, opt, widget));
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
        checkRect.setBottom(qMin(checkRect.bottom(), widget->rect().bottom()));
        checkRect.setWidth(checkRect.height());
    }

    QRect textRect(subElementRect(SE_CheckBoxContents, opt, widget));
    int hor(opt->direction==Qt::LeftToRight?Qt::AlignLeft:Qt::AlignRight);
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->text, fg);

    if (opt->state & (State_On|State_NoChange))
    {
        bg = QPalette::Highlight;
        fg = QPalette::HighlightedText;
    }

    QColor bgc(opt->palette.color(bg));
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED && !(option->state & (State_On|State_NoChange)))
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }

    QLinearGradient lg(0, 0, 0, checkRect.height());
    lg.setStops(Settings::gradientStops(Settings::conf.pushbtn.gradient, bgc));
    QBrush mask(lg);
    Render::drawClickable(Settings::conf.pushbtn.shadow, checkRect, painter, Render::All, 3, Settings::conf.shadows.opacity, widget, &mask);

    if (opt->state & (State_On|State_NoChange))
        Ops::drawCheckMark(painter, opt->palette.color(fg), checkRect, opt->state & State_NoChange);

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
    QRect checkRect(subElementRect(SE_RadioButtonIndicator, opt, widget));
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
        checkRect.setBottom(qMin(checkRect.bottom(), widget->rect().bottom()));
        checkRect.setWidth(checkRect.height());
    }

    QRect textRect(subElementRect(SE_RadioButtonContents, opt, widget));
    int hor(opt->direction==Qt::LeftToRight?Qt::AlignLeft:Qt::AlignRight);
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->text, fg);
//    Render::renderShadow(Render::Raised, checkRect, painter);

    if (opt->state & State_On)
    {
        fg = QPalette::HighlightedText;
        bg = QPalette::Highlight;
    }

    QColor bgc(opt->palette.color(bg));
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED && !(option->state & State_On))
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }
    QLinearGradient lg(0, 0, 0, checkRect.height());
    lg.setStops(Settings::gradientStops(Settings::conf.pushbtn.gradient, bgc));
    QBrush mask(lg);
    Render::drawClickable(Settings::conf.pushbtn.shadow, checkRect, painter, Render::All, MAXRND, Settings::conf.shadows.opacity, widget, &mask);

    if (opt->state & State_On)
    {
        painter->save();
        painter->setBrush(opt->palette.color(fg));
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawEllipse(checkRect.adjusted(6, 6, -6, -6));
        painter->restore();
    }
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
    castOpt(ToolButton, opt, option);
    if (!opt)
        return true;
    painter->save();

    const QToolButton *toolBtn(qobject_cast<const QToolButton *>(widget));
    const QToolBar *bar(0);
    if (widget)
        bar = qobject_cast<const QToolBar *>(widget->parentWidget());

    Render::Sides sides = Render::All;
    bool nextSelected(false), prevSelected(false), isInTopToolBar(false);
    QRect rect(opt->rect);
    QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
    const bool hasMenu(Ops::hasMenu(toolBtn, opt));

    if (bar)
    {
        if (hasMenu && arrow.isValid())
            painter->setClipRegion(QRegion(rect)-QRegion(arrow));
        const QRect geo = widget->geometry();
        int x, y, r, b, h = widget->height(), hc = y+h/2, w = widget->width(), wc = x+w/2;
        int margin = pixelMetric(PM_ToolBarSeparatorExtent, opt, widget);
        geo.getCoords(&x, &y, &r, &b);
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
        if (const QMainWindow *win = qobject_cast<const QMainWindow *>(bar->parentWidget()))
            if (win->toolBarArea(const_cast<QToolBar *>(bar)) == Qt::TopToolBarArea)
                isInTopToolBar = true;

        QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::ButtonText)), fg(Ops::fgRole(widget, QPalette::ButtonText));
        QColor bc(option->palette.color(bg));
        QColor bca(bc);
        QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);

        if (option->SUNKEN)
            bca = bc = sc;
        else if (option->ENABLED)
        {
            int hl(Anim::ToolBtns::level(qobject_cast<const QToolButton *>(widget), false));
            int hla(Anim::ToolBtns::level(qobject_cast<const QToolButton *>(widget), true));
            bc = Color::mid(bc, sc, STEPS-hl, hl);
            bca = Color::mid(bca, sc, STEPS-hla, hla);
        }

        QLinearGradient lg(0, 0, 0, rect.height());
        lg.setStops(Settings::gradientStops(Settings::conf.toolbtn.gradient, bc));
        QBrush mask(lg);
        Render::drawClickable(Settings::conf.toolbtn.shadow, rect, painter, sides, Settings::conf.toolbtn.rnd, Settings::conf.shadows.opacity, widget, &mask);

        if (Ops::hasMenu(toolBtn, opt))
        {
            QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
            painter->setClipRect(arrow);

            QLinearGradient lga(0, 0, 0, rect.height());
            lga.setStops(Settings::gradientStops(Settings::conf.toolbtn.gradient, bca));
            QBrush amask(lga);
            Render::drawClickable(Settings::conf.toolbtn.shadow, rect, painter, sides, Settings::conf.toolbtn.rnd, Settings::conf.shadows.opacity, widget, &amask);
        }

        painter->setClipping(false);

        if (!(sides&Render::Right) && !nextSelected && bar->orientation() == Qt::Horizontal)
        {
            painter->setPen(QColor(0, 0, 0, 32));
            QRect r(rect);
            switch (Settings::conf.toolbtn.shadow)
            {
            case Render::Sunken:
            case Render::Etched:
            case Render::Raised:
                r.adjust(0, 1, 0, -2);
                break;
            case Render::Simple:
                r.adjust(0, 0, 0, -1);
                break;
            default: break;
            }
            painter->drawLine(r.topRight(), r.bottomRight());
        }
    }
    const QPixmap pix = opt->icon.pixmap(opt->iconSize, opt->ENABLED ? QIcon::Normal : QIcon::Disabled);
    Qt::Alignment iAlign = Qt::AlignCenter, tAlign = Qt::AlignCenter;

    if (Ops::hasMenu(toolBtn, opt))
    {
        rect.setRight(arrow.left());
        painter->setPen(QColor(0, 0, 0, 32));
        painter->drawLine(rect.adjusted(0, 0, 0, -1).topRight(), rect.adjusted(0, 1, 0, -1).bottomRight());
        Ops::drawArrow(painter, opt->palette.color(QPalette::ButtonText), arrow, Ops::Down, Qt::AlignCenter, 7);
    }

    if (rect.height() > opt->iconSize.height() && rect.width() > opt->iconSize.width())
        rect.shrink(3);
    QRect ir(rect);
    switch (opt->toolButtonStyle)
    {
    case Qt::ToolButtonTextBesideIcon:
//        iAlign = Qt::AlignLeft|Qt::AlignVCenter;
//        tAlign = Qt::AlignLeft|Qt::AlignVCenter;
        ir.setRight(ir.left()+opt->iconSize.width());
        rect.setLeft(ir.right());

        break;
    case Qt::ToolButtonTextUnderIcon:
//        iAlign = Qt::AlignTop|Qt::AlignHCenter
//        tAlign = Qt::AlignBottom|Qt::AlignHCenter;

        ir.setBottom(ir.top()+opt->iconSize.height());
        rect.setTop(ir.bottom());
        break;
    default: break;
    }

    if (opt->toolButtonStyle != Qt::ToolButtonTextOnly)
//    if (!pix.isNull())
        drawItemPixmap(painter, ir, iAlign, pix);
    if (opt->toolButtonStyle)
        drawItemText(painter, rect, tAlign, opt->palette, opt->ENABLED, opt->text, QPalette::ButtonText);
    painter->restore();
    return true;
}
