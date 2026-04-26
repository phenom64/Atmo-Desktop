/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**************************************************************************
*   Based on styleproject, Copyright (C) 2013 by Robert Metsaranta        *
*   therealestrob@gmail.com                                              *
***************************************************************************/

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
#include <QProgressBar>
#include <QStyleOptionProgressBar>
#include <QElapsedTimer>
#include <QPainterPath>

#include "nse.h"
#include "atmolib/gfx.h"
#include "overlay.h"
#include "atmolib/ops.h"
#include "atmolib/color.h"
#include "atmolib/animhandler.h"
#include "config/settings.h"
#include "atmolib/fx.h"

using namespace NSE;

/* gel helpers */
static inline QLinearGradient gelCylinder(const QRect &r, const QColor &base, const bool invert = false)
{
    QLinearGradient lg(r.topLeft(), r.bottomLeft());
    if (!invert)
    {
        lg.setColorAt(0.0, base.lighter(150));
        lg.setColorAt(0.5, base);
        lg.setColorAt(0.51, base.darker(110));
        lg.setColorAt(1.0, base.darker(130));
    }
    else
    {
        /* pressed state: flip the gradient to push light downwards */
        lg.setColorAt(0.0, base.darker(130));
        lg.setColorAt(0.49, base.darker(110));
        lg.setColorAt(0.5, base);
        lg.setColorAt(1.0, base.lighter(150));
    }
    return lg;
}

static inline QLinearGradient gelShine(const QRect &r)
{
    QLinearGradient shine(r.topLeft(), r.bottomLeft());
    const qreal cutoff(0.45f); // Slightly adjusted for better gloss
    shine.setColorAt(0.0, QColor(255, 255, 255, 210));
    shine.setColorAt(cutoff, QColor(255, 255, 255, 50));
    shine.setColorAt(cutoff+0.0001f, QColor(255, 255, 255, 0));
    shine.setColorAt(1.0, QColor(255, 255, 255, 0));
    return shine;
}

static QElapsedTimer s_progressTimer;

bool
Style::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (qobject_cast<const QComboBox *>(widget?widget->parentWidget():0))
        return true;
    if (widget && widget->objectName() == QStringLiteral("qt_spinbox_lineedit"))
        return true;

    QBrush mask(option->palette.base());

    const quint8 rnd = inUno(qobject_cast<QToolBar *>(widget ? widget->parentWidget() : 0)) ? dConf.input.unoRnd : dConf.input.rnd;
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
        lg.setStops(NSE::Settings::gradientStops(dConf.input.gradient, c));
        mask = QBrush(lg);
    }

    GFX::drawClickable(shadow, option->rect, painter, mask, rnd, 0, All, option, widget);
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
    QRect iconRect;
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
        QColor fgc(opt->palette.color(Ops::fgRole(widget, QPalette::ButtonText)));
        const int hl = Anim::Basic::level(widget);
        if (dConf.pushbtn.tint.second > -1)
            bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);
        QLinearGradient lg(frameRect.topLeft(), frameRect.bottomLeft());
        lg.setStops(NSE::Settings::gradientStops(dConf.pushbtn.gradient, bgc));

        QRect main(opt->rect);
        if (ltr)
            main.setRight(arrowRect.left()-1);
        else
            main.setLeft(arrowRect.right()+1);
        GFX::drawClickable(dConf.pushbtn.shadow, main, painter, lg, dConf.pushbtn.rnd, hl, All & ~(ltr?Right:Left), option, widget);

        QColor hc;
        if (dConf.pushbtn.shadow == Yosemite || dConf.pushbtn.shadow == ElCapitan)
            hc = opt->palette.color(QPalette::Highlight);
        else
            hc = Color::mid(bgc, fgc, 10, 1);
        QLinearGradient lga(opt->rect.topLeft(), opt->rect.bottomLeft());
        lga.setStops(NSE::Settings::gradientStops(dConf.pushbtn.gradient, hc));

        GFX::drawClickable(dConf.pushbtn.shadow, arrowRect, painter, lga, dConf.pushbtn.rnd, hl, All & ~(ltr?Left:Right), option, widget);
        const quint8 m(GFX::shadowMargin(dConf.pushbtn.shadow));
        if (ltr)
        {
            const Sides sides =  All & ~(ltr?Left:Right);
            QRect r = arrowRect.sShrinked(m);
            r.setWidth(1);
            painter->fillRect(r, QColor(0, 0, 0, dConf.shadows.opacity>>(!isEnabled(opt))));
            r.translate(1, 0);
            painter->fillRect(r, QColor(255, 255, 255, dConf.shadows.illumination>>(!isEnabled(opt))));
        }
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
            lg.setStops(NSE::Settings::gradientStops(dConf.input.gradient, c));
            brush = QBrush(lg);
        }
        GFX::drawClickable(dConf.input.shadow, opt->rect, painter, brush, dConf.input.rnd, 0, All, option, widget);
    }

    drawItemPixmap(painter, iconRect, Qt::AlignCenter, opt->currentIcon.pixmap(opt->iconSize));
    QColor ac(opt->palette.color(opt->editable?QPalette::Text:QPalette::ButtonText));
    if (dConf.pushbtn.shadow == Yosemite || dConf.pushbtn.shadow == ElCapitan)
        ac = opt->palette.color(QPalette::HighlightedText);
    QRect a1(arrowRect.adjusted(0, 0, 0, -(arrowRect.height()/2+2)));
    QRect a2(arrowRect.adjusted(0, arrowRect.height()/2+2, 0, 0));
    int m(qMax<int>(4, GFX::shadowMargin(opt->editable?dConf.input.shadow:dConf.pushbtn.shadow))/2);
    a1.moveLeft(a1.left()+(ltr?-m:m));
    a2.moveLeft(a2.left()+(ltr?-m:m));

    const QComboBox *box = qobject_cast<const QComboBox *>(widget);
    const bool enabled(isEnabled(opt) && (!box || box->count()));
    const bool upDisabled(!enabled || (!box || !box->currentIndex()));
    if (upDisabled)
        ac.setAlpha(127);
    GFX::drawArrow(painter, ac, a1, North, 7, Qt::AlignHCenter|Qt::AlignBottom, !upDisabled);
    const bool downDisabled(!enabled || (!box || box->currentIndex()==box->count()-1));
    ac.setAlpha(downDisabled?127:255);
    GFX::drawArrow(painter, ac, a2, South, 7, Qt::AlignHCenter|Qt::AlignTop, !downDisabled);
    return true;
}

