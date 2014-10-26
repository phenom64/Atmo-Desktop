
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QStyleOptionToolButton>
#include <QMainWindow>
#include <QDebug>
#include <QBrush>
#include <QDockWidget>
#include <QMenu>

#include "styleproject.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/animhandler.h"
#include "stylelib/settings.h"

/*
 * This here paints the button, in order to override
 * QCommonStyle painting, simply return true here
 * and paint what you want yourself.
 */

bool
StyleProject::drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Button, opt, option);
    if (!opt)
        return true;

    drawPushButtonBevel(option, painter, widget);
    drawPushButtonLabel(option, painter, widget);
    return true;
}

/* not sure if these 2 are needed at all */

bool
StyleProject::drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Button, opt, option);
    if (!opt)
        return true;
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::ButtonText)), fg(Ops::fgRole(widget, QPalette::ButtonText));
    if (!(opt->features & QStyleOptionButton::Flat))
    {
        QColor bc(option->palette.color(bg));
        if (Settings::conf.pushbtn.tint.second > -1)
            bc = Color::mid(bc, Settings::conf.pushbtn.tint.first, 100-Settings::conf.pushbtn.tint.second, Settings::conf.pushbtn.tint.second);
        QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);

        if (option->SUNKEN || opt->features & QStyleOptionButton::DefaultButton)
            bc = sc;
        else if (option->ENABLED)
        {
            int hl(Anim::Basic::level(widget));
            bc = Color::mid(bc, sc, STEPS-hl, hl);
        }
        QLinearGradient lg(0, 0, 0, Render::maskHeight(Settings::conf.pushbtn.shadow, opt->rect.height()));
        lg.setStops(Settings::gradientStops(Settings::conf.pushbtn.gradient, bc));
        QBrush m(lg);
        Render::drawClickable(Settings::conf.pushbtn.shadow, opt->rect, painter, Render::All, Settings::conf.pushbtn.rnd, Settings::conf.shadows.opacity, widget, &m);
    }
    return true;
}

bool
StyleProject::drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Button, opt, option);
    if (!opt)
        return true;
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::ButtonText));
    if (opt->features & QStyleOptionButton::Flat)
        fg = (widget&&widget->parentWidget()?widget->parentWidget()->foregroundRole():QPalette::WindowText);
    QRect r(opt->rect);
    if (!opt->text.isEmpty())
        drawItemText(painter, r, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, fg);
    else
        drawItemPixmap(painter, r, Qt::AlignCenter, opt->icon.pixmap(opt->iconSize, opt->ENABLED?QIcon::Normal:QIcon::Disabled));
    return true;
}


/* checkboxes */

bool
StyleProject::drawCheckBox(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Button, opt, option);
    if (!opt)
        return true;

    QPalette::ColorRole bg(QPalette::Button), fg(QPalette::ButtonText);
    QRect checkRect(subElementRect(SE_CheckBoxIndicator, opt, widget));
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
        checkRect.setBottom(qMin(checkRect.bottom(), widget->rect().bottom()));
        checkRect.setWidth(checkRect.height());
    }

    QRect textRect(subElementRect(SE_CheckBoxContents, opt, widget));
    int hor(opt->direction==Qt::LeftToRight?Qt::AlignLeft:Qt::AlignRight);
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->text, fg);

    if (opt->state & (State_On|State_NoChange))
    {
        bg = QPalette::Highlight;
        fg = QPalette::HighlightedText;
    }
    QColor bgc(opt->palette.color(bg));
    if (Settings::conf.pushbtn.tint.second > -1)
        bgc = Color::mid(bgc, Settings::conf.pushbtn.tint.first, 100-Settings::conf.pushbtn.tint.second, Settings::conf.pushbtn.tint.second);
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED && !(option->state & (State_On|State_NoChange)))
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }

    QLinearGradient lg(0, 0, 0, Render::maskHeight(Settings::conf.pushbtn.shadow, checkRect.height()));
    lg.setStops(Settings::gradientStops(Settings::conf.pushbtn.gradient, bgc));
    QBrush mask(lg);
    Render::drawClickable(Settings::conf.pushbtn.shadow, checkRect, painter, Render::All, 3, Settings::conf.shadows.opacity, widget, &mask);

    if (opt->state & (State_On|State_NoChange))
        Ops::drawCheckMark(painter, opt->palette.color(fg), checkRect.shrinked(2).translated(1, -1), opt->state & State_NoChange);

    return true;
}

bool
StyleProject::drawCheckBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}

/* radiobuttons */

