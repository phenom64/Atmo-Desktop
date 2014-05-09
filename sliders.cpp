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
    QRect slider(subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget));
    QRect groove(subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));

    castObj(const QScrollBar *, bar, widget);
    if (widget && (hitTestComplexControl(CC_ScrollBar, opt, widget->mapFromGlobal(QCursor::pos()), widget) == SC_ScrollBarSlider
                   || (bar && bar->isSliderDown())))
        fg = QPalette::Highlight;
    QColor bgc(opt->palette.color(bg)), fgc(opt->palette.color(fg));
    Color::ensureContrast(bgc, fgc);
    painter->fillRect(groove, bgc);
//    const bool hor(opt->orientation==Qt::Horizontal);
//    const int m(1); //margin
//    slider.adjust(!hor*m, hor*m, -!hor*m, hor*m);
    const int o(painter->opacity());
    if (!(widget && widget->underMouse()))
        painter->setOpacity(0.5f);
    Render::renderMask(slider, painter, fgc);
    painter->setOpacity(o);
    return true;
}

bool
StyleProject::drawScrollAreaCorner(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Window));
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
    int d(3); //shadow size for slider and groove 'extent'
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

    Render::renderShadow(Render::Sunken, groove.adjusted(-1, -1, 1, 2), painter, 32, Render::All, 0.5f);
    Render::renderShadow(Render::Raised, slider, painter);

    --d;
    slider.adjust(d, d, -d, -d);
    QLinearGradient lg(0, d, 0, slider.height()); //buahahaha just noticed a bug in rendermask.
    lg.setColorAt(0.0f, Color::mid(option->palette.color(bg), Qt::white, 2, 1));
    lg.setColorAt(1.0f, Color::mid(option->palette.color(bg), Qt::black, 2, 1));
    Render::renderMask(slider, painter, lg);
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
    return true;
}

bool
StyleProject::drawProgressBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ProgressBarV2, opt, option);
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
    castOpt(ProgressBarV2, opt, option);
    if (!opt)
        return true;

    QRect cont(subElementRect(SE_ProgressBarContents, opt, widget)); //The progress indicator of a QProgressBar.
    const QColor h(opt->palette.color(QPalette::Highlight));
    const bool hor(opt->orientation == Qt::Horizontal);
    const int s(hor?cont.height():cont.width());

    quint64 thing = h.rgba();
    thing |= ((quint64)s << 32);
    static QMap<quint64, QPixmap > pixMap;
    if (!pixMap.contains(thing))
    {
        QPixmap pix(s/2, s);
        pix.fill(Qt::transparent);
        QLinearGradient lg(pix.rect().topLeft(), pix.rect().bottomLeft());
        QColor top(Color::mid(h, Qt::white, 5, 1));
        Color::shiftHue(top, -8);
        QColor bottom(Color::mid(h, Qt::black, 5, 1));
        Color::shiftHue(bottom, 8);
        QPainter p(&pix);
        lg.setColorAt(0.0f, top);
        lg.setColorAt(1.0f, bottom);
        p.fillRect(pix.rect(), lg);
        QRadialGradient rg(pix.rect().center()+QPoint(0, pix.rect().height()*0.25f), pix.rect().height());
        QColor t(Color::mid(top, Qt::white));
        Color::shiftHue(t, -32);
        t.setAlpha(192);
        rg.setColorAt(0.0f, t);
        rg.setColorAt(1.0f, Qt::transparent);
        p.fillRect(pix.rect(), rg);
        QLinearGradient lg2(pix.rect().topLeft(), pix.rect().bottomLeft());
        lg2.setColorAt(0.0f, QColor(0, 0, 0, 64));
        lg2.setColorAt(0.15f, QColor(255, 255, 255, 127));
        lg2.setColorAt(0.4f, QColor(255, 255, 255, 64));
        lg2.setColorAt(0.5f, Qt::transparent);
        p.fillRect(pix.rect(), lg2);
        p.end();

        QPixmap pixs(s, s);
        pixs.fill(Qt::transparent);
        p.begin(&pixs);
        p.fillRect(pixs.rect(), pix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QPixmap::fromImage(pix.toImage().mirrored()), s/3));
        QRect r(pixs.rect().adjusted(s/6, 0, -s/6, 0));
        p.drawLine(r.topLeft(), r.bottomRight());
        p.end();
        pixMap.insert(thing, pixs);
    }
    QPixmap pixmap = pixMap.value(thing);
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
        if (a > pixmap.width())
            a = 0;
        if (widget)
            anim.insert(widget, a);
    }
    painter->save();
    const bool inv(hor?opt->invertedAppearance:!opt->bottomToTop);
    painter->setBrushOrigin(hor?inv?a:-a:0, !hor?inv?a:-a:0);
    painter->fillRect(cont.adjusted(1, 1, -1, -2), pixmap);
    painter->restore();
    return true;
}

bool
StyleProject::drawProgressBarGroove(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ProgressBarV2, opt, option);
    if (!opt)
        return true;

    QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget)); //The groove where the progress indicator is drawn in a QProgressBar.
    painter->fillRect(groove.adjusted(1, 1, -1, -2), opt->palette.color(Ops::bgRole(widget, QPalette::Base)));
    Render::renderShadow(Render::Sunken, groove, painter, 2, Render::All, 0.33f);
    return true;
}

bool
StyleProject::drawProgressBarLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ProgressBarV2, opt, option);
    if (!opt)
        return true;

    const QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::Text)), bg(Ops::bgRole(widget, QPalette::Base));
    QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget)); //The groove where the progress indicator is drawn in a QProgressBar.
    QRect cont(subElementRect(SE_ProgressBarContents, opt, widget)); //The progress indicator of a QProgressBar.
    QRect label(subElementRect(SE_ProgressBarLabel, opt, widget)); //he text label of a QProgressBar.
    const bool hor(opt->orientation == Qt::Horizontal);
    const float w_2(groove.width()/2.0f), h_2(groove.height()/2.0f);
#define TRANSLATE(_BOOL_) if(!hor) {painter->translate(w_2, h_2);painter->rotate(_BOOL_?-90.0f:90.0f);painter->translate(-w_2, -h_2);}
    painter->setClipRegion(QRegion(groove)-QRegion(cont));
    TRANSLATE(opt->bottomToTop);
    drawItemText(painter, label, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, fg);
    TRANSLATE(!opt->bottomToTop);

    painter->setClipRegion(QRegion(cont));
    TRANSLATE(opt->bottomToTop);
    drawItemText(painter, label, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, QPalette::HighlightedText);
    TRANSLATE(!opt->bottomToTop);
    painter->setClipping(false);
    return true;
}
