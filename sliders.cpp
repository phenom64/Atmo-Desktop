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

#include "styleproject.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/progresshandler.h"
#include "stylelib/animhandler.h"
#include "config/settings.h"

bool
StyleProject::drawScrollBar(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
//    castOpt(Slider, opt, option);
    const QStyleOptionSlider *opt = qstyleoption_cast<const QStyleOptionSlider *>(option);
    if (!opt)
        return true;

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
        const int m(2);
        slider.adjust(m, m, -m, -m);

        QColor bgc(opt->palette.color(bg));

        if (widget && (widget->inherits("QWebView") || (area && area->inherits("KHTMLView"))))
        {
            painter->fillRect(groove, option->palette.color(QPalette::Base));
            fg = QPalette::Text;
        }
        else if (bar && painter->device() == bar
                 && opt->palette.color(QPalette::Window) == bar->window()->palette().color(QPalette::Window)
                 && (bgc.alpha() < 0xff || bg != QPalette::Base ))
        {
            const QRect geo(bar->mapTo(bar->window(), QPoint()), bar->size());
            bar->window()->render(painter, geo.topLeft(), geo, QWidget::DrawWindowBackground);
        }
        else
            painter->fillRect(groove, bgc);

        QColor fgc(opt->palette.color(fg));

        if (bg == QPalette::Base
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
            painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity*255.0f));
            painter->drawLine(l);
            painter->setPen(saved);
        }
        if (bar && !bar->isSliderDown())
            fgc.setAlpha(85.0f+((170.0f/(float)STEPS)*level));
        Render::renderMask(slider, painter, fgc);
    }
    else if (dConf.scrollers.style == 1)
    {
        if (opt->SUNKEN)
            level = STEPS;
        const QPalette pal = QApplication::palette();
        const QColor bgColor(Color::mid(pal.color(QPalette::Highlight), pal.color(QPalette::Window), level, STEPS-level)),
                fgColor(Color::mid(pal.color(QPalette::HighlightedText), pal.color(QPalette::WindowText), level, STEPS-level));

        const bool hor(opt->orientation == Qt::Horizontal),
                ltr(opt->direction == Qt::LeftToRight);

        QLinearGradient lg(0, 0, !hor*opt->rect.width(), hor*opt->rect.height());
        const QGradientStops sliderStops(Settings::gradientStops(dConf.scrollers.sliderGrad, bgColor));
        lg.setStops(sliderStops);
        painter->fillRect(opt->rect, lg);

        Ops::drawArrow(painter, fgColor, up, hor?Ops::Left:Ops::Up, 7);
        Ops::drawArrow(painter, fgColor, down, hor?Ops::Right:Ops::Down, 7);

        groove.setBottom(groove.bottom()+1);
        lg.setStops(Settings::gradientStops(dConf.scrollers.grooveGrad, pal.color(QPalette::Base)));
        QBrush bg(lg);
        Render::drawClickable(Render::Sunken,
                              groove.adjusted(-!hor, -hor, !hor, hor),
                              painter,
                              32,
                              dConf.shadows.opacity,
                              widget,
                              opt,
                              &bg);
        lg.setStops(sliderStops);

        const bool inView((area && area->viewport()->autoFillBackground() && area->frameShape() == QFrame::StyledPanel && area->frameShadow() == QFrame::Sunken)
                          || qobject_cast<const QTextEdit *>(area)
                          || (widget && widget->parentWidget() && widget->parentWidget()->inherits("KateView"))); // I hate application specific hacks! what the fuck is kateview anyway?

        Render::renderShadow(Render::Raised, slider.adjusted(-1, -1, 1, 1), painter, 32, Render::All, dConf.shadows.opacity);
        Render::renderMask(slider.adjusted(!(inView && ltr && !hor), 1, -!(inView && ltr && !hor), -(!hor*2)), painter, lg);

        const QPen pen(painter->pen());
        for (int i = 0; i < 2; ++i)
        {
            if (!i)
                painter->setPen(pal.color(QPalette::Window));
            else
                painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity*255.0f));

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
    }
    return true;
}