bool
Style::drawComboBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionComboBox *opt = qstyleoption_cast<const QStyleOptionComboBox *>(option);
    if (!opt || opt->editable)
        return true;
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::ButtonText));
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

    QRect up(subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, widget).shrinked(2).translated(ltr?-2:2, 0));
    QRect down(subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, widget).shrinked(2).translated(ltr?-2:2, 0));
    QRect edit(opt->rect);

    QBrush mask(option->palette.brush(QPalette::Base));
    if (mask.style() < 2)
    {
        QColor c(mask.color());
        if (dConf.input.tint.second > -1)
            c = Color::mid(c, dConf.input.tint.first, 100-dConf.input.tint.second, dConf.input.tint.second);
        QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
        lg.setStops(NSE::Settings::gradientStops(dConf.input.gradient, c));
        mask = QBrush(lg);
    }

    GFX::drawClickable(dConf.input.shadow, edit, painter, mask, dConf.input.rnd, 0, All, option, widget);
    const bool enabled(isEnabled(opt));
    const QSpinBox *box = qobject_cast<const QSpinBox *>(widget);
    QColor c = opt->palette.color(QPalette::Text);
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

 bool NSE::Style::drawRubberBand(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    if (!option)
        return false;

    QColor c = option->palette.color(QPalette::Highlight);
    c.setAlpha(63);

    GFX::drawClickable(Raised, option->rect, painter, c, 5, Steps);
    return true;
}

// --------------------------------------------------------------------------------
//  PROGRESS BAR ENGINE
// --------------------------------------------------------------------------------

QRect
Style::progressContents(const QStyleOption *opt, const QWidget *widget) const
{
    const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(opt);
    const QStyleOptionProgressBar *barv2 = qstyleoption_cast<const QStyleOptionProgressBar *>(opt);
    if (!bar)
        return QRect();
    if (bar->minimum == 0 && bar->maximum == 0) //busy
        return bar->rect;
    const QRect optrect(subElementRect(SE_ProgressBarGroove, opt, widget));
    const bool hor(!barv2 || (barv2->state & State_Horizontal));
    qreal d((qreal)(hor?optrect.width():optrect.height())/(qreal)bar->maximum);
    int progress(d*bar->progress);
    int w(hor?progress:optrect.width());
    int h(hor?optrect.height():progress);
    int x(optrect.x()), y(optrect.y());
    QRect r(x, hor?y:(y+optrect.height())-progress, w, h);
    if (barv2 && barv2->invertedAppearance)
    {
        if (hor)
            r.moveRight(optrect.right());
        else
            r.moveTop(optrect.top());
    }
    return visualRect(opt->direction, optrect, r);
}

bool
Style::drawProgressBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionProgressBar *opt = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
    if (!opt)
        return true;
    drawProgressBarGroove(option, painter, widget);
    drawProgressBarContents(option, painter, widget);
    drawProgressBarLabel(option, painter, widget);
    return true;
}

bool
Style::drawProgressBarGroove(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionProgressBar *opt = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
    if (!opt)
        return true;

    // Force Raised shadow for that "floating" Aqua look
    GFX::drawShadow(Raised, opt->rect, painter, isEnabled(opt), dConf.progressbars.rnd);

    QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget));
    // Fill the empty track with a faint sunken derivative or just base
    QColor base = opt->palette.color(QPalette::Dark); 
    base.setAlpha(50);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(groove, dConf.progressbars.rnd, dConf.progressbars.rnd);
    painter->fillPath(path, base);
    painter->restore();

    return true;
}