bool
StyleProject::drawRadioButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Button, opt, option);
    if (!opt)
        return true;

    QPalette::ColorRole bg(QPalette::Button), fg(QPalette::ButtonText);
    QRect checkRect(subElementRect(SE_RadioButtonIndicator, opt, widget));
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
        checkRect.setBottom(qMin(checkRect.bottom(), widget->rect().bottom()));
        checkRect.setWidth(checkRect.height());
    }

    QRect textRect(subElementRect(SE_RadioButtonContents, opt, widget));
    int hor(opt->direction==Qt::LeftToRight?Qt::AlignLeft:Qt::AlignRight);
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->text, fg);
//    Render::renderShadow(Render::Raised, checkRect, painter);

    if (opt->state & State_On)
    {
        fg = QPalette::HighlightedText;
        bg = QPalette::Highlight;
    }

    QColor bgc(opt->palette.color(bg));
    if (Settings::conf.pushbtn.tint.second > -1)
        bgc = Color::mid(bgc, Settings::conf.pushbtn.tint.first, 100-Settings::conf.pushbtn.tint.second, Settings::conf.pushbtn.tint.second);
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED && !(option->state & State_On))
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }
    QLinearGradient lg(0, 0, 0, Render::maskHeight(Settings::conf.pushbtn.shadow, checkRect.height()));
    lg.setStops(Settings::gradientStops(Settings::conf.pushbtn.gradient, bgc));
    QBrush mask(lg);
    Render::drawClickable(Settings::conf.pushbtn.shadow, checkRect, painter, Render::All, MAXRND, Settings::conf.shadows.opacity, widget, &mask);

    if (opt->state & State_On)
    {
        painter->save();
        painter->setBrush(opt->palette.color(fg));
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawEllipse(checkRect.adjusted(6, 6, -6, -6));
        painter->restore();
    }
    return true;
}

bool
StyleProject::drawRadioButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}

bool
StyleProject::drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
}

static QRect maskRect(const QRect &rect)
{
    QRect r(rect);
    switch (Settings::conf.toolbtn.shadow)
    {
    case Render::Sunken:
    case Render::Etched:
    case Render::Raised:
        r.adjust(0, 1, 0, -2);
        break;
    case Render::Simple:
        r.adjust(0, 0, 0, -1);
        break;
    case Render::Carved:
        r.adjust(0, 3, 0, -3);
        break;
    default: break;
    }
    return r;
}

class IconCheck
{
public:
    inline IconCheck(const QWidget *w, const QColor &c, const int &s) : widget(w), color(c), size(s) {}
    inline const bool operator==(const IconCheck &check) const { return check.color==color&&check.widget==widget&&check.size==size;}
    QColor color;
    const QWidget *widget;
    int size;
};

bool
StyleProject::drawToolButton(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ToolButton, opt, option);
    if (!opt)
        return true;

    painter->save();

    const QToolButton *btn = qobject_cast<const QToolButton *>(widget);
    const QToolBar *bar(0);
    if (widget)
        bar = qobject_cast<const QToolBar *>(widget->parentWidget());

    Render::Sides sides = Render::All;
    bool nextSelected(false), prevSelected(false), isInTopToolBar(false);
    Ops::toolButtonData(btn, pixelMetric(PM_ToolBarSeparatorExtent, opt, widget), nextSelected, prevSelected, isInTopToolBar, sides);
    QRect rect(opt->rect);
    QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
    QRect mr(maskRect(rect));
    const bool hasMenu(Ops::hasMenu(btn, opt));
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::ButtonText)), fg(Ops::fgRole(widget, QPalette::ButtonText));
    if (bar)
    {
        if (hasMenu && arrow.isValid())
            painter->setClipRegion(QRegion(rect)-QRegion(arrow));

        QColor bc(option->palette.color(QPalette::Active, bg));
        if (Settings::conf.toolbtn.tint.second > -1)
            bc = Color::mid(bc, Settings::conf.toolbtn.tint.first, 100-Settings::conf.toolbtn.tint.second, Settings::conf.toolbtn.tint.second);
        QColor bca(bc);
        QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);

        Render::Shadow shadow(Settings::conf.toolbtn.shadow);
        bool ns(false);
        if (option->SUNKEN)
        {
            if (shadow == Render::Etched)
                shadow = Render::Sunken;
            if (!btn->property("DSP_menupress").toBool())
            {
                if (Settings::conf.toolbtn.invAct)
                    bc = Color::mid(bc, opt->palette.color(fg), 1, 2);
                else
                    bc = sc;
                if (Color::luminosity(bc) < Color::luminosity(bar->palette().color(bar->backgroundRole())))
                    ns = true;
            }
        }
        else if (option->ENABLED)
        {
            int hl(Anim::ToolBtns::level(btn, false));
            bc = Color::mid(bc, sc, STEPS-hl, hl);
        }

        QLinearGradient lg(0, 0, 0, Render::maskHeight(Settings::conf.toolbtn.shadow, rect.height()));
        lg.setStops(Settings::gradientStops(Settings::conf.toolbtn.gradient, bc));
        QBrush mask(lg);
        Render::drawClickable(shadow, rect, painter, sides, Settings::conf.toolbtn.rnd, Settings::conf.shadows.opacity, widget, &mask, 0, ns);

        if (hasMenu)
        {
            if (option->SUNKEN && btn->property("DSP_menupress").toBool())
                bca = sc;
            else
            {
                const int hla(Anim::ToolBtns::level(btn, true));
                bca = Color::mid(bca, sc, STEPS-hla, hla);
            }

            painter->setClipRect(arrow);
            QLinearGradient lga(0, 0, 0, Render::maskHeight(Settings::conf.toolbtn.shadow, rect.height()));
            lga.setStops(Settings::gradientStops(Settings::conf.toolbtn.gradient, bca));
            QBrush amask(lga);
            Render::drawClickable(shadow, rect, painter, sides, Settings::conf.toolbtn.rnd, Settings::conf.shadows.opacity, widget, &amask, 0, ns);
        }

        if (option->SUNKEN && Settings::conf.toolbtn.shadow == Render::Etched && Render::pos(sides, bar->orientation()) != Render::Alone)
        {
            QPixmap pix(rect.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            const Render::Sides inv(Render::All-sides);
            const QRect mr(Render::maskRect(shadow, rect, sides));
            Render::renderShadow(shadow, mr, &pt, Settings::conf.toolbtn.rnd, inv, Settings::conf.shadows.opacity);
            pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            Render::renderShadow(shadow, mr, &pt, Settings::conf.toolbtn.rnd, sides, Settings::conf.shadows.opacity);
            pt.end();
            painter->drawTiledPixmap(rect, pix);
        }

        painter->setClipping(false);

        if (!(sides&Render::Right) && !nextSelected && bar->orientation() == Qt::Horizontal)
        {
            painter->setPen(QColor(0, 0, 0, 32));
            painter->drawLine(mr.topRight(), mr.bottomRight());
        }
    }

    if (hasMenu)
    {
        mr.setRight(arrow.left());
        painter->setPen(QColor(0, 0, 0, 32));
        painter->drawLine(mr.topRight(), mr.bottomRight());
        Ops::drawArrow(painter, opt->palette.color(QPalette::ButtonText), arrow, Ops::Down, Qt::AlignCenter, 7);
    }
