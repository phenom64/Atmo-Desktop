
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
#include "stylelib/handlers.h"

/*
 * This here paints the button, in order to override
 * QCommonStyle painting, simply return true here
 * and paint what you want yourself.
 */

bool
StyleProject::drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option);
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
    const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option);
    if (!opt)
        return true;
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::ButtonText)), fg(Ops::fgRole(widget, QPalette::ButtonText));
    if (!(opt->features & QStyleOptionButton::Flat))
    {
        QColor bc(option->palette.color(bg));
        if (option->SUNKEN)
            bc = bc.darker(150);
        if (dConf.pushbtn.tint.second > -1)
            bc = Color::mid(bc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);
        QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);

        if (option->SUNKEN || opt->features & QStyleOptionButton::DefaultButton)
            bc = sc;
        else if (option->ENABLED)
        {
            int hl(Anim::Basic::level(widget));
            bc = Color::mid(bc, sc, STEPS-hl, hl);
        }

        const QRect maskRect(Render::maskRect(dConf.pushbtn.shadow, opt->rect));
        QLinearGradient lg(0, 0, 0, maskRect.height());
        lg.setStops(Settings::gradientStops(dConf.pushbtn.gradient, bc));
        QBrush m(lg);
        Render::drawClickable(dConf.pushbtn.shadow, opt->rect, painter, dConf.pushbtn.rnd, dConf.shadows.opacity, widget, option, &m);
    }
    return true;
}

bool
StyleProject::drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option);
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
    const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option);
    if (!opt)
        return true;

    QPalette::ColorRole bg(QPalette::Button), fg(QPalette::WindowText);
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
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->text, widget&&widget->parentWidget()?widget->parentWidget()->foregroundRole():fg);

    if (opt->state & (State_On|State_NoChange))
    {
        bg = QPalette::Highlight;
        fg = QPalette::HighlightedText;
    }
    QColor bgc(opt->palette.color(bg));
    if (dConf.pushbtn.tint.second > -1)
        bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED && !(option->state & (State_On|State_NoChange)))
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }

    QLinearGradient lg(0, 0, 0, Render::maskHeight(dConf.pushbtn.shadow, checkRect.height()));
    lg.setStops(Settings::gradientStops(dConf.pushbtn.gradient, bgc));
    QBrush mask(lg);
    Render::drawClickable(dConf.pushbtn.shadow, checkRect, painter, 3, dConf.shadows.opacity, widget, option, &mask);

    if (opt->state & (State_On|State_NoChange))
        Ops::drawCheckMark(painter, opt->palette.color(fg), checkRect.shrinked(2).translated(1, -1), opt->state & State_NoChange);

    return true;
}

bool
StyleProject::drawCheckBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return true;
}

/* radiobuttons */

bool
StyleProject::drawRadioButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option);
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
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->text, widget&&widget->parentWidget()?widget->parentWidget()->foregroundRole():fg);
//    Render::renderShadow(Render::Raised, checkRect, painter);

    if (opt->state & State_On)
    {
        fg = QPalette::HighlightedText;
        bg = QPalette::Highlight;
    }

    QColor bgc(opt->palette.color(bg));
    if (dConf.pushbtn.tint.second > -1)
        bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED && !(option->state & State_On))
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }
    QLinearGradient lg(0, 0, 0, Render::maskHeight(dConf.pushbtn.shadow, checkRect.height()));
    lg.setStops(Settings::gradientStops(dConf.pushbtn.gradient, bgc));
    QBrush mask(lg);
    Render::drawClickable(dConf.pushbtn.shadow, checkRect, painter, MAXRND, dConf.shadows.opacity, widget, option, &mask);

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

class IconCheck
{
public:
    inline IconCheck(const QWidget *w, const QColor &c, const int &s, const QString &n) : widget(w), color(c), size(s), name(n) {}
    inline const bool operator==(const IconCheck &check) const { return check.color==color&&check.widget==widget&&check.size==size&&check.name==name; }
    QColor color;
    const QWidget *widget;
    QString name;
    int size;
};

bool
StyleProject::drawToolButton(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return true;

    drawToolButtonBevel(option, painter, widget);
    if (!(opt->icon.isNull() && opt->text.isEmpty()))
        drawToolButtonLabel(option, painter, widget);
    return true;
}

