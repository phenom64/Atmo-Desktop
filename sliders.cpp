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
#include <QScrollBar>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QDebug>
#include <QPainter>
#include <QSlider>
#include <QAbstractScrollArea>
#include <QTextEdit>
#include <QProgressBar>
#include <QStyleOptionProgressBar>
#include <QTextBrowser>
#include <QApplication>
#include <QPolygon>
#include <QMap>
#include <QElapsedTimer>

#include "nse.h"
#include "atmolib/gfx.h"
#include "atmolib/ops.h"
#include "atmolib/color.h"
#include "atmolib/progresshandler.h"
#include "atmolib/animhandler.h"
#include "config/settings.h"
#include "overlay.h"
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
    const qreal cutoff(0.4f);
    shine.setColorAt(0.0, QColor(255, 255, 255, 180));
    shine.setColorAt(cutoff, QColor(255, 255, 255, 20));
    shine.setColorAt(cutoff+0.0001f, QColor(255, 255, 255, 0));
    shine.setColorAt(1.0, QColor(255, 255, 255, 0));
    return shine;
}

static QElapsedTimer s_progressTimer;

bool
Style::drawScrollBar(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
//    castOpt(Slider, opt, option);
    const QStyleOptionSlider *opt = qstyleoption_cast<const QStyleOptionSlider *>(option);
    if (!opt)
        return true;

    const bool hor(opt->orientation == Qt::Horizontal),
            ltr(opt->direction == Qt::LeftToRight);

    int level(Anim::Basic::level(widget));
    QRect up(subControlRect(CC_ScrollBar, option, SC_ScrollBarSubLine, widget));
    QRect down(subControlRect(CC_ScrollBar, option, SC_ScrollBarAddLine, widget));
    QRect slider(subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget));
    QRect groove(subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));
    const QScrollBar *bar = qobject_cast<const QScrollBar *>(widget);
    const QAbstractScrollArea *area = Ops::getAncestor<const QAbstractScrollArea *>(widget);
    if (dConf.scrollers.style == 0)
    {
        QPalette::ColorRole fg(Ops::fgRole(area, QPalette::WindowText)), bg(Ops::bgRole(area, QPalette::Window));
        const int m(Ops::dpiScaled(widget, 3));
        slider.adjust(m, m, -m, -m);

        QColor bgc(opt->palette.color(bg));
        QColor fgc(opt->palette.color(fg));
        bool isKate(false);
        if (area)
        {
//            qDebug() << area << area->viewport()->foregroundRole() << area->viewport()->backgroundRole();
            bgc = area->viewport()->palette().color(area->viewport()->backgroundRole());
            fgc = area->viewport()->palette().color(area->viewport()->foregroundRole());
        }
//        else if (widget)
//        {
//            const QWidget *p = widget;
//            bgc = p->palette().color(QPalette::Window);
//            fgc = p->palette().color(QPalette::WindowText);
//        }
        else if (widget && widget->parentWidget() && widget->parentWidget()->inherits("KTextEditor::ViewPrivate"))
        {
            isKate = true;
            static QColor *c(0);
            if (!c)
            {
                QWidget *p = widget->parentWidget();
                QList<QWidget *> kids = p->findChildren<QWidget *>();
                for (int i = 0; i < kids.size(); ++i)
                {
                    if (kids.at(i)->inherits("KateViewInternal"))
                    {
                        QWidget *kateView = kids.at(i);
                        QImage img(1, 1, QImage::Format_ARGB32_Premultiplied);
                        img.fill(Qt::transparent);
                        const QRect r(kateView->rect());
                        kateView->render(&img, QPoint(), QRegion(r.right(), r.bottom(), r.right()+1, r.bottom()+1));
                        c = new QColor(QColor::fromRgba(img.pixel(QPoint(0, 0))));
                    }
                }
            }
            bgc = *c;
        }
        if (bgc.alpha() != 0xff)
            bgc = widget ? widget->parentWidget()->palette().color(QPalette::Window) : qApp->palette().color(QPalette::Window);
        if (!Color::contrast(fgc, bgc))
            fgc = Qt::white; //both most likely pretty much black so try white first...
        if (!Color::contrast(fgc, bgc))
            fgc = Qt::black;
        if (widget && (widget->inherits("QWebView") || (area && area->inherits("KHTMLView"))))
        {
            painter->fillRect(groove, bgc);
            fg = QPalette::Text;
        }
        else if (isKate)
        {
            painter->fillRect(groove, bgc);
        }
        else if (bar && bar->parentWidget() && painter->device() == bar
                 && opt->palette.color(QPalette::Window) == bar->window()->palette().color(QPalette::Window)
                 && (bgc.alpha() < 0xff || bg != QPalette::Base ))
        {
//            QWidget *p = bar->parentWidget();

            const QPoint tl(bar->mapTo(bar->window(), QPoint()));
//            bar->parentWidget()->render(painter, geo.topLeft(), geo, 0);
            GFX::drawWindowBg(painter, bar, opt->palette.color(QPalette::Window), opt->rect, tl);
        }
        else
        {
//            bool isTransparent(false);
//#if QT_VERSION >= 0x050000
//            isTransparent = pixelMetric(PM_ScrollView_ScrollBarOverlap, option, widget);
//#endif
//            if (!isTransparent)
                painter->fillRect(groove, bgc);
        }
        if (dConf.scrollers.separator
                && bg == QPalette::Base
                && area
                && area->frameShadow() == QFrame::Sunken
                && area->frameShape() == QFrame::StyledPanel)
        {
            QLine l(groove.topLeft(), groove.bottomLeft());
            if (opt->orientation == Qt::Horizontal)
            {
                l.setPoints(groove.topLeft(), groove.topRight());
                slider.setBottom(slider.bottom()+1);
            }
            else
                slider.setRight(slider.right()+1);
            const QPen saved(painter->pen());
            painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
            painter->drawLine(l);
            painter->setPen(saved);
        }
        if (bar && !bar->isSliderDown())
            fgc.setAlpha(qMin<int>(255, 85.0f+((170.0f/(float)Steps)*level)));
        GFX::drawMask(slider, painter, fgc);
//        const int rnd(qMin(slider.height(), slider.width())/2);
//        painter->drawRoundedRect(slider, rnd, rnd);
    }
    else if (dConf.scrollers.style >= 1)
    {
        if (isSunken(opt))
            level = Steps;
        const QPalette pal = QApplication::palette();
        QColor bgColor(pal.color(QPalette::Button)), fgColor(pal.color(QPalette::ButtonText));

        QLinearGradient lg(0, 0, !hor*opt->rect.width(), hor*opt->rect.height());
        lg.setStops(NSE::Settings::gradientStops(dConf.scrollers.sliderGrad, bgColor));

        ///the background
        painter->fillRect(opt->rect, lg);

        if (opt->minimum == opt->sliderValue)
            fgColor.setAlpha(127);
        GFX::drawArrow(painter, fgColor, up.translated(hor?-1:0, hor?0:-1), hor?West:North, 7, Qt::AlignCenter, fgColor.alpha() == 0xff);
        if (opt->maximum == opt->sliderValue)
            fgColor.setAlpha(127);
        else
            fgColor.setAlpha(255);
        GFX::drawArrow(painter, fgColor, down.translated(hor?1:0, hor?0:1), hor?East:South, 7, Qt::AlignCenter, fgColor.alpha() == 0xff);

        const QPen pen(painter->pen());
        const bool inView((area /*&& area->viewport()->autoFillBackground()*/ && area->frameShape() == QFrame::StyledPanel && area->frameShadow() == QFrame::Sunken)
                          || qobject_cast<const QTextEdit *>(area)
                          || (widget && widget->parentWidget() && widget->parentWidget()->inherits("KTextEditor::ViewPrivate"))); // I hate application specific hacks! what the fuck is kateview anyway?
        for (int i = 0; i < 2; ++i)
        {
            if (!i)
                painter->setPen(pal.color(QPalette::Window));
            else
                painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));

            if (inView)
            {
                if (hor)
                    painter->drawLine(opt->rect.topLeft(), opt->rect.topRight());
                else if (ltr)
                    painter->drawLine(opt->rect.topLeft(), opt->rect.bottomLeft());
                else
                    painter->drawLine(opt->rect.topRight(), opt->rect.bottomRight());
            }
            else
            {
                const QBrush brush(painter->brush());
                painter->setBrush(Qt::NoBrush);
                painter->drawRect(opt->rect.adjusted(0, 0, -1, -1));
                painter->setBrush(brush);
            }
        }
        painter->setPen(pen);

        ///the groove
        QColor bgc = pal.color(QPalette::Base);
        switch (dConf.scrollers.grooveStyle)
        {
        case 0: break;
        case 1: bgc = Color::mid(bgc, pal.color(QPalette::Text)); break;
        case 2: bgc = pal.color(QPalette::Text); break;
        default: break;
        }

        lg.setStops(Settings::gradientStops(dConf.scrollers.grooveGrad, bgc));
        const quint8 sm = GFX::shadowMargin(dConf.scrollers.grooveShadow);
        GFX::drawClickable(dConf.scrollers.grooveShadow, groove.adjusted(-sm, -sm, sm, sm), painter, lg, MaxRnd, 0, All, option, widget);

        ///the slider
        if (slider.size().isEmpty())
            return true;
        if (dConf.scrollers.style == 2)
            bgColor = Color::mid(pal.color(QPalette::Highlight), bgColor, 2, 1);
        lg.setStops(Settings::gradientStops(dConf.scrollers.sliderGrad, Color::mid(pal.color(QPalette::Highlight), bgColor, level, Steps-level)));
        slider.adjust(1, 1, -1, -1);
        quint8 roundNess((qMin(slider.width(), slider.height()) >> 1));
        QBrush brush(lg);
        QPoint offset(slider.topLeft()+QPoint(sm, sm));
        if (dConf.scrollers.style == 3)
        {
            int sz(qMin(slider.width(), slider.height()));
            if (!sz)
                return true;
            const qreal dpr = painter->device()->devicePixelRatioF();
            quint64 check((quint64)pal.color(QPalette::Highlight).rgba() | (quint64)sz << 32 | (quint64)qBound(10, qRound(dpr * 10.0), 255) << 48);
            static QMap<quint64, QPixmap> pixMap;
            if (!pixMap.contains(check))
            {
                QPixmap pix(qMax(1, qRound(sz * dpr)), qMax(1, qRound(sz * dpr)));
                pix.setDevicePixelRatio(dpr);
                pix.fill(Qt::transparent);
                QPainter p(&pix);
                QLinearGradient gradient(pix.rect().topLeft(), hor?pix.rect().bottomLeft():pix.rect().topRight());
                gradient.setStops(NSE::Settings::gradientStops(dConf.scrollers.sliderGrad, pal.color(QPalette::Highlight)));
                p.fillRect(pix.rect(), gradient);
                QRadialGradient rad(pix.rect().center()+QPoint(1, 1), pix.rect().width());
                rad.setColorAt(0, QColor(255, 255, 255, 170));
                rad.setColorAt(1, Qt::transparent);
                rad.setSpread(QGradient::ReflectSpread);
                p.setCompositionMode(QPainter::CompositionMode_Overlay);
                p.fillRect(pix.rect(), rad);
                p.end();
                pixMap.insert(check, pix);
            }
            brush = pixMap.value(check);
            const int sPos = opt->maximum-(opt->minimum+opt->sliderPosition);
            const int hLev = Anim::Basic::level(widget);
            offset = QPoint((slider.x()+sm)+(hor*hLev+(hor*sPos)), slider.y()+sm+(!hor*hLev)+(!hor*sPos));
        }

        GFX::drawMask(slider, painter, brush, roundNess, All, offset);
        const quint8 rm = GFX::shadowMargin(Raised);
        GFX::drawShadow(Raised, slider.adjusted(-rm, -rm, rm, rm), painter, isEnabled(opt), roundNess+rm);
    }
    return true;
}