bool
StyleProject::drawScrollAreaCorner(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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
                          || (widget && widget->parentWidget() && widget->parentWidget()->inherits("KateView"))); // I hate application specific hacks! what the fuck is kateview anyway?
        const bool bothVisible(area->verticalScrollBar()->isVisible() && area->horizontalScrollBar()->isVisible());
        if (!bothVisible || !inView)
            return true;
        const QPen pen(painter->pen());
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity*255.0f));
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
StyleProject::drawSlider(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionSlider *opt = qstyleoption_cast<const QStyleOptionSlider *>(option);
    if (!opt)
        return false;
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::WindowText)), bg(Ops::bgRole(widget, QPalette::Window));
    QRect slider(subControlRect(CC_Slider, option, SC_SliderHandle, widget));
    const QRect realGroove(subControlRect(CC_Slider, option, SC_SliderGroove, widget));
    QRect groove(realGroove);

    const bool hor(opt->orientation==Qt::Horizontal);
    const Render::Shadow gs(dConf.sliders.grooveShadow);
    int d(/*gs==Render::Carved?12:*/dConf.sliders.size/2); //shadow size for slider and groove 'extent'
    const QPoint c(groove.center());
    if (hor)
        groove.setHeight(d);
    else
        groove.setWidth(d);
    groove.moveCenter(c);
    d/=2;
    groove.adjust(hor*d, !hor*d, -(hor*d), -(!hor*d)); //set the proper inset for the groove in order to account for the slider shadow

    QColor bgc(opt->palette.color(bg));
    const QColor grooveBg(Color::mid(opt->palette.color(fg), opt->palette.color(bg), 3, 1));
    QLinearGradient lga(0, 0, !hor*Render::maskWidth(gs, groove.width()), hor*Render::maskHeight(gs, groove.height()));
    lga.setStops(Settings::gradientStops(dConf.sliders.grooveGrad, grooveBg));
    QBrush amask(lga);

    QRect clip(groove);
    if (dConf.sliders.fillGroove)
    {
        if (opt->orientation == Qt::Horizontal)
        {
            if (opt->upsideDown)
                clip.setLeft(slider.center().x());
            else
                clip.setRight(slider.center().x());
        }
        else
        {
            if (opt->upsideDown)
                clip.setTop(slider.center().y());
            else
                clip.setBottom(slider.center().y());
        }
    }
    if (dConf.sliders.fillGroove)
        painter->setClipRegion(QRegion(groove)-QRegion(clip));
    Render::drawClickable(gs, groove, painter, d, dConf.shadows.opacity, widget, option, &amask);
    if (dConf.sliders.fillGroove)
    {
        painter->setClipRect(clip);
        QLinearGradient lgh(0, 0, !hor*Render::maskWidth(gs, groove.width()), hor*Render::maskHeight(gs, groove.height()));
        lgh.setStops(Settings::gradientStops(dConf.sliders.grooveGrad, opt->palette.color(QPalette::Highlight)));
        QBrush hmask(lgh);
        Render::drawClickable(gs, groove, painter, d, dConf.shadows.opacity, widget, option, &hmask);
    }
    painter->setClipping(false);

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
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED)
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }

    QLinearGradient lg(0, 0, 0, Render::maskHeight(Render::Raised, dConf.sliders.size));
    lg.setStops(Settings::gradientStops(dConf.sliders.sliderGrad, bgc));
    QBrush mask(lg);
    Render::drawClickable(dConf.pushbtn.shadow, slider, painter, dConf.sliders.size/2, dConf.shadows.opacity, widget, option, &mask);

    if (dConf.sliders.dot)
    {
        const int ds(slider.height()/3);
        const QRect dot(slider.adjusted(ds, ds, -ds, -ds));
        QLinearGradient dg(dot.topLeft(), dot.bottomLeft());
        dg.setColorAt(0.0f, Color::mid(opt->palette.color(fg), opt->palette.color(bg), 3, 1));
        dg.setColorAt(1.0f, Color::mid(opt->palette.color(fg), opt->palette.color(bg)));
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(dg);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(dot);
        painter->restore();
    }
    return true;
}