bool
StyleProject::drawToolButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return true;

    if (widget && widget->inherits("KMultiTabBarTab"))
    {
        const int hl(Anim::Basic::level(widget));
        if (opt->SUNKEN || opt->HOVER || hl)
        {
            QColor c(opt->palette.color(QPalette::ButtonText));

            c.setAlpha(opt->SUNKEN?63:(63.0f/STEPS)*hl);
            painter->fillRect(opt->rect, c);
        }
        painter->save();
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity*255.0f));
        if (widget->geometry().bottom() != widget->parentWidget()->rect().bottom())
            painter->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
        if (widget->geometry().right() != widget->parentWidget()->rect().right())
            painter->drawLine(opt->rect.topRight(), opt->rect.bottomRight());
        painter->restore();
        return true;
    }

    if (!widget || (dConf.toolbtn.flat&&option->SUNKEN))
    {
        Render::renderShadow(option->SUNKEN?Render::Sunken:Render::Etched, option->rect, painter, dConf.toolbtn.rnd, Render::All, dConf.shadows.opacity);
        return true;
    }
    const QToolButton *btn = qobject_cast<const QToolButton *>(widget);
    const QToolBar *bar = qobject_cast<const QToolBar *>(widget->parentWidget());
    int hover[2];
    for (int i = 0; i < 2; ++i)
        hover[i] = Anim::ToolBtns::level(btn, i);

    if (opt->SUNKEN)
        hover[0] = STEPS;

    if (dConf.toolbtn.flat||!bar)
    {
        if (hover[0] || opt->SUNKEN)
            Render::renderShadow(opt->SUNKEN?Render::Sunken:Render::Etched, option->rect, painter, dConf.toolbtn.rnd, Render::All, (dConf.shadows.opacity/STEPS)*hover[0]);
        return true;
    }

    Render::Sides sides = Render::All;
    bool nextSelected(false), prevSelected(false), isInTopToolBar(false);
    Ops::toolButtonData(btn, pixelMetric(PM_ToolBarSeparatorExtent, opt, widget), nextSelected, prevSelected, isInTopToolBar, sides);

    if (dConf.toolbtn.shadow == Render::Rect)
    {
        const QPalette pal(bar?bar->palette():opt->palette);
        int hl(Anim::ToolBtns::level(btn, false));
        if (opt->SUNKEN)
            hl = STEPS;
        QColor bc = Color::mid(pal.foreground().color(), pal.highlight().color(), STEPS-hl, hl);
        QBrush brush(bc);
        if (opt->SUNKEN)
        {
            Render::renderMask(opt->rect.sShrinked(1), painter, brush, dConf.toolbtn.rnd, sides);
            brush = pal.foreground();
        }
        Render::renderShadow(Render::Rect, opt->rect, painter, dConf.toolbtn.rnd, sides, 1.0f, &brush);
        return true;
    }

    painter->save();
    QRect rect(opt->rect);
    QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
    QRect mr(Render::maskRect(dConf.toolbtn.shadow, rect, sides));
    const bool hasMenu(Ops::hasMenu(btn, opt));
    const bool arrowPress(Handlers::ToolBar::isArrowPressed(btn));
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Button)), fg(Ops::fgRole(widget, QPalette::ButtonText));
    if (bar)
    {
        if (hasMenu && arrow.isValid())
            painter->setClipRegion(QRegion(rect)-QRegion(arrow));

        QColor bc(option->palette.color(QPalette::Active, bg));
        if (dConf.toolbtn.tint.second > -1)
            bc = Color::mid(bc, dConf.toolbtn.tint.first, 100-dConf.toolbtn.tint.second, dConf.toolbtn.tint.second);
        QColor bca(bc);
        QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);

        Render::Shadow shadow(dConf.toolbtn.shadow);
        bool ns(false);
        if (option->SUNKEN)
        {
            if (shadow == Render::Etched)
                shadow = Render::Sunken;
            if (!arrowPress)
            {
                if (dConf.toolbtn.invAct)
                    bc = Color::mid(bc, opt->palette.color(fg), 1, 2);
                else
                    bc = sc;
            }
        }
        else if (option->ENABLED)
        {
            int hl(Anim::ToolBtns::level(btn, false));
            bc = Color::mid(bc, sc, STEPS-hl, hl);
        }
        if (Color::luminosity(bc) < Color::luminosity(bar->palette().color(bar->backgroundRole()))*0.8f)
            ns = true;

        QLinearGradient lg(0, 0, 0, Render::maskHeight(dConf.toolbtn.shadow, rect.height()));
        lg.setStops(Settings::gradientStops(dConf.toolbtn.gradient, bc));
        QBrush mask(lg);
        Render::drawClickable(shadow, rect, painter, dConf.toolbtn.rnd, dConf.shadows.opacity, widget, opt, &mask, 0, sides);

        if (hasMenu)
        {
            if (option->SUNKEN && arrowPress)
                bca = sc;
            else
            {
                const int hla(Anim::ToolBtns::level(btn, true));
                bca = Color::mid(bca, sc, STEPS-hla, hla);
            }

            painter->setClipRect(arrow);
            QLinearGradient lga(0, 0, 0, Render::maskHeight(dConf.toolbtn.shadow, rect.height()));
            lga.setStops(Settings::gradientStops(dConf.toolbtn.gradient, bca));
            QBrush amask(lga);
            Render::drawClickable(shadow, rect, painter, dConf.toolbtn.rnd, dConf.shadows.opacity, widget, opt, &amask, 0, sides);
        }

        if (option->SUNKEN && dConf.toolbtn.shadow == Render::Etched && Render::pos(sides, bar->orientation()) != Render::Alone)
        {
            QPixmap pix(rect.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            const Render::Sides inv(Render::All-sides);
            const QRect mr(Render::maskRect(shadow, rect, sides));
            Render::renderShadow(shadow, mr, &pt, dConf.toolbtn.rnd, inv, dConf.shadows.opacity);
            pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            Render::renderShadow(shadow, mr, &pt, dConf.toolbtn.rnd, sides, dConf.shadows.opacity);
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
    painter->restore();
    return true;
}

bool
StyleProject::drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
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
    QRect mr(Render::maskRect(dConf.toolbtn.shadow, rect, sides));
    const bool hasMenu(Ops::hasMenu(btn, opt));
    const bool isFlat(dConf.toolbtn.flat);
    QPalette::ColorRole bg(Ops::bgRole(isFlat?bar:widget, isFlat?QPalette::Window:QPalette::Button)),
            fg(Ops::fgRole(isFlat?bar:widget, isFlat?QPalette::WindowText:QPalette::ButtonText));

    const bool arrowPress(Handlers::ToolBar::isArrowPressed(btn));

    if (hasMenu)
    {
        mr.setRight(arrow.left());
        painter->setPen(QColor(0, 0, 0, 32));
        painter->drawLine(mr.topRight(), mr.bottomRight());
        Ops::drawArrow(painter, opt->palette.color(fg), arrow, Ops::Down, 7);
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

    if (!dConf.toolbtn.flat
            && dConf.toolbtn.shadow == Render::Carved
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
    if (isFlat || !bar)
    {
        textRole = QPalette::WindowText;
        bg = Ops::opposingRole(textRole);
    }
    else if (opt->SUNKEN && dConf.toolbtn.invAct && (!btn || !arrowPress))
    {
        textRole = bar?bg:fg;
        bg = Ops::opposingRole(textRole);
    }
    if (bar && dConf.toolbtn.shadow == Render::Rect)
    {
        fg = textRole = opt->SUNKEN?QPalette::HighlightedText:QPalette::WindowText;
        bg = opt->SUNKEN?QPalette::Highlight:QPalette::Window;
    }
    if (!bar && widget->parentWidget())
    {
        fg = textRole = widget->parentWidget()->foregroundRole();
        bg = widget->parentWidget()->backgroundRole();
    }

    if (opt->toolButtonStyle != Qt::ToolButtonTextOnly)
    {
        QPixmap pix = opt->icon.pixmap(opt->iconSize, opt->ENABLED ? QIcon::Normal : QIcon::Disabled);
        if (!pix.isNull())
        {
            if (dConf.toolbtn.folCol && bar)
            {
                const bool isDark(Color::luminosity(opt->palette.color(textRole)) > Color::luminosity(opt->palette.color(bg)));
                pix = opt->icon.pixmap(opt->iconSize, QIcon::Normal);
                static QList<IconCheck> s_list;
                static QList<QPixmap> s_pix;
                const IconCheck check(widget, opt->palette.color(textRole), opt->iconSize.height(), opt->icon.name());
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