bool
Style::drawScrollAreaCorner(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (dConf.scrollers.style == 0)
    {
        if (option && widget && (widget->inherits("QWebView") || widget->inherits("KHTMLView")))
        {
            painter->fillRect(option->rect, option->palette.color(QPalette::Base));
            return true;
        }
        if (const QAbstractScrollArea *area = qobject_cast<const QAbstractScrollArea *>(widget))
        {
            if (area->viewport()->autoFillBackground()
                    && area->viewport()->palette().color(QPalette::Base).alpha() == 0xff
                    && area->verticalScrollBar()->isVisible()
                    && area->horizontalScrollBar()->isVisible())
                painter->fillRect(option->rect, option->palette.color(QPalette::Base));
        }
    }
    else if (dConf.scrollers.style == 1)
    {
        const QAbstractScrollArea *area = Ops::getAncestor<const QAbstractScrollArea *>(widget);
        if (!area)
            return true;
        const bool inView((area->viewport()->autoFillBackground() && area->frameShape() == QFrame::StyledPanel && area->frameShadow() == QFrame::Sunken)
                          || qobject_cast<const QTextEdit *>(area)
                          || (widget && widget->parentWidget() && widget->parentWidget()->inherits("KateView")));
        const bool bothVisible(area->verticalScrollBar()->isVisible() && area->horizontalScrollBar()->isVisible());
        if (!bothVisible || !inView)
            return true;
        const QPen pen(painter->pen());
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        const QBrush brush(painter->brush());
        painter->setBrush(Qt::NoBrush);
        painter->setClipRect(option->rect.adjusted(0, 0, -1, -1));
        painter->drawRect(option->rect);
        painter->setClipping(false);
        painter->setPen(pen);
        painter->setBrush(brush);
    }
    return true;
}

