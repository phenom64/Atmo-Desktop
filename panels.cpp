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

#include "styleproject.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/unohandler.h"
#include "overlay.h"
#include "stylelib/settings.h"
#include "stylelib/xhandler.h"

bool
StyleProject::drawStatusBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!widget || !widget->window() || !painter->isActive() || widget->palette().color(widget->backgroundRole()) != widget->window()->palette().color(QPalette::Window))
        return true;

    Render::Sides sides = Render::All;
    QPoint topLeft = widget->mapTo(widget->window(), widget->rect().topLeft());
    QRect winRect = widget->window()->rect();
    QRect widgetRect = QRect(topLeft, widget->size());

    if (widgetRect.left() <= winRect.left())
        sides &= ~Render::Left;
    if (widgetRect.right() >= winRect.right())
        sides &= ~Render::Right;
    if (widgetRect.bottom() >= winRect.bottom())
        sides &= ~Render::Bottom;

    if (sides & (Render::Left|Render::Right))
    {
        painter->fillRect(widget->rect(), widget->palette().color(widget->backgroundRole()));
        return true;
    }
    const QRect r(widget->rect());
    if (sides & Render::Bottom)
    {
        UNO::Handler::drawUnoPart(painter, r/*.sAdjusted(1, 1, -1, -1)*/, widget, QPoint(), XHandler::opacity());;
    }
    else
    {
        QPixmap pix(r.size());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        UNO::Handler::drawUnoPart(&p, r/*.sAdjusted(1, 1, -1, -1)*/, widget, QPoint(), XHandler::opacity());
        p.end();
        Render::renderMask(r, painter, pix, 4, Render::Bottom|Render::Left|Render::Right);
    }

    painter->save();
    painter->setPen(Qt::black);
    painter->setOpacity(Settings::conf.shadows.opacity);
    painter->drawLine(r.topLeft(), r.topRight());
    if (sides & Render::Bottom)
        painter->drawLine(r.bottomLeft(), r.bottomRight());
    painter->restore();
    return true;
}

bool
StyleProject::drawSplitter(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && !qobject_cast<const QToolBar *>(widget->parentWidget()))
    if (option->rect.width() == 1 || option->rect.height() == 1)
        painter->fillRect(option->rect, QColor(0, 0, 0, Settings::conf.shadows.opacity*255.0f));
    return true;
}

bool
StyleProject::drawToolBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && option)
    if (castObj(const QMainWindow *, win, widget->parentWidget()))
    {
        painter->save();
        if (Settings::conf.removeTitleBars && XHandler::opacity() < 1.0f && win->windowFlags() & Qt::FramelessWindowHint)
        {
            Render::Sides sides(Render::checkedForWindowEdges(widget));
            sides = Render::All-sides;
            QPixmap p(option->rect.size());
            p.fill(Qt::transparent);
            QPainter pt(&p);
            UNO::Handler::drawUnoPart(&pt, option->rect, widget, widget->geometry().topLeft(), XHandler::opacity());
            pt.end();
            Render::renderMask(option->rect, painter, p, 4, sides);
        }
        else
            UNO::Handler::drawUnoPart(painter, option->rect, widget, widget->geometry().topLeft(), XHandler::opacity());
//        painter->drawTiledPixmap(option->rect, bg, widget->geometry().topLeft());
//        int hh(UNO::unoHeight(win, UNO::ToolBars));
//        if (hh == widget->geometry().bottom()+1)
//        {
//            painter->setPen(Qt::black);
//            painter->setOpacity(Settings::conf.shadows.opacity);
//            painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
//        }
        painter->restore();
    }
    return true;
}

bool
StyleProject::drawMenuBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget)
    if (castObj(const QMainWindow *, win, widget->parentWidget()))
        UNO::Handler::drawUnoPart(painter, option->rect, widget, widget->geometry().topLeft(), XHandler::opacity());
    return true;
}

bool
StyleProject::drawWindow(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return true;
}

bool
StyleProject::drawMenu(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Base));
    Render::Sides sides = Render::All;
    QColor bgc(option->palette.color(bg));
    if (widget && !widget->property("DSP_hasmenuarrow").toBool())
        bgc.setAlpha(XHandler::opacity()*255.0f);
//    Render::renderMask(option->rect, painter, bgc, 4, sides);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(bgc);
    painter->drawRoundedRect(option->rect, 4, 4);
#if 0
    if (Settings::conf.menues.icons)
    {
        const QRect ir(option->rect.adjusted(0, 0, -(option->rect.width()-28), 0));
        painter->setClipRect(ir);
        painter->setBrush(QColor(0, 0, 0, 32));
        painter->drawRoundedRect(option->rect, 4, 4);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QColor(0, 0, 0, 255.0f*Settings::conf.shadows.opacity));
        painter->translate(0.5f, 0);
        painter->drawLine(ir.topRight(), ir.bottomRight());
    }
#endif
    painter->restore();
    return true;
}