bool
Style::drawProgressBarContents(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionProgressBar *opt = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
    if (!opt)
        return true;
    if (!opt->progress && !(opt->minimum == 0 && opt->maximum == 0))
        return true;
    
    // We MUST be in Aqua mode (or forced to it by this update)
    // -------------------------------------------------------
    if (!s_progressTimer.isValid())
            s_progressTimer.start();
    
    const bool busy(opt->minimum == 0 && opt->maximum == 0);
    QRect cont = progressContents(opt, widget);
    QRect groove = subElementRect(SE_ProgressBarGroove, opt, widget);
    const QColor h(opt->palette.color(QPalette::Highlight));

    // Ensure we don't bleed out of the groove
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setClipRect(groove);

    QRect bar(cont); 
    
    // Create Rounded Path
    QPainterPath path;
    const int rnd(qMax<int>(0, dConf.progressbars.rnd));
    path.addRoundedRect(bar, rnd, rnd);

    // 1. Base Liquid Cylinder
    painter->fillPath(path, gelCylinder(bar, h));

    // 2. Texture & Animation
    const qint64 elapsed(s_progressTimer.elapsed());
    const int stripeSize = 32; // Defined width of one stripe pattern cycle

    if (busy)
    {
        // Indeterminate: "Candy Cane" - 45 degree stripes (White/Transparent)
        // This makes the "barber pole" effect.
        
        QLinearGradient stripeGrad(0, 0, stripeSize, 0);
        stripeGrad.setSpread(QGradient::RepeatSpread);
        
        // Crisp white stripe logic
        QColor white(255, 255, 255, 160);
        QColor trans(255, 255, 255, 0);

        stripeGrad.setColorAt(0.0, white);
        stripeGrad.setColorAt(0.5, white);
        stripeGrad.setColorAt(0.501, trans);
        stripeGrad.setColorAt(1.0, trans);

        QBrush stripeBrush(stripeGrad);
        QTransform t;
        t.rotate(45); // Standard barber pole angle
        // Animate shift
        t.translate(-qreal((elapsed / 15) % stripeSize), 0);
        stripeBrush.setTransform(t);
        
        painter->fillPath(path, stripeBrush);
    }
    else
    {
        // Determinate: Vertical Ribbed Effect (Darker Bands) inside the liquid
        // "Alive" phase shift
        
        QLinearGradient ribGrad(0, 0, stripeSize, 0);
        ribGrad.setSpread(QGradient::RepeatSpread);
        
        QColor darkBand(h.darker(115)); // Slightly darker than base
        QColor baseBand(h); // Base color
        
        // Smooth sine-like oscillation or sharp bands? Aqua uses soft vertical bands.
        ribGrad.setColorAt(0.0, darkBand);
        ribGrad.setColorAt(0.5, baseBand);
        ribGrad.setColorAt(1.0, darkBand);
        
        QBrush ribBrush(ribGrad);
        QTransform t;
        // Slower animation for determinate
        t.translate(-qreal((elapsed / 60) % stripeSize), 0); 
        ribBrush.setTransform(t);
        
        // We use CompositionMode_Multiply or similar if we want to shade the existing cylinder,
        // but simple over-fill with alpha modulation works too.
        painter->save();
        painter->setCompositionMode(QPainter::CompositionMode_Multiply);
        painter->fillPath(path, ribBrush);
        painter->restore();
    }

    // 3. Top Specular Highlight (The Glass Tube Effect)
    // This goes OVER the stripes/ribs to make them look like they are inside the tube.
    painter->fillPath(path, gelShine(bar));

    // 4. Subtle Inner Rim/Stroke
    painter->setPen(QPen(h.darker(140), 1));
    painter->drawPath(path);

    painter->restore();
    return true;
}

 bool NSE::Style::drawProgressBarLabel(const QStyleOption *option, QPainter *painter, const QWidget *) const
{
    const QStyleOptionProgressBar *opt = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
    if (!opt || opt->text.isEmpty())
        return true;

    // Use a drop shadow for the text to ensure readability on the glass
    QPalette::ColorRole fgRole = QPalette::HighlightedText; 
    // Aqua progress text is often white with shadow, or black if outside.
    // Assuming centered text inside bar for now.
    
    painter->save();
    // Center text
    painter->setPen(QPen(QColor(0,0,0,128))); // Text Drop Shadow
    painter->drawText(opt->rect.translated(0, 1), Qt::AlignCenter, opt->text);
    
    painter->setPen(opt->palette.color(fgRole));
    painter->drawText(opt->rect, Qt::AlignCenter, opt->text);
    painter->restore();
    
    return true;
}