bool
Style::drawSlider(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionSlider *opt = qstyleoption_cast<const QStyleOptionSlider *>(option);
    if (!opt)
        return false;
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::ButtonText)), bg(Ops::bgRole(widget, QPalette::Button));
    QRect slider(subControlRect(CC_Slider, option, SC_SliderHandle, widget));
    const QRect realGroove(subControlRect(CC_Slider, option, SC_SliderGroove, widget));
    QRect groove(realGroove);

    const bool hor(opt->orientation==Qt::Horizontal);
    const ShadowStyle gs(dConf.sliders.grooveShadow);
    const int d(/*gs==Carved?12:*/dConf.sliders.grooveStyle>=3?dConf.sliders.size-2:dConf.sliders.size*0.66f); //shadow size for slider and groove 'extent'
    const QPoint c(groove.center());
    if (hor)
        groove.setHeight(d);
    else
        groove.setWidth(d);
    groove.moveCenter(c);

    static const int m(GFX::shadowMargin(gs));
    groove.adjust(hor*m, !hor*m, -(hor*m), -(!hor*m)); //set the proper inset for the groove in order to account for the slider shadow

    QColor gbgc = opt->palette.color(bg);
    switch (dConf.sliders.grooveStyle)
    {
    case 0: break;
    case 1: gbgc = Color::mid(gbgc, opt->palette.color(fg)); break;
    case 2: gbgc = opt->palette.color(fg); break;
    default: gbgc = Color::mid(gbgc, opt->palette.color(fg)); break;
    }

    QLinearGradient lga(groove.topLeft(), hor?groove.bottomLeft():groove.topRight());
    lga.setStops(NSE::Settings::gradientStops(dConf.sliders.grooveGrad, gbgc));

    QRect fill(groove), unfill(groove);
    Sides sides = All, fillSides = All;
    if (dConf.sliders.fillGroove)
    {
        if (hor)
        {
            const int x(slider.center().x());
            if (opt->upsideDown)
            {
                sides &= ~Right;
                fillSides &= ~Left;
                fill.setLeft(x);
                unfill.setRight(x);
            }
            else
            {
                sides &= ~Left;
                fillSides &= ~Right;
                fill.setRight(x);
                unfill.setLeft(x);
            }
        }
        else
        {
            const int y(slider.center().y());
            if (opt->upsideDown)
            {
                sides &= ~Bottom;
                fillSides &= ~Top;
                fill.setTop(y);
                unfill.setBottom(y);
            }
            else
            {
                sides &= ~Top;
                fillSides &= ~Bottom;
                fill.setBottom(y);
                unfill.setTop(y);
            }
        }
    }
    const int hl = Anim::Basic::level(widget);
    GFX::drawClickable(gs, unfill, painter, lga, d, 0, sides, option, widget);
    if (dConf.sliders.fillGroove || dConf.sliders.grooveStyle == 4)
    {
        QLinearGradient lgh(groove.topLeft(), hor?groove.bottomLeft():groove.topRight());
        lgh.setStops(NSE::Settings::gradientStops(dConf.sliders.grooveGrad, opt->palette.color(dConf.sliders.grooveStyle == 4 ? QPalette::Base : QPalette::Highlight)));
        GFX::drawClickable(gs, fill, painter, lgh, d, 0, fillSides, option, widget);
    }

    if (opt->tickPosition)
    {
        if (hor)
        {
            painter->setClipRegion(QRegion(opt->rect)-QRegion(realGroove));
            int x, y, x2, y2;
            opt->rect.getCoords(&x, &y, &x2, &y2);

            const int available(pixelMetric(PM_SliderSpaceAvailable, option, widget));

            int interval = opt->tickInterval;
            if (interval < 1)
                interval = opt->pageStep;
            int current(opt->minimum);
            const int fudge(pixelMetric( PM_SliderLength, option, widget )/2);
            while( current <= opt->maximum )
            {
                const int position( sliderPositionFromValue( opt->minimum, opt->maximum, current, available ) + fudge );
                if (hor)
                    painter->drawLine(position, y, position, y2);
                else
                    painter->drawLine(x, position, x2, position);
                current += interval;
            }
            painter->setClipping(false);
        }
        else //vert...
        {

        }
    }
    QColor bgc(opt->palette.color(bg));
