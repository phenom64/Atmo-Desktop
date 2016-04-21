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
#include <QStyleOptionProgressBarV2>
#include <QTextBrowser>
#include <QApplication>
#include <QPolygon>
#include <QMap>

#include "dsp.h"
#include "stylelib/gfx.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/progresshandler.h"
#include "stylelib/animhandler.h"
#include "config/settings.h"
#include "overlay.h"
#include "stylelib/fx.h"

using namespace DSP;

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
        const int m(3);
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
        lg.setStops(DSP::Settings::gradientStops(dConf.scrollers.sliderGrad, bgColor));

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
        const bool inView((area && area->viewport()->autoFillBackground() && area->frameShape() == QFrame::StyledPanel && area->frameShadow() == QFrame::Sunken)
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
            quint64 check((quint64)pal.color(QPalette::Highlight).rgba()|(quint64)sz << 32);
            QMap<quint64, QPixmap> pixMap;
            if (!pixMap.contains(check))
            {
                QPixmap pix(sz, sz);
                pix.fill(Qt::transparent);
                QPainter p(&pix);
                QLinearGradient gradient(pix.rect().topLeft(), hor?pix.rect().bottomLeft():pix.rect().topRight());
                gradient.setStops(DSP::Settings::gradientStops(dConf.scrollers.sliderGrad, pal.color(QPalette::Highlight)));
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
    lga.setStops(DSP::Settings::gradientStops(dConf.sliders.grooveGrad, gbgc));

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
        lgh.setStops(DSP::Settings::gradientStops(dConf.sliders.grooveGrad, opt->palette.color(dConf.sliders.grooveStyle == 4 ? QPalette::Base : QPalette::Highlight)));
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
//        cg.setStops(DSP::Settings::gradientStops(dConf.sliders.sliderGrad, bgc));
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
        lg.setStops(DSP::Settings::gradientStops(dConf.sliders.sliderGrad, bgc));
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

QRect
Style::progressContents(const QStyleOption *opt, const QWidget *widget) const
{
    const QStyleOptionProgressBar *bar = qstyleoption_cast<const QStyleOptionProgressBar *>(opt);
    const QStyleOptionProgressBarV2 *barv2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(opt);
    if (!bar)
        return QRect();
    if (bar->minimum == 0 && bar->maximum == 0) //busy
        return bar->rect;
    const QRect optrect(subElementRect(SE_ProgressBarGroove, opt, widget));
    const bool hor(!barv2 || barv2->orientation == Qt::Horizontal);
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
Style::drawProgressBarContents(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionProgressBar *opt = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
    if (!opt)
        return true;
    if (!opt->progress && !(opt->minimum == 0 && opt->maximum == 0))
        return true;

    const QStyleOptionProgressBarV2 *optv2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option);
    const bool hor(!optv2 || optv2->orientation == Qt::Horizontal);
    const QRect cont(progressContents(opt, widget)); //The progress indicator of a QProgressBar.
    const QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget));
    const QColor h(opt->palette.color(QPalette::Highlight));

    if (dConf.progressbars.textPos)
    {
        const int sz(hor?cont.height()/3:cont.width()/3);
        QRect c(cont.adjusted(!hor*sz, hor*sz, -(!hor*sz), -(hor*sz)));
        GFX::drawMask(c, painter, h, dConf.progressbars.rnd);
        return true;
    }

    static const quint8 sm = GFX::shadowMargin(dConf.progressbars.shadow);
    const quint64 s((hor?groove.height():groove.width())-sm*2);
    quint64 thing((quint64)h.rgba()|(s << 32));
    if (optv2)
        thing |= (quint64(optv2->orientation) << 48);
    static QMap<quint64, QPixmap> s_pixMap;
    if (!s_pixMap.contains(thing))
    {
        QPixmap pix(qMax<int>(8, dConf.progressbars.stripeSize), s);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        QLinearGradient lg(pix.rect().topLeft(), pix.rect().bottomLeft());
        lg.setStops(Settings::gradientStops(dConf.progressbars.gradient, h));
        p.fillRect(pix.rect(), lg);
        if (dConf.progressbars.stripeSize)
        {
            QColor stripe(opt->palette.color(QPalette::Base));
            const int fourthW(pix.width()/4);
            const int fourthH(pix.height()/4);
            const int w(pix.width());
            const int h(pix.height());
            const int topRight[] = { w-fourthW,0, w,0, w,fourthH };
            const int main[] = { 0,0, fourthW,0, w,h-fourthH, w,h, w-fourthW,h, 0,fourthH };
            const int bottomLeft[] = { 0,h-fourthH, fourthW,h, 0,h };
            p.setPen(Qt::NoPen);

            QLinearGradient grad(pix.rect().topLeft(), pix.rect().bottomLeft());
            grad.setStops(Settings::gradientStops(dConf.progressbars.gradient, stripe));

            p.setBrush(grad);
            p.setRenderHint(QPainter::Antialiasing);
            p.setCompositionMode(QPainter::CompositionMode_HardLight);
            p.drawPolygon(QPolygon(3, topRight));
            p.drawPolygon(QPolygon(6, main));
            p.drawPolygon(QPolygon(3, bottomLeft));
            p.end();
        }
        if (!hor)
        {
            QTransform t;
            int d(s/2);
            t.translate(d, d);
            t.rotate(90);
            t.translate(-d, -d);
            pix = pix.transformed(t);
        }
        s_pixMap.insert(thing, pix);
    }

    static QMap<quint64, int> s_busyMap;
    const bool busy(opt&&opt->minimum==0&&opt->maximum==0);
    int offset = qMax<int>(dConf.progressbars.stripeSize, 8u);
    if (s_busyMap.contains((quint64)widget))
        offset = s_busyMap.value((quint64)widget);

    painter->setClipRect(cont);
    GFX::drawClickable(dConf.progressbars.shadow, groove, painter, s_pixMap.value(thing), dConf.progressbars.rnd, 0, All, option, widget, QPoint((groove.x()+sm)+busy*offset, groove.y()+sm));
    painter->setClipping(false);

    s_busyMap.insert((quint64)widget, offset?offset-1:qMax<int>(dConf.progressbars.stripeSize, 8u));
    return true;
}

