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
    if (!widget || !widget->window() || !painter->isActive() || widget->palette().color(widget->backgroundRole()) != widget->window()->palette().color(QPalette::Window))
        return true;

    const QRect r(widget->rect());
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
        painter->fillRect(option->rect, QColor(0, 0, 0, dConf.shadows.opacity));
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
    QRect cont(subControlRect(CC_GroupBox, opt, SC_GroupBoxContents, widget));

    GFX::drawShadow(Sunken, cont, painter, isEnabled(opt), 8);
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
        drawItemText(painter, label, opt->textAlignment, opt->palette, opt->ENABLED, opt->text, widget?widget->foregroundRole():QPalette::WindowText);
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
    if (!widget || widget->isWindow())
        return true;

    const QStyleOptionFrameV3 *opt = qstyleoption_cast<const QStyleOptionFrameV3 *>(option);
    if (!opt)
        return true;

    if (opt->frameShape == QFrame::NoFrame /*|| opt->frameShape == QFrame::HLine || opt->frameShape == QFrame::VLine*/)
        return true;

#define SAVEPEN const QPen pen(painter->pen())
#define RESTOREPEN painter->setPen(pen)
    if (opt->frameShape == QFrame::HLine)
    {
        SAVEPEN;
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        int l, t, r, b, y(opt->rect.center().y());
        opt->rect.getRect(&l, &t, &r, &b);
        painter->drawLine(l, y, r, y);
        painter->setPen(QColor(255, 255, 255, dConf.shadows.opacity/2));
        painter->drawLine(l, y+1, r, y+1);
        RESTOREPEN;
        return true;
    }
    if (opt->frameShape == QFrame::VLine)
    {
        SAVEPEN;
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        int l, t, r, b, x(opt->rect.center().x());
        opt->rect.getRect(&l, &t, &r, &b);
        painter->drawLine(x, t, x, b);
        RESTOREPEN;
        return true;
    }
#undef SAVEPEN
#undef RESTOREPEN

    const QFrame *frame = qobject_cast<const QFrame *>(widget);
    if (Overlay::overlay(frame))
        return true;

    QRect r(option->rect);
    const bool isView(qobject_cast<const QAbstractScrollArea *>(widget));
    float o(dConf.shadows.opacity);
    if (!opt->ENABLED)
        o/=2;

    if ((frame && frame->frameShadow() == QFrame::Sunken) || (opt->state & State_Sunken))
        GFX::drawShadow(Sunken, r, painter, isEnabled(opt), !isView&&(!frame || !qobject_cast<QMainWindow *>(frame->window()))*7, All);
    else if (opt->state & State_Raised)
        GFX::drawShadow(Raised, r, painter, isEnabled(opt), 8);
    else if (frame && frame->frameShadow() == QFrame::Plain)
        GFX::drawShadow(Etched, r, painter, isEnabled(opt), 8);
    return true;
}