QRect
StyleProject::progressContents(const QStyleOption *opt, const QWidget *widget) const
{
    castOpt(ProgressBar, bar, opt);
    castOpt(ProgressBarV2, barv2, opt);
    if (!bar)
        return QRect();
    const bool hor(!barv2 || barv2->orientation == Qt::Horizontal);
    qreal d((qreal)(hor?opt->rect.width():opt->rect.height())/(qreal)bar->maximum);
    int progress(d*bar->progress);
    int w(hor?progress:bar->rect.width());
    int h(hor?bar->rect.height():progress);
    int x(bar->rect.x()), y(bar->rect.y());
    QRect r(x, hor?y:(y+bar->rect.height())-progress, w, h);
    if (barv2 && barv2->invertedAppearance)
    {
        if (hor)
            r.moveRight(opt->rect.right());
        else
            r.moveTop(opt->rect.top());
    }
    if (bar->minimum == bar->maximum)
        if (castObj(const QProgressBar *, pBar, widget))
        {
            int s(qMin(bar->rect.height(), bar->rect.width()));
            r.setSize(QSize(s, s));

            if (hor)
                r.moveLeft(ProgressHandler::busyValue(pBar));
            else
                r.moveBottom(pBar->height()-ProgressHandler::busyValue(pBar));
        }
    return visualRect(opt->direction, opt->rect, r);
}

bool
StyleProject::drawProgressBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ProgressBar, opt, option);
    if (!opt)
        return true;
    drawProgressBarGroove(option, painter, widget);
    drawProgressBarContents(option, painter, widget);
    drawProgressBarLabel(option, painter, widget);
    return true;
}

bool
StyleProject::drawProgressBarContents(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ProgressBar, opt, option);
    if (!opt)
        return true;
    if (!opt->progress && !(opt->minimum == 0 && opt->maximum == 0))
        return true;

    painter->save();
    QRect cont(progressContents(opt, widget)); //The progress indicator of a QProgressBar.
    const QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget));
    const QColor h(opt->palette.color(QPalette::Highlight));
    castOpt(ProgressBarV2, optv2, option);
    const bool hor(!optv2 || optv2->orientation == Qt::Horizontal);
//    const QRect mask(Render::maskRect(dConf.progressbars.shadow, cont, Render::All));
    const quint64 s((hor?groove.height():groove.width())+2);
    const QPalette::ColorRole /*fg(Ops::fgRole(widget, QPalette::Text)),*/ bg(Ops::bgRole(widget, QPalette::Base));

    quint64 thing(h.rgba());
    thing |= (s << 32);
    static QMap<quint64, QPixmap> s_pixMap;
    if (!s_pixMap.contains(thing))
    {
        QPixmap pix(32, s);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        QLinearGradient lg(pix.rect().topLeft(), pix.rect().bottomLeft());
        lg.setStops(Settings::gradientStops(dConf.input.gradient, opt->palette.color(QPalette::Highlight)));
//        lg.setColorAt(0.0f, Color::mid(opt->palette.color(QPalette::Highlight), Qt::white, 5, 1));
//        lg.setColorAt(1.0f, opt->palette.color(QPalette::Highlight)/*Color::mid(bc, Qt::black, 7, 1)*/);
        p.fillRect(pix.rect(), lg);
        const int penWidth(12);
        lg.setColorAt(0.0f, Color::mid(opt->palette.color(bg), opt->palette.color(QPalette::Highlight)));
        lg.setColorAt(1.0f, opt->palette.color(QPalette::Highlight)/*Color::mid(bc, Qt::black, 7, 1)*/);
        p.setPen(QPen(lg, penWidth));
        p.setRenderHint(QPainter::Antialiasing);
        QRect penRect(pix.rect().adjusted(penWidth/2, -1, -(penWidth/2), 1));
        p.drawLine(penRect.topLeft(), penRect.bottomRight());
        p.end();
        s_pixMap.insert(thing, pix);
    }
    QPixmap pixmap = s_pixMap.value(thing);
    if (!hor)
    {
        QTransform t;
        int d(s/2);
        t.translate(d, d);
        t.rotate(90);
        t.translate(-d, -d);
        pixmap = pixmap.transformed(t);
    }
    int a(0);
    if (opt->maximum != 0)
    {
        static QMap<const QWidget *, int> anim;
        if (widget && anim.contains(widget))
            a = anim.value(widget);
        ++a;
        if (a > 32)
            a = 0;
        if (widget)
            anim.insert(widget, a);
    }
    bool inv(false);
    if (optv2)
        inv = hor?optv2->invertedAppearance:!optv2->bottomToTop;
    QPoint offSet(hor?inv?a:-a:0, !hor?inv?a:-a:0);
