#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QToolBar>
#include <QMainWindow>
#include <QStyleOptionGroupBox>
#include <QStyleOptionDockWidget>
#include <QStyleOptionDockWidgetV2>
#include <QCheckBox>
#include <QMenuBar>
#include <QMap>
#include <QToolButton>
#include <QDockWidget>
#include <QAbstractScrollArea>

#include "dsp.h"
#include "stylelib/gfx.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/handlers.h"
#include "overlay.h"
#include "config/settings.h"
#include "stylelib/xhandler.h"

using namespace DSP;

bool
Style::drawStatusBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
//    Q_UNUSED(option);
    if (!widget || widget->palette().color(widget->backgroundRole()) != widget->window()->palette().color(QPalette::Window))
        return true;

    if (dConf.uno.enabled)
    {
        Sides sides = All & ~Top;
        QPoint topLeft = widget->mapTo(widget->window(), widget->rect().topLeft());
        QRect winRect = widget->window()->rect();
        QRect widgetRect = QRect(topLeft, widget->size());

        if (widgetRect.bottom() >= winRect.bottom())
            sides &= ~Bottom;
        if (widgetRect.left() <= winRect.left())
            sides &= ~Left;
        if (widgetRect.right() >= winRect.right())
            sides &= ~Right;

        if (!sides)
        {
            Handlers::Window::drawUnoPart(painter, option->rect/*.sAdjusted(1, 1, -1, -1)*/, widget);
            GFX::drawShadow(Rect, option->rect, painter, isEnabled(option), 0, Top);
        }
    }
    return true;
}

bool
Style::drawSplitter(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (dConf.uno.enabled)
    if (widget && !qobject_cast<const QToolBar *>(widget->parentWidget()))
    if (option->rect.width() == 1 || option->rect.height() == 1)
    {
        painter->fillRect(option->rect, QColor(0, 0, 0, dConf.shadows.opacity));
        return true;
    }
    if (option->rect.height() < option->rect.width())
    {
        const int sz = option->rect.height();
        QRect r(0, 0, sz, sz);
        r.moveCenter(option->rect.center());
        GFX::drawHandle(painter, r, true);
    }
    else
    {
        const int sz = option->rect.width();
        QRect r(0, 0, sz, sz);
        r.moveCenter(option->rect.center());
        GFX::drawHandle(painter, r, false);
    }

    return true;
}

bool
Style::drawColumnViewGrip(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    GFX::drawHandle(painter, option->rect.shrinked(3), false);
    return true;
}

bool
Style::drawToolBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (dConf.uno.enabled
            && widget
            && option
            && qobject_cast<const QMainWindow *>(widget->parentWidget()))
            Handlers::Window::drawUnoPart(painter, option->rect, widget, widget->geometry().topLeft());
    return true;
}

bool
Style::drawMenuBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (dConf.uno.enabled
            && widget
            && qobject_cast<const QMainWindow *>(widget->parentWidget()))
        Handlers::Window::drawUnoPart(painter, option->rect, widget, widget->geometry().topLeft());
    return true;
}

bool
Style::drawWindow(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && widget->isWindow())
        painter->fillRect(option->rect, option->palette.color(QPalette::Window));
    return true;
}

bool
Style::drawMenu(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Base));
    QColor bgc(option->palette.color(bg));
    if (widget && !widget->property("DSP_hasmenuarrow").toBool())
        bgc.setAlpha(XHandler::opacity()*255.0f);

//    painter->save();
//    painter->setRenderHint(QPainter::Antialiasing);
//    painter->setPen(Qt::NoPen);
//    painter->setBrush(bgc);
    Sides sides(All);
    if (widget && widget->property("DSP_SHAPETOP").toBool())
        sides &= ~Top;

    QLinearGradient lg(option->rect.topLeft(), option->rect.bottomLeft());
    lg.setStops(Settings::gradientStops(dConf.menues.gradient, bgc));

    GFX::drawMask(option->rect, painter, lg, 4, sides);
//    painter->drawRoundedRect(option->rect, 4, 4);
//    painter->restore();
    return true;
}

bool
Style::drawGroupBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionGroupBox *opt = qstyleoption_cast<const QStyleOptionGroupBox *>(option);
    if (!opt)
        return true;

//    QRect frame(subControlRect(CC_GroupBox, opt, SC_GroupBoxFrame, widget)); //no need?
    QRect label(subControlRect(CC_GroupBox, opt, SC_GroupBoxLabel, widget));
    QRect check(subControlRect(CC_GroupBox, opt, SC_GroupBoxCheckBox, widget));
    QRect cont(subControlRect(CC_GroupBox, opt, SC_GroupBoxFrame, widget));

//    GFX::drawShadow(Sunken, cont, painter, isEnabled(opt), 6);
    GFX::drawClickable(Sunken, cont, painter, QColor(0, 0, 0, 15), dConf.frameRnd);
    if (opt->subControls & SC_GroupBoxCheckBox)
    {
        QStyleOptionButton btn;
        btn.QStyleOption::operator =(*option);
        btn.rect = check;
        drawCheckBox(&btn, painter, widget);
    }
    if (!opt->text.isEmpty())
    {
        painter->save();
        QFont f(painter->font());
        f.setBold(true);
        painter->setFont(f);
        drawItemText(painter, label, opt->textAlignment|Qt::AlignVCenter, opt->palette, isEnabled(opt), opt->text, widget?widget->foregroundRole():QPalette::WindowText);
        painter->restore();
    }
    return true;
}