bool
StyleProject::drawGroupBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(GroupBox, opt, option);
    if (!opt)
        return true;

//    QRect frame(subControlRect(CC_GroupBox, opt, SC_GroupBoxFrame, widget)); //no need?
    QRect label(subControlRect(CC_GroupBox, opt, SC_GroupBoxLabel, widget));
    QRect check(subControlRect(CC_GroupBox, opt, SC_GroupBoxCheckBox, widget));
    QRect cont(subControlRect(CC_GroupBox, opt, SC_GroupBoxContents, widget));

    Render::renderShadow(Render::Sunken, cont, painter, 8, Render::All, Settings::conf.shadows.opacity*0.5f);
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
StyleProject::drawToolTip(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QLinearGradient lg(option->rect.topLeft(), option->rect.bottomLeft());
    lg.setColorAt(0.0f, Color::mid(option->palette.color(QPalette::ToolTipBase), Qt::white, 5, 1));
    lg.setColorAt(1.0f, Color::mid(option->palette.color(QPalette::ToolTipBase), Qt::black, 5, 1));
    Render::renderMask(option->rect, painter, lg, 4);
    return true;
}

bool
StyleProject::drawDockTitle(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(DockWidget, opt, option);
    if (!opt)
        return true;

    const QRect tr(subElementRect(SE_DockWidgetTitleBarText, opt, widget));
//    const QRect cr(subElementRect(SE_DockWidgetCloseButton, opt, widget));
//    const QRect fr(subElementRect(SE_DockWidgetFloatButton, opt, widget));
//    const QRect ir(subElementRect(SE_DockWidgetIcon, opt, widget));
    castObj(const QDockWidget *, dock, widget);

    painter->save();
    painter->setOpacity(Settings::conf.shadows.opacity);
    painter->setPen(opt->palette.color(Ops::fgRole(widget, QPalette::WindowText)));
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
    painter->restore();

    const QFont f(painter->font());
    QFont bold(f);
    bold.setBold(true);
    const QString title(QFontMetrics(bold).elidedText(opt->title, Qt::ElideRight, tr.width()));
    painter->setFont(bold);
    drawItemText(painter, tr, Qt::AlignCenter, opt->palette, opt->ENABLED, title, widget?widget->foregroundRole():QPalette::WindowText);
    painter->setFont(f);
    return true;
}

bool
StyleProject::drawFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!widget)
        return true;

    castOpt(FrameV3, opt, option);
    if (!opt)
        return true;

    if (opt->frameShape == QFrame::NoFrame /*|| opt->frameShape == QFrame::HLine || opt->frameShape == QFrame::VLine*/)
        return true;

#define SAVEPEN const QPen pen(painter->pen())
#define RESTOREPEN painter->setPen(pen)
    if (opt->frameShape == QFrame::HLine)
    {
        SAVEPEN;
        painter->setPen(QColor(0, 0, 0, Settings::conf.shadows.opacity*255.0f));
        int l, t, r, b, y(opt->rect.center().y());
        opt->rect.getRect(&l, &t, &r, &b);
        painter->drawLine(l, y, r, y);
        painter->setPen(QColor(255, 255, 255, (Settings::conf.shadows.opacity*255.0f)/2));
        painter->drawLine(l, y+1, r, y+1);
        RESTOREPEN;
        return true;
    }
    if (opt->frameShape == QFrame::VLine)
    {
        SAVEPEN;
        painter->setPen(QColor(0, 0, 0, Settings::conf.shadows.opacity*255.0f));
        int l, t, r, b, x(opt->rect.center().x());
        opt->rect.getRect(&l, &t, &r, &b);
        painter->drawLine(x, t, x, b);
//        painter->setPen(QColor(255, 255, 255, (Settings::conf.shadows.opacity*255.0f)/2));
//        painter->drawLine(l, y+1, r, y+1);
        RESTOREPEN;
        return true;
    }
#undef SAVEPEN
#undef RESTOREPEN

    castObj(const QFrame *, frame, widget);
    if (OverLay::hasOverLay(frame))
        return true;

    int roundNess(2);
    QRect r(option->rect);

    if ((frame && frame->frameShadow() == QFrame::Sunken) || (opt->state & State_Sunken))
        Render::renderShadow(Render::Sunken, r.adjusted(1, 1, -1, 0), painter, roundNess, Render::All, Settings::conf.shadows.opacity*0.5f);

    if (opt->state & State_Raised)
    {
        QPixmap pix(frame->rect().size());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        Render::renderShadow(Render::Raised, pix.rect(), &p, 8, Render::All, Settings::conf.shadows.opacity*0.5f);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Render::renderMask(pix.rect().adjusted(2, 2, -2, -2), &p, Qt::black, 6);
        p.end();
        painter->drawTiledPixmap(frame->rect(), pix);
    }
    if (frame && frame->frameShadow() == QFrame::Plain)
        Render::renderShadow(Render::Etched, r, painter, 6, Render::All, Settings::conf.shadows.opacity*0.5f);
    return true;
}
