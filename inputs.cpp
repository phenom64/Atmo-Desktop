
#include <QStyleOption>
#include <QStyleOptionFrame>
#include <QStyleOptionComboBox>
#include <QStyleOptionSpinBox>
#include <QDebug>
#include <QLineEdit>
#include <QToolBar>
#include <QGroupBox>
#include <QComboBox>
#include <QAbstractItemView>

#include "styleproject.h"
#include "stylelib/render.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/animhandler.h"
#include "config/settings.h"

static void drawSafariLineEdit(const QRect &r, QPainter *p, const QBrush &b, const QStyleOption *opt = 0)
{
    Render::renderMask(r.adjusted(1, 1, -1, -2), p, b);
    QBrush *c = 0;
    bool focus(false);
    if (opt && opt->state & QStyle::State_HasFocus)
    {
        c = new QBrush(opt->palette.color(QPalette::Highlight));
        focus = true;
    }
    Render::renderShadow(Render::Etched, r, p, 32, Render::All, 0.1f, c);
    Render::renderShadow(Render::Sunken, r, p, 32, Render::All, focus?0.6f:0.2f, c);
}

bool
StyleProject::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (qobject_cast<const QComboBox *>(widget?widget->parentWidget():0))
        return true;
    if (widget && widget->objectName() == "qt_spinbox_lineedit")
        return true;

    QBrush mask(option->palette.brush(QPalette::Base));
    if (mask.style() == Qt::SolidPattern || mask.style() == Qt::NoBrush)
    {
        QColor c(mask.color());
        if (dConf.input.tint.second > -1)
            c = Color::mid(c, dConf.input.tint.first, 100-dConf.input.tint.second, dConf.input.tint.second);
        QLinearGradient lg(0, 0, 0, Render::maskHeight(dConf.input.shadow, option->rect.height()));
        lg.setStops(Settings::gradientStops(dConf.input.gradient, c));
        mask = QBrush(lg);
    }

    Render::drawClickable(dConf.input.shadow, option->rect, painter, dConf.input.rnd, dConf.shadows.opacity, widget, option, &mask);
    return true;
}

bool
StyleProject::drawComboBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ComboBox, opt, option);
    if (!opt)
        return true;

    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Button))
            , fg(Ops::fgRole(widget, QPalette::ButtonText))
            , ar(QPalette::HighlightedText);

    const bool ltr(opt->direction == Qt::LeftToRight);

    QRect arrowRect(subControlRect(CC_ComboBox, opt, SC_ComboBoxArrow, widget));
    QRect frameRect(subControlRect(CC_ComboBox, opt, SC_ComboBoxFrame, widget));
    QRect iconRect; //there is no SC_ rect function for this?
    QRect textRect(frameRect);

    if (!opt->currentIcon.isNull())
    {
        const int h(frameRect.height());
        iconRect = QRect(0, 0, h, h);
    }
    if (opt->direction == Qt::LeftToRight)
    {
        iconRect.moveLeft(frameRect.left());
        textRect.setLeft(iconRect.right());
        textRect.setRight(arrowRect.left());
    }
    else if (opt->direction == Qt::RightToLeft)
    {
        iconRect.moveRight(frameRect.right());
        textRect.setRight(iconRect.left());
        textRect.setLeft(arrowRect.right());
    }
    int m(2);

    if (!opt->editable)
    {
        QColor bgc(opt->palette.color(bg));
        QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
        if (option->ENABLED && !(option->state & State_On))
        {
            int hl(Anim::Basic::level(widget));
            bgc = Color::mid(bgc, sc, STEPS-hl, hl);
        }
        if (dConf.pushbtn.tint.second > -1)
            bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);
        painter->setClipRegion(QRegion(frameRect)-QRegion(arrowRect));
        QLinearGradient lg(0, 0, 0, Render::maskHeight(dConf.pushbtn.shadow, frameRect.height()));
        lg.setStops(Settings::gradientStops(dConf.pushbtn.gradient, bgc));
        QBrush mask(lg);
        Render::drawClickable(dConf.pushbtn.shadow, opt->rect, painter, dConf.pushbtn.rnd, dConf.shadows.opacity, widget, option, &mask);

        const int o(dConf.shadows.opacity*255.0f);

        const QColor hc(opt->palette.color(QPalette::Highlight));
        QLinearGradient lga(0, 0, 0, Render::maskHeight(dConf.pushbtn.shadow, opt->rect.height()));
        lga.setStops(Settings::gradientStops(dConf.pushbtn.gradient, hc));
        mask = QBrush(lga);

        painter->setClipRect(arrowRect);
        Render::drawClickable(dConf.pushbtn.shadow, opt->rect, painter, dConf.pushbtn.rnd, dConf.shadows.opacity, widget, option, &mask, 0, Render::All & ~(ltr?Render::Left:Render::Right));
        painter->setClipping(false);