bool
Style::drawToolTip(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QLinearGradient lg(option->rect.topLeft(), option->rect.bottomLeft());
    lg.setColorAt(0.0f, Color::mid(option->palette.color(QPalette::ToolTipBase), Qt::white, 5, 1));
    lg.setColorAt(1.0f, Color::mid(option->palette.color(QPalette::ToolTipBase), Qt::black, 5, 1));
    const QBrush brush(painter->brush());
    const QPen pen(painter->pen());
    const bool hadAa(painter->testRenderHint(QPainter::Antialiasing));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(lg);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(option->rect, 4, 4);
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->setRenderHint(QPainter::Antialiasing, hadAa);
    return true;
}

bool
Style::drawDockTitle(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionDockWidget *opt = qstyleoption_cast<const QStyleOptionDockWidget *>(option);
    if (!opt)
        return true;

    const QRect tr(subElementRect(SE_DockWidgetTitleBarText, opt, widget));
//    const QRect cr(subElementRect(SE_DockWidgetCloseButton, opt, widget));
//    const QRect fr(subElementRect(SE_DockWidgetFloatButton, opt, widget));
//    const QRect ir(subElementRect(SE_DockWidgetIcon, opt, widget));
#if 0
    if (dConf.uno.enabled)
    {
        const QDockWidget *dock = qobject_cast<const QDockWidget *>(widget);
        const QPen savedPen(painter->pen());
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        QRect l(tr);
        l.setLeft(0);
        l.setBottom(l.bottom()+1);
        painter->drawLine(l.bottomLeft(), l.bottomRight());
        if (dock && !dock->isFloating())
        {
            const bool left(dock->geometry().x() < dock->window()->width() - dock->geometry().right());
            if (left && dock->geometry().x() > 0)
                painter->drawLine(dock->rect().topLeft(), dock->rect().bottomLeft());
            if (!left && dock->geometry().right()+1 < dock->window()->rect().right())
                painter->drawLine(dock->rect().topRight(), dock->rect().bottomRight());
        }
        painter->setPen(savedPen);
    }
#endif

    const QFont f(painter->font());
    QFont bold(f);
    bold.setBold(true);
    const QString title(QFontMetrics(bold).elidedText(opt->title, Qt::ElideRight, tr.width(), Qt::TextShowMnemonic));
    painter->setFont(bold);
    drawItemText(painter, tr, Qt::AlignCenter, opt->palette, opt->ENABLED, title, widget?widget->foregroundRole():QPalette::WindowText);
    painter->setFont(f);
    return true;
}

bool
Style::drawFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!widget || widget->isWindow() || Overlay::overlay(widget))
        return true;

    const QStyleOptionFrameV3 *opt = qstyleoption_cast<const QStyleOptionFrameV3 *>(option);
    if (!opt)
        return true;

    if (opt->frameShape == QFrame::NoFrame /*|| opt->frameShape == QFrame::HLine || opt->frameShape == QFrame::VLine*/)
        return true;

    if (opt->frameShape == QFrame::HLine)
    {
        const QPen pen(painter->pen());
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        int l, t, r, b, y(opt->rect.center().y());
        opt->rect.getRect(&l, &t, &r, &b);
        painter->drawLine(l, y, r, y);
        painter->setPen(QColor(255, 255, 255, dConf.shadows.illumination));
        painter->drawLine(l, y+1, r, y+1);
        painter->setPen(pen);
        return true;
    }
    if (opt->frameShape == QFrame::VLine)
    {
        const QPen pen(painter->pen());
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        int l, t, r, b, x(opt->rect.center().x());
        opt->rect.getRect(&l, &t, &r, &b);
        painter->drawLine(x, t, x, b);
        painter->setPen(pen);
        return true;
    }

    const QFrame *frame = qobject_cast<const QFrame *>(widget);
    QRect r(option->rect);
    const bool isView(qobject_cast<const QAbstractScrollArea *>(widget));

//    if (true && isView)
//    {
//        const QAbstractScrollArea *area = static_cast<const QAbstractScrollArea *>(widget);
//        Mask::render(r, area->viewport()->palette().color(area->viewport()->backgroundRole()), painter, 2.0f);
//        return true;
//    }

    if ((frame && frame->frameShadow() == QFrame::Sunken) || (opt->state & State_Sunken))
    {


        if (isView && dConf.views.traditional)
        {
            QColor base = frame->palette().color(frame->backgroundRole());
            const QAbstractScrollArea *area = static_cast<const QAbstractScrollArea *>(widget);
            base = area->viewport()->palette().color(area->viewport()->backgroundRole());
            GFX::drawClickable(dConf.views.viewShadow, r, painter, base, dConf.views.viewRnd);
        }
        else
        {
//        static const ShadowStyle style = Raised;
//        static const int sm = FrameWidth - (GFX::shadowMargin(style)/*+1*/);
//        r.adjust(sm, sm, -sm, -sm);
            int rnd(dConf.frameRnd);
            if (isView || (widget && widget->inherits("KTextEditor::ViewPrivate")))
                rnd = 0;
            GFX::drawShadow(Sunken, r, painter, isEnabled(opt), rnd);
        }
    }
    else if (opt->state & State_Raised)
        GFX::drawShadow(Raised, r, painter, isEnabled(opt), dConf.frameRnd);
    else if (frame && frame->frameShadow() == QFrame::Plain)
        GFX::drawShadow(Etched, r, painter, isEnabled(opt), dConf.frameRnd);
    return true;
}
