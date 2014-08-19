#include <QScrollBar>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QDebug>
#include <QPainter>
#include <QSlider>
#include <QAbstractScrollArea>
#include <QProgressBar>
#include <QStyleOptionProgressBar>
#include <QStyleOptionProgressBarV2>

#include "styleproject.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/progresshandler.h"
#include "stylelib/animhandler.h"

bool
StyleProject::drawScrollBar(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Slider, opt, option);
    if (!opt)
        return true;
    const QWidget *w = Ops::getAncestor<const QAbstractScrollArea *>(widget);
    QPalette::ColorRole fg(Ops::fgRole(w, QPalette::WindowText)), bg(Ops::bgRole(w, QPalette::Window));
    //not used atm
    //QRect up(subControlRect(CC_ScrollBar, option, SC_ScrollBarSubLine, widget));
    //QRect down(subControlRect(CC_ScrollBar, option, SC_ScrollBarAddLine, widget));
    const int m(2);
    QRect slider(subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget).adjusted(m, m, -m, -m));
    QRect groove(subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));

    castObj(const QScrollBar *, bar, widget);
//    if (widget && (hitTestComplexControl(CC_ScrollBar, opt, widget->mapFromGlobal(QCursor::pos()), widget) == SC_ScrollBarSlider
//                   || (bar && bar->isSliderDown())))
//        fg = QPalette::Highlight;
    QColor bgc(opt->palette.color(bg)), fgc(opt->palette.color(fg));
    Color::ensureContrast(bgc, fgc);
    painter->fillRect(groove, bgc);
    if (bg == QPalette::Base)
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
        painter->setPen(QColor(0, 0, 0, 32));
        painter->drawLine(l);
        painter->setPen(saved);
    }
//    const bool hor(opt->orientation==Qt::Horizontal);
//    const int m(1); //margin
//    slider.adjust(!hor*m, hor*m, -!hor*m, hor*m);
    const int o(painter->opacity());
    const int level(Anim::Basic::level(widget));
    const float add(0.5f/(float)STEPS);
    if (bar && !bar->isSliderDown())
        painter->setOpacity(0.5f+add*level);
    Render::renderMask(slider, painter, fgc);
    painter->setOpacity(o);
    return true;
}

bool
StyleProject::drawScrollAreaCorner(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Window));
    painter->fillRect(option->rect, option->palette.color(bg));
    return true;
}

bool
StyleProject::drawSlider(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Slider, opt, option);
    if (!opt)
        return false;
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::WindowText)), bg(Ops::bgRole(widget, QPalette::Window));
    QRect slider(subControlRect(CC_Slider, option, SC_SliderHandle, widget));
    QRect groove(subControlRect(CC_Slider, option, SC_SliderGroove, widget));


    const bool hor(opt->orientation==Qt::Horizontal);
    int d(2); //shadow size for slider and groove 'extent'
//    const int m(qMin(groove.height(), groove.width())/3); //margin
    const QPoint c(groove.center());
    if (hor)
        groove.setHeight(d);
    else
        groove.setWidth(d);
    groove.moveCenter(c);
    ++d; //need to account the one px frame...
    groove.adjust(hor*d, !hor*d, -(hor*d), -(!hor*d)); //set the proper inset for the groove in order to account for the slider shadow
    --d;

    Render::renderMask(groove, painter, Color::mid(opt->palette.color(bg), opt->palette.color(fg)));
//    Render::renderShadow(Render::Sunken, groove.adjusted(-1, -1, 1, 2), painter, 32, Render::All, 0.5f);
//    Render::renderShadow(Render::Raised, slider, painter);

    --d;
    slider.adjust(d, d, -d, -d);

    QColor bgc(opt->palette.color(bg));
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED)
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }

    QLinearGradient lg(0, d, 0, slider.height()); //buahahaha just noticed a bug in rendermask.

    lg.setColorAt(0.0f, Color::mid(bgc, Qt::white, 5, 1));
    lg.setColorAt(1.0f, bgc);

    Render::renderMask(slider.adjusted(1, 1, -1, -1), painter, lg);

//    QLinearGradient shadow(0, 0, 0, option->rect.height());
//    shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
//    shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
//    shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
    QBrush b(QColor(0, 0, 0, 48));

    Render::renderShadow(Render::Simple, slider, painter, 32, Render::All, 1.0f, &b);
