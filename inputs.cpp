
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
#include <QPainter>
#include <QSpinBox>

#include "dsp.h"
#include "stylelib/gfx.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/animhandler.h"
#include "config/settings.h"

using namespace DSP;

bool
Style::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (qobject_cast<const QComboBox *>(widget?widget->parentWidget():0))
        return true;
    if (widget && widget->objectName() == "qt_spinbox_lineedit")
        return true;

    QBrush mask(option->palette.base());

    ShadowStyle shadow = dConf.input.shadow;
    const bool isInactive = dConf.differentInactive && shadow == Yosemite && widget && qobject_cast<QToolBar *>(widget->parent()) && !widget->window()->isActiveWindow();

    if (isInactive)
        shadow = Rect;

    if (mask.style() == Qt::SolidPattern || mask.style() == Qt::NoBrush)
    {
        QColor c(mask.color());
        if (dConf.input.tint.second > -1)
            c = Color::mid(c, dConf.input.tint.first, 100-dConf.input.tint.second, dConf.input.tint.second);
        if (isInactive)
            c.setAlpha(127);
        QLinearGradient lg(option->rect.topLeft(), option->rect.bottomLeft());
        lg.setStops(DSP::Settings::gradientStops(dConf.input.gradient, c));
        mask = QBrush(lg);
    }

    GFX::drawClickable(shadow, option->rect, painter, mask, dConf.input.rnd, All, option, widget);
    return true;
}

bool
Style::drawComboBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionComboBox *opt = qstyleoption_cast<const QStyleOptionComboBox *>(option);
    if (!opt)
        return true;

    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Button));
    const bool ltr(opt->direction == Qt::LeftToRight);

    QRect arrowRect(subControlRect(CC_ComboBox, opt, SC_ComboBoxArrow, widget));
    QRect frameRect(subControlRect(CC_ComboBox, opt, SC_ComboBoxFrame, widget));
    QRect iconRect; //there is no SC_ rect function for this?
    QRect textRect(frameRect);

    if (!opt->currentIcon.isNull())
    {
        const int h(frameRect.height());
        iconRect = QRect(0, 0, h, h);
        static const int iconMargin(GFX::shadowMargin(opt->editable?dConf.input.shadow:dConf.pushbtn.shadow));
        if (opt->direction == Qt::LeftToRight)
        {
            iconRect.moveLeft(frameRect.left()+iconMargin);
            textRect.setLeft(iconRect.right());
            textRect.setRight(arrowRect.left());
        }
        else if (opt->direction == Qt::RightToLeft)
        {
            iconRect.moveRight(frameRect.right()+iconMargin);
            textRect.setRight(iconRect.left());
            textRect.setLeft(arrowRect.right());
        }
    }



    if (!opt->editable)
    {
        QColor bgc(opt->palette.color(bg));
        QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
        if (isEnabled(option) && !(option->state & State_On))
        {
            int hl(Anim::Basic::level(widget));
            bgc = Color::mid(bgc, sc, Steps-hl, hl);
        }
        if (dConf.pushbtn.tint.second > -1)
            bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);
        painter->setClipRegion(QRegion(frameRect)-QRegion(arrowRect));
        QLinearGradient lg(frameRect.topLeft(), frameRect.bottomLeft());
        lg.setStops(DSP::Settings::gradientStops(dConf.pushbtn.gradient, bgc));

        GFX::drawClickable(dConf.pushbtn.shadow, opt->rect, painter, lg, dConf.pushbtn.rnd, All, option, widget);

        const QColor hc(Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1));
        QLinearGradient lga(opt->rect.topLeft(), opt->rect.bottomLeft());
        lga.setStops(DSP::Settings::gradientStops(dConf.pushbtn.gradient, hc));

        painter->setClipRect(arrowRect);
        GFX::drawClickable(dConf.pushbtn.shadow, opt->rect, painter, lga, dConf.pushbtn.rnd, All & ~(ltr?Left:Right), option, widget);
        painter->setClipping(false);
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
            QLinearGradient lg(option->rect.topLeft(), option->rect.bottomLeft());
            lg.setStops(DSP::Settings::gradientStops(dConf.input.gradient, c));
            brush = QBrush(lg);
        }
        GFX::drawClickable(dConf.input.shadow, opt->rect, painter, brush, dConf.input.rnd, All, option, widget);
    }

    drawItemPixmap(painter, iconRect, Qt::AlignCenter, opt->currentIcon.pixmap(opt->iconSize));
    QColor ac(opt->palette.color(opt->editable?QPalette::Text:QPalette::ButtonText));
    QRect a1(arrowRect.adjusted(0, 0, 0, -arrowRect.height()/2).translated(0, 1));
    QRect a2(arrowRect.adjusted(0, arrowRect.height()/2, 0, 0).translated(0, -1));
    int m(qMax<int>(2, GFX::shadowMargin(opt->editable?dConf.input.shadow:dConf.pushbtn.shadow))/2);
    a1.moveLeft(a1.left()+(ltr?-m:m));
    a2.moveLeft(a2.left()+(ltr?-m:m));

    const QComboBox *box = qobject_cast<const QComboBox *>(widget);
    const bool enabled(isEnabled(opt) && (!box || box->count()));
    const bool upDisabled(!enabled || (!box || !box->currentIndex()));
    if (upDisabled)
        ac.setAlpha(127);
    GFX::drawArrow(painter, ac, a1, North, 7, Qt::AlignCenter, !upDisabled);
    const bool downDisabled(!enabled || (!box || box->currentIndex()==box->count()-1));
    ac.setAlpha(downDisabled?127:255);
    GFX::drawArrow(painter, ac, a2, South, 7, Qt::AlignCenter, !downDisabled);
    return true;
}

