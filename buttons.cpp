
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

//        Render::renderShadow(Render::Raised, option->rect, painter);
//        int m(2);
//        QRect r(option->rect.shrinked(m));
        const QRect r(opt->rect);
        QLinearGradient lg(r.topLeft(), r.bottomLeft());
        lg.setColorAt(0.0f, Color::mid(bc, Qt::white, 5, 1));
        lg.setColorAt(1.0f, bc/*Color::mid(bc, Qt::black, 7, 1)*/);
        Render::renderMask(r, painter, lg, m_s.pushbtn.rnd);
        QLinearGradient shadow(0, 0, 0, r.height());
        shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
        shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
        shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
        QBrush b(shadow);
        Render::renderShadow(Render::Simple, r, painter, m_s.pushbtn.rnd, Render::All, 1.0f, &b);
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
    if (!opt->text.isEmpty())
        drawItemText(painter, opt->rect, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, fg);
    else
        drawItemPixmap(painter, opt->rect, Qt::AlignCenter, opt->icon.pixmap(opt->iconSize, opt->ENABLED?QIcon::Normal:QIcon::Disabled));
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


//    const float o(painter->opacity());
//    painter->setOpacity(0.5f*o);
//    Render::renderShadow(Render::Raised, checkRect, painter, 5);
//    painter->setOpacity(o);


    QColor bgc(opt->palette.color(bg));
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED && !(option->state & (State_On|State_NoChange)))
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }

    QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
    lg.setColorAt(0.0f, Color::mid(bgc, Qt::white, 5, 1));
    lg.setColorAt(1.0f, bgc/*Color::mid(bc, Qt::black, 7, 1)*/);

    Render::renderMask(checkRect, painter, lg, 3);
    QLinearGradient shadow(0, 0, 0, checkRect.height());
    shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
    shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
    shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
    QBrush b(shadow);
    Render::renderShadow(Render::Simple, checkRect, painter, 3, Render::All, 1.0f, &b);

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

    QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
    lg.setColorAt(0.0f, Color::mid(bgc, Qt::white, 5, 1));
    lg.setColorAt(1.0f, bgc/*Color::mid(bc, Qt::black, 7, 1)*/);

    Render::renderMask(checkRect, painter, lg);
    QLinearGradient shadow(0, 0, 0, checkRect.height());
    shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
    shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
    shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
    QBrush b(shadow);
    Render::renderShadow(Render::Simple, checkRect, painter, 32, Render::All, 1.0f, &b);

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

        QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
        lg.setColorAt(0.0f, Color::mid(bc, Qt::white, 5, 1));
        lg.setColorAt(1.0f, bc/*Color::mid(bc, Qt::black, 10, 1)*/);

        QLinearGradient shadow(0, 0, 0, option->rect.height());
        shadow.setColorAt(0.0f, QColor(0, 0, 0, 0));
        const int o(m_s.shadows.opacity*255);
        shadow.setColorAt(0.8f, QColor(0, 0, 0, o/3));
        shadow.setColorAt(1.0f, QColor(0, 0, 0, o));
        QBrush brush(shadow);
        Render::renderShadow(Render::Simple, rect, painter, m_s.toolbtn.rnd, sides, 1.0f, &brush);
        Render::renderMask(rect.sAdjusted(0, 0, 0, -1), painter, lg, m_s.toolbtn.rnd, sides);

//        if (option->SUNKEN)
//            Render::renderShadow(shadow, rect.sAdjusted(1, 1, -1, -2), painter, 4, Render::All-sides, 0.4f);

        if (Ops::hasMenu(toolBtn, opt))
        {
            QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
            painter->setClipRect(arrow);
            QLinearGradient lga(rect.topLeft(), rect.bottomLeft());
            lga.setColorAt(0.0f, Color::mid(bca, Qt::white, 5, 1));
            lga.setColorAt(1.0f, bca/*Color::mid(bc, Qt::black, 7, 1)*/);
            Render::renderShadow(Render::Simple, rect, painter, m_s.toolbtn.rnd, sides, m_s.shadows.opacity/*, &brush*/);
            Render::renderMask(rect.sAdjusted(0, 0, 0, -1), painter, lga, m_s.toolbtn.rnd, sides);
        }

        painter->setClipping(false);

        if (!(sides&Render::Right) && !nextSelected)
        {
            painter->setPen(QColor(0, 0, 0, 32));
            painter->drawLine(rect.adjusted(0, 0, 0, -1).topRight(), rect.adjusted(0, 1, 0, -1).bottomRight());
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
//        iAlign = Qt::AlignTop|Qt::AlignHCenter;
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