bool
Style::drawProgressBarGroove(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionProgressBarV2 *opt = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option);
    if (!opt)
        return true;

    const QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget)); //The groove where the progress indicator is drawn in a QProgressBar.
    const bool hor(opt->orientation == Qt::Horizontal);
    if (dConf.progressbars.textPos)
    {
        const int sz(hor?groove.height()/3:groove.width()/3);
        QRect g(groove.adjusted(!hor*sz, hor*sz, -(!hor*sz), -(hor*sz)));
        QColor c(opt->palette.color(widget?widget->foregroundRole():QPalette::WindowText));
        c.setAlpha(63);
        GFX::drawMask(g, painter, c, dConf.progressbars.rnd);
        return true;
    }
    QRect cont(progressContents(opt, widget)); //The progress indicator of a QProgressBar.
    painter->setClipRegion(QRegion(groove)-QRegion(cont));

    QLinearGradient lg(groove.topLeft(), hor?groove.bottomLeft():groove.topRight());
    lg.setStops(Settings::gradientStops(dConf.input.gradient, opt->palette.color(QPalette::Base)));
    GFX::drawClickable(dConf.progressbars.shadow, groove, painter, lg, dConf.progressbars.rnd, 0, All, option, widget);
    painter->setClipping(false);
    return true;
}

bool
Style::drawProgressBarLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionProgressBar *opt = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
    const QStyleOptionProgressBarV2 *optv2 = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(option);
    if (!opt || opt->text.isEmpty() || (dConf.progressbars.txtHover && !isMouseOver(opt) && !dConf.progressbars.textPos))
        return true;

    painter->save();

    const QPalette::ColorRole fg(dConf.progressbars.textPos == 0 ? QPalette::Text : QPalette::WindowText);
    QRect optrect(opt->rect/*subElementRect(SE_ProgressBarGroove, opt, widget)*/); //The groove where the progress indicator is drawn in a QProgressBar.
    QRect cont(progressContents(opt, widget)); //The progress indicator of a QProgressBar.
    QRect label(subElementRect(SE_ProgressBarLabel, opt, widget)); //The text label of a QProgressBar.
    const bool hor(!optv2 || optv2->orientation == Qt::Horizontal);
    const float w_2(optrect.width()/2.0f), h_2(optrect.height()/2.0f);
#define TRANSLATE(_BOOL_) \
    if (!hor) \
    { \
        painter->translate(w_2, h_2); \
        painter->rotate(_BOOL_?-90.0f:90.0f); \
        painter->translate(-w_2, -h_2); \
        const QPoint center(optrect.center()); \
        QSize osz(optrect.size()); \
        osz.transpose(); \
        optrect.setSize(osz); \
        optrect.moveCenter(center); \
        QSize lsz(label.size()); \
        lsz.transpose(); \
        label.setSize(lsz); \
        label.moveCenter(optrect.center()); \
        if (dConf.progressbars.textPos == 1) \
            label.moveLeft(optrect.left()); \
        else if (dConf.progressbars.textPos == 2) \
            label.moveRight(optrect.right()); \
    }
    painter->setClipRegion(QRegion(optrect)-QRegion(cont));
    const bool btt(optv2 && optv2->bottomToTop);
    TRANSLATE(btt);
    drawItemText(painter, label, Qt::AlignCenter, opt->palette, isEnabled(opt), opt->text, fg);
    TRANSLATE(!btt);
    painter->setClipRegion(QRegion(cont));
    TRANSLATE(btt);
    if (!dConf.progressbars.textPos && dConf.progressbars.stripeSize)
    {
        QImage img(label.size(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter p(&img);
        p.setFont(painter->font());
        p.setPen(Color::lum(opt->palette.color(QPalette::Highlight))>Color::lum(opt->palette.color(QPalette::HighlightedText))?Qt::white:Qt::black);
        p.drawText(img.rect(), Qt::AlignCenter|Qt::TextHideMnemonic, opt->text);
        p.end();
        FX::expblur(img, 3);
        painter->drawImage(label.topLeft(), img);
    }
//    else
        drawItemText(painter, label, Qt::AlignCenter, opt->palette, isEnabled(opt), opt->text, QPalette::HighlightedText);
    TRANSLATE(!btt);
#undef TRANSLATE
    painter->restore();
    return true;
}