//    const bool hor(!bar||bar->orientation() == Qt::Horizontal);
    QRect ir(mr);
    const Render::Pos rp(Render::pos(sides, bar?bar->orientation():Qt::Horizontal));
    switch (opt->toolButtonStyle)
    {
    case Qt::ToolButtonTextBesideIcon:
        if (rp == Render::First || rp == Render::Alone)
            ir.translate(6, 0);
        else
            ir.translate(4, 0);
        ir.setRight(ir.left()+opt->iconSize.width());
        mr.setLeft(ir.right());
        break;
    case Qt::ToolButtonTextUnderIcon:
        ir.setBottom(ir.top()+opt->iconSize.height());
        mr.setTop(ir.bottom());
        break;
    default: break;
    }

    if (Settings::conf.toolbtn.shadow == Render::Carved
            && opt->toolButtonStyle == Qt::ToolButtonIconOnly
            && bar && bar->orientation() == Qt::Horizontal)
    {
        if (rp == Render::First)
            ir.translate(2, 0);
        else if (rp == Render::Last && !hasMenu)
            ir.translate(-2, 0);
    }
    const bool inDock(widget&&widget->objectName().startsWith("qt_dockwidget"));
    QPalette::ColorRole textRole(bar?QPalette::ButtonText:widget->parentWidget()?widget->parentWidget()->foregroundRole():QPalette::WindowText);
    if (opt->SUNKEN && Settings::conf.toolbtn.invAct)
        textRole = bar?bg:fg;
    if (opt->toolButtonStyle != Qt::ToolButtonTextOnly)
    {
        QPixmap pix = opt->icon.pixmap(opt->iconSize, opt->ENABLED ? QIcon::Normal : QIcon::Disabled);
        if (!pix.isNull())
        {
            const bool isDark(opt->SUNKEN && bar && Color::luminosity(opt->palette.color(bg)) > Color::luminosity(opt->palette.color(fg)));
            if (Settings::conf.toolbtn.folCol && bar)
            {
                pix = opt->icon.pixmap(opt->iconSize, QIcon::Normal);
                static QList<IconCheck> s_list;
                static QList<QPixmap> s_pix;
                const IconCheck check(widget, opt->palette.color(textRole), opt->iconSize.height());
                if (s_list.contains(check))
                    pix = s_pix.at(s_list.indexOf(check));
                else
                {
                    pix = Render::monochromized(pix, opt->palette.color(textRole), Render::Inset, isDark);
                    s_list << check;
                    s_pix << pix;
                }
            }
            drawItemPixmap(painter, inDock?widget->rect():ir, Qt::AlignCenter, pix);
        }
    }
    if (opt->toolButtonStyle)
        drawItemText(painter, mr, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, textRole);
    painter->restore();
    return true;
}