bool
Style::drawComboBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionComboBox *opt = qstyleoption_cast<const QStyleOptionComboBox *>(option);
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
            rect.setLeft(rect.left()+(opt->iconSize.width()+GFX::shadowMargin(opt->editable?dConf.input.shadow:dConf.pushbtn.shadow)));
        else
            rect.setRight(rect.right()-(opt->iconSize.width()+GFX::shadowMargin(opt->editable?dConf.input.shadow:dConf.pushbtn.shadow)));
    }
    drawItemText(painter, rect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->currentText, fg);
    return true;
}

bool
Style::drawSpinBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionSpinBox *opt = qstyleoption_cast<const QStyleOptionSpinBox *>(option);
    if (!opt)
        return true;

    const bool ltr(opt->direction == Qt::LeftToRight);

//    QRect frame(subControlRect(CC_SpinBox, opt, SC_SpinBoxFrame, widget));
    QRect up(subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, widget).shrinked(2).translated(ltr?-2:2, 0));
//    up.setBottom(up.bottom()-1);
    QRect down(subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, widget).shrinked(2).translated(ltr?-2:2, 0));
//    down.setTop(down.top()+1);
//    QRect edit(subControlRect(CC_SpinBox, opt, SC_SpinBoxEditField, widget));
    QRect edit(opt->rect);

    QBrush mask(option->palette.brush(QPalette::Base));
//    QRect r = opt->rect;
//    r.setLeft(edit.right());
//    Render::drawClickable(dConf.input.shadow, r, painter, dConf.input.rnd, dConf.shadows.opacity, widget, option, &mask, 0, All&~Left);
//    mask = option->palette.brush(QPalette::Base);
    if (mask.style() < 2)
    {
        QColor c(mask.color());
        if (dConf.input.tint.second > -1)
            c = Color::mid(c, dConf.input.tint.first, 100-dConf.input.tint.second, dConf.input.tint.second);
        QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
        lg.setStops(DSP::Settings::gradientStops(dConf.input.gradient, c));
        mask = QBrush(lg);
    }

    GFX::drawClickable(dConf.input.shadow, edit, painter, mask, dConf.input.rnd, All, option, widget);
    const bool enabled(isEnabled(opt));
    const QSpinBox *box = qobject_cast<const QSpinBox *>(widget);
    QColor c = opt->palette.color(QPalette::WindowText);
    if (!enabled || (box && box->maximum() == box->value()))
        c.setAlpha(127);
    GFX::drawArrow(painter, c, up, North, dConf.arrowSize, Qt::AlignBottom|(ltr?Qt::AlignLeft:Qt::AlignRight), enabled && (box && box->minimum() != box->value()));
    if (!enabled || (box && box->minimum() == box->value()))
        c.setAlpha(127);
    else
        c.setAlpha(255);
    GFX::drawArrow(painter, c, down, South, dConf.arrowSize, Qt::AlignTop|(ltr?Qt::AlignLeft:Qt::AlignRight), enabled && (box && box->minimum() != box->value()));
    return true;
}