#if 0
    int ds(slider.height()/2);
    ds &= ~1;
    QRect dot(0, 0, ds, ds);
    dot.moveCenter(slider.center());
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(option->palette.color(fg));
    painter->drawEllipse(dot);
    painter->restore();
#endif
    return true;
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

//    qDebug() << opt->progress << opt->minimum << opt->maximum;
    QRect cont(subElementRect(SE_ProgressBarContents, opt, widget)); //The progress indicator of a QProgressBar.
    QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget));
    const QColor h(opt->palette.color(QPalette::Highlight));
    castOpt(ProgressBarV2, optv2, option);
    const bool hor(!optv2 || optv2->orientation == Qt::Horizontal);
    const int s(hor?cont.height():cont.width());

    const QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::Text)), bg(Ops::bgRole(widget, QPalette::Base));

    quint64 thing = h.rgba();
    thing |= ((quint64)s << 32);
    static QMap<quint64, QPixmap> s_pixMap;
    if (!s_pixMap.contains(thing))
    {
        QPixmap pix(32, s);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        QLinearGradient lg(pix.rect().topLeft(), pix.rect().bottomLeft());
        lg.setColorAt(0.0f, Color::mid(opt->palette.color(QPalette::Highlight), Qt::white, 5, 1));
        lg.setColorAt(1.0f, opt->palette.color(QPalette::Highlight)/*Color::mid(bc, Qt::black, 7, 1)*/);
        p.fillRect(pix.rect(), lg);
        const int penWidth(12);
        lg.setColorAt(0.0f, Color::mid(opt->palette.color(bg), Qt::white, 5, 1));
        lg.setColorAt(1.0f, opt->palette.color(bg)/*Color::mid(bc, Qt::black, 7, 1)*/);
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
    painter->setClipRect(cont);
    Render::renderMask(groove, painter, pixmap, 4, Render::All, offSet);
    QLinearGradient shadow(0, 0, 0, opt->rect.height());
    shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
    shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
    shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
    QBrush b(shadow);

    Render::renderShadow(Render::Simple, groove, painter, 4, Render::All, 1.0f, &b);
    painter->setClipping(false);
    return true;
}

bool
StyleProject::drawProgressBarGroove(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ProgressBar, opt, option);
    if (!opt)
        return true;

    QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget)); //The groove where the progress indicator is drawn in a QProgressBar.
    Render::renderMask(groove, painter, opt->palette.color(Ops::bgRole(widget, QPalette::Base)), 4);

    QLinearGradient shadow(0, 0, 0, opt->rect.height());
    shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
    shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
    shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
    QBrush b(shadow);

    Render::renderShadow(Render::Simple, groove, painter, 4, Render::All, 1.0f, &b);
    return true;
}

bool
StyleProject::drawProgressBarLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ProgressBar, opt, option);
    castOpt(ProgressBarV2, optv2, option);
    if (!opt || opt->text.isEmpty())
        return true;

    const QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::Text)), bg(Ops::bgRole(widget, QPalette::Base));
    QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget)); //The groove where the progress indicator is drawn in a QProgressBar.
    QRect cont(subElementRect(SE_ProgressBarContents, opt, widget)); //The progress indicator of a QProgressBar.
    QRect label(subElementRect(SE_ProgressBarLabel, opt, widget)); //he text label of a QProgressBar.
    const bool hor(!optv2 || optv2->orientation == Qt::Horizontal);
    const float w_2(groove.width()/2.0f), h_2(groove.height()/2.0f);
#define TRANSLATE(_BOOL_) if(!hor) {painter->translate(w_2, h_2);painter->rotate(_BOOL_?-90.0f:90.0f);painter->translate(-w_2, -h_2);}
    painter->setClipRegion(QRegion(groove)-QRegion(cont));
    const bool btt(optv2 && optv2->bottomToTop);
    TRANSLATE(btt);
    drawItemText(painter, label, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, fg);
    TRANSLATE(!btt);

    painter->setClipRegion(QRegion(cont));
    TRANSLATE(btt);
    QRect tr(opt->fontMetrics.boundingRect(label, Qt::AlignCenter, opt->text));
    int rnd(qMin(tr.height(), tr.width())/2);
    painter->save();
    painter->setBrush(opt->palette.color(QPalette::Highlight));
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRoundedRect(tr, rnd, rnd);
    painter->restore();
    drawItemText(painter, label, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, QPalette::HighlightedText);
    TRANSLATE(!btt);
    painter->setClipping(false);
    return true;
}