//    offSet += Render::maskRect(dConf.progressbars.shadow, groove).topLeft();
    painter->setClipRect(cont);
    QBrush b(pixmap);
    Render::drawClickable(dConf.progressbars.shadow, groove, painter, dConf.progressbars.rnd, dConf.shadows.opacity, widget, option, &b, 0, Render::All, offSet);
    painter->restore();
    return true;
}

bool
StyleProject::drawProgressBarGroove(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ProgressBarV2, opt, option);
    if (!opt)
        return true;

    const QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget)); //The groove where the progress indicator is drawn in a QProgressBar.
    QRect cont(progressContents(opt, widget)); //The progress indicator of a QProgressBar.
    painter->setClipRegion(QRegion(groove)-QRegion(cont));
    const bool hor(opt->orientation == Qt::Horizontal);
    QLinearGradient lg(0, 0, !hor*groove.width(), hor*cont.height());
    lg.setStops(Settings::gradientStops(dConf.input.gradient, opt->palette.color(QPalette::Base)));
    QBrush b(lg);
    Render::drawClickable(dConf.progressbars.shadow, groove, painter, dConf.progressbars.rnd, dConf.shadows.opacity, widget, option, &b);
    painter->setClipping(false);
    return true;
}

bool
StyleProject::drawProgressBarLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ProgressBar, opt, option);
    castOpt(ProgressBarV2, optv2, option);
    if (!opt || opt->text.isEmpty())
        return true;

    painter->save();

    const QPalette::ColorRole fg(QPalette::Text), bg(QPalette::Base);
    QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget)); //The groove where the progress indicator is drawn in a QProgressBar.
    QRect cont(progressContents(opt, widget)); //The progress indicator of a QProgressBar.
    QRect label(subElementRect(SE_ProgressBarLabel, opt, widget)); //he text label of a QProgressBar.
    const bool hor(!optv2 || optv2->orientation == Qt::Horizontal);
    const float w_2(groove.width()/2.0f), h_2(groove.height()/2.0f);
#define TRANSLATE(_BOOL_) \
    if(!hor) \
    { \
        painter->translate(w_2, h_2); \
        painter->rotate(_BOOL_?-90.0f:90.0f); \
        painter->translate(-w_2, -h_2); \
        const QPoint center(label.center()); \
        QSize lsz(label.size()); \
        lsz.transpose(); \
        label.setSize(lsz); \
        label.moveCenter(center); \
    }
    painter->setClipRegion(QRegion(groove)-QRegion(cont));
    const bool btt(optv2 && optv2->bottomToTop);
    TRANSLATE(btt);
    drawItemText(painter, label, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, fg);
    TRANSLATE(!btt);
    painter->setClipRegion(QRegion(cont));
    TRANSLATE(btt);
    drawItemText(painter, label, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, QPalette::HighlightedText);
    TRANSLATE(!btt);
    painter->restore();
    return true;
}