//        arrowRect.adjust(!ltr?m:0, m, -(ltr?m:0), -m);
//        painter->setPen(QColor(0, 0, 0, o/3));
//        const float op(painter->opacity());
//        painter->setOpacity(dConf.shadows.opacity);
//        if (ltr)
//            painter->drawLine(arrowRect.topLeft()/*-QPoint(1, 0)*/, arrowRect.bottomLeft()-QPoint(1, 0));
//        else
//            painter->drawLine(arrowRect.topRight()/*+QPoint(1, 0)*/, arrowRect.bottomRight()+QPoint(1, 0));
//        painter->setOpacity(op);
    }
    else
    {
        QBrush brush = opt->palette.brush(bg);
        if (widget)
            if (QLineEdit *l = widget->findChild<QLineEdit *>())
                brush = l->palette().brush(l->backgroundRole());
        if (brush.style() < 2)
        {
            QColor c(brush.color());
            if (dConf.input.tint.second > -1)
                c = Color::mid(c, dConf.input.tint.first, 100-dConf.input.tint.second, dConf.input.tint.second);
            QLinearGradient lg(0, 0, 0, Render::maskHeight(dConf.input.shadow, option->rect.height()));
            lg.setStops(Settings::gradientStops(dConf.input.gradient, c));
            brush = QBrush(lg);
        }
//        drawSafariLineEdit(opt->rect, painter, brush);
        Render::drawClickable(dConf.input.shadow, option->rect, painter, dConf.input.rnd, dConf.shadows.opacity, widget, option, &brush);
    }

    drawItemPixmap(painter, iconRect, Qt::AlignCenter, opt->currentIcon.pixmap(opt->iconSize));
    m*=2;
    const QColor ac(opt->palette.color(opt->editable?QPalette::Text:QPalette::HighlightedText));
    QRect a1(arrowRect.adjusted(0, 0, 0, -arrowRect.height()/2));
    QRect a2(arrowRect.adjusted(0, arrowRect.height()/2, 0, 0));
    Ops::drawArrow(painter, ac, a1/*arrowRect.adjusted(m*1.25f, m, -m, -m)*/, Ops::Up, 7);
    Ops::drawArrow(painter, ac, a2/*arrowRect.adjusted(m*1.25f, m, -m, -m)*/, Ops::Down, 7);
    return true;
}

bool
StyleProject::drawComboBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ComboBox, opt, option);
    if (!opt || opt->editable)
        return true;
    QPalette::ColorRole /*bg(Ops::bgRole(widget, QPalette::Button)),*/ fg(Ops::fgRole(widget, QPalette::ButtonText));
    const int m(6);
    const bool ltr(opt->direction==Qt::LeftToRight);
    QRect rect(subControlRect(CC_ComboBox, opt, SC_ComboBoxEditField, widget).adjusted(m, 0, -m, 0));
    const int hor(ltr?Qt::AlignLeft:Qt::AlignRight);
    if (!opt->currentIcon.isNull())
    {
        if (ltr)
            rect.setLeft(rect.left()+opt->iconSize.width());
        else
            rect.setRight(rect.right()-opt->iconSize.width());
    }
    drawItemText(painter, rect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->currentText, fg);
    return true;
}

bool
StyleProject::drawSpinBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(SpinBox, opt, option);
    if (!opt)
        return true;

//    QRect frame(subControlRect(CC_SpinBox, opt, SC_SpinBoxFrame, widget));
    QRect up(subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, widget).shrinked(2));
//    up.setBottom(up.bottom()-1);
    QRect down(subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, widget).shrinked(2));
//    down.setTop(down.top()+1);
//    QRect edit(subControlRect(CC_SpinBox, opt, SC_SpinBoxEditField, widget));

    QBrush mask(option->palette.brush(QPalette::Base));
    if (mask.style() < 2)
    {
        QColor c(mask.color());
        if (dConf.input.tint.second > -1)
            c = Color::mid(c, dConf.input.tint.first, 100-dConf.input.tint.second, dConf.input.tint.second);
        QLinearGradient lg(0, 0, 0, Render::maskHeight(dConf.input.shadow, opt->rect.height()));
        lg.setStops(Settings::gradientStops(dConf.input.gradient, c));
        mask = QBrush(lg);
    }
    Render::drawClickable(dConf.input.shadow, opt->rect, painter, dConf.input.rnd, dConf.shadows.opacity, widget, option, &mask);

    Ops::drawArrow(painter, opt->palette.color(QPalette::Text), up, Ops::Up, dConf.arrowSize);
    Ops::drawArrow(painter, opt->palette.color(QPalette::Text), down, Ops::Down, dConf.arrowSize);
    return true;
}