//    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
//    if (isEnabled(option))
//        bgc = Color::mid(bgc, sc, Steps-hl, hl);

    QGradient g;
    if (dConf.sliders.metallic)
    {
        const int add = !(slider.width() & 1) * 1;
        QConicalGradient cg(slider.center()+QPoint(add,add), -45);
//        cg.setStops(NSE::Settings::gradientStops(dConf.sliders.sliderGrad, bgc));
        const QColor light = Color::mid(bgc, Qt::white, 2, 1);
        const QColor dark = Color::mid(bgc, Qt::black, 2, 1);
        cg.setColorAt(0, light);
        cg.setColorAt(0.25f, dark);
        cg.setColorAt(0.5f, light);
        cg.setColorAt(0.75f, dark);
        cg.setColorAt(1, light);
        g = cg;
    }
    else
    {
        QLinearGradient lg(slider.topLeft(), slider.bottomLeft());
        lg.setStops(NSE::Settings::gradientStops(dConf.sliders.sliderGrad, bgc));
        g = lg;
    }
    const ShadowStyle sliderShadow(dConf.sliders.grooveShadow==Rect?Rect:dConf.sliders.grooveShadow==-1?-1:Raised);
    GFX::drawClickable(sliderShadow, slider, painter, g, MaxRnd, hl, All, 0, widget);

    if (dConf.sliders.dot && !dConf.sliders.metallic)
    {
        const int ds(4+(dConf.sliders.size&1)/*slider.height()/3*/);
        QRect dot(0, 0, ds, ds);
        dot.moveCenter(slider.center());
        GFX::drawRadioMark(painter, Color::mid(opt->palette.color(fg), opt->palette.color(bg), 3, 1), dot, true);
    }
    return true;
}
