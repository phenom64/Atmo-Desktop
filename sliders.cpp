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

bool
StyleProject::drawScrollBar(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Slider, opt, option);
    if (!opt)
        return true;
    QPalette::ColorRole fg(QPalette::WindowText), bg(QPalette::Window);
    if (widget)
    {
        fg = widget->foregroundRole();
        bg = widget->backgroundRole();
        if (widget->parentWidget() && widget->parentWidget()->parentWidget())
        if (castObj(const QAbstractScrollArea *, view, widget->parentWidget()->parentWidget()))
        {
            fg = view->viewport()->foregroundRole();
            bg = view->viewport()->backgroundRole();
        }
    }
    //not used atm
//    QRect up(subControlRect(CC_ScrollBar, option, SC_ScrollBarSubLine, widget));
//    QRect down(subControlRect(CC_ScrollBar, option, SC_ScrollBarAddLine, widget));
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
    QPalette::ColorRole bg(QPalette::Window);
    if (widget)
    {
        bg = widget->backgroundRole();
        if (castObj(const QAbstractScrollArea *, view, widget))
            bg = view->viewport()->backgroundRole();
    }
    painter->fillRect(option->rect, option->palette.color(bg));
    return true;
}

bool
StyleProject::drawSlider(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Slider, opt, option);
    if (!opt)
        return false;
    QPalette::ColorRole fg(QPalette::WindowText), bg(QPalette::Window);
    if (widget)
    {
        fg = widget->foregroundRole();
        bg = widget->backgroundRole();
    }
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

    QRect groove(subElementRect(SE_ProgressBarGroove, opt, widget)); //The groove where the progress indicator is drawn in a QProgressBar.
    QRect cont(subElementRect(SE_ProgressBarContents, opt, widget)); //The progress indicator of a QProgressBar.
    QRect label(subElementRect(SE_ProgressBarLabel, opt, widget)); //he text label of a QProgressBar.
    const bool hor(opt->orientation == Qt::Horizontal);
    const float w_2(groove.width()/2.0f), h_2(groove.height()/2.0f);

#define TRANSLATE(_BOOL_) if(!hor) {painter->translate(w_2, h_2);painter->rotate(_BOOL_?-90.0f:90.0f);painter->translate(-w_2, -h_2);}
    TRANSLATE(opt->bottomToTop);
    drawItemText(painter, label, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, widget?widget->foregroundRole():QPalette::WindowText);
    TRANSLATE(!opt->bottomToTop);
    Render::renderMask(cont.adjusted(1, 1, -1, -2), painter, opt->palette.color(QPalette::Highlight), 1);
    Render::renderShadow(Render::Sunken, groove, painter, 2, Render::All, 0.33f);

    painter->setClipRect(cont);
    TRANSLATE(opt->bottomToTop);
    drawItemText(painter, label, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, QPalette::HighlightedText);
    TRANSLATE(!opt->bottomToTop);
    painter->setClipping(false);
    return true;
}
