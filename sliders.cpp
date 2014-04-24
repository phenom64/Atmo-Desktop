#include <QScrollBar>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QDebug>
#include <QPainter>
#include <QSlider>
#include <QAbstractScrollArea>

#include "styleproject.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"

bool
StyleProject::drawScrollBar(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
//    const QStyleOptionSlider *opt = qstyleoption_cast<const QStyleOptionSlider *>(option);
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
StyleProject::drawSlider(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    if (!painter->isActive())
        return false;
    QPalette::ColorRole fg(QPalette::WindowText), bg(QPalette::Window);
    if (widget)
    {
        fg = widget->foregroundRole();
        bg = widget->backgroundRole();
    }
    QRect slider(subControlRect(CC_Slider, option, SC_SliderHandle, widget));
    QRect groove(subControlRect(CC_Slider, option, SC_SliderGroove, widget));
    painter->fillRect(groove, Color::mid(option->palette.color(bg), option->palette.color(fg)));
    painter->fillRect(slider, option->palette.color(fg));
    return true;
}
