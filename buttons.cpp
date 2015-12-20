
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
#include <QLineEdit>
#include <QCheckBox>

#include "dsp.h"
#include "stylelib/render.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "stylelib/animhandler.h"
#include "stylelib/shadows.h"
#include "config/settings.h"
#include "stylelib/handlers.h"
#include "stylelib/masks.h"
#include "stylelib/fx.h"

using namespace DSP;

bool
Style::drawPushButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option);
    if (!opt)
        return true;

    drawPushButtonBevel(option, painter, widget);
    drawPushButtonLabel(option, painter, widget);
    return true;
}

bool
Style::drawPushButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option);
    if (!opt)
        return true;
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Button))/*, fg(Ops::fgRole(widget, QPalette::ButtonText))*/;
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
            bc = Color::mid(bc, sc, Steps-hl, hl);
        }

        const QRect maskRect(Render::maskRect(dConf.pushbtn.shadow, opt->rect));
        QLinearGradient lg(0, 0, 0, maskRect.height());
        lg.setStops(DSP::Settings::gradientStops(dConf.pushbtn.gradient, bc));
        QBrush m(lg);
        Render::drawClickable(dConf.pushbtn.shadow, opt->rect, painter, dConf.pushbtn.rnd, dConf.shadows.opacity, widget, option, &m);
    }
    return true;
}

bool
Style::drawPushButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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
Style::drawCheckBox(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionButton *opt = qstyleoption_cast<const QStyleOptionButton *>(option);
    if (!opt)
        return true;

    QPalette::ColorRole bg(QPalette::Button), fg(QPalette::WindowText);
    const QRect realCheckRect(subElementRect(SE_CheckBoxIndicator, opt, widget));
    QRect checkRect(realCheckRect.shrinked(3));
    if (widget)
    {
        if (qobject_cast<const QCheckBox *>(widget))
        {
            bg = widget->backgroundRole();
            fg = widget->foregroundRole();
        }
        checkRect.setBottom(qMin(checkRect.bottom(), widget->rect().bottom()));
        checkRect.setWidth(checkRect.height());
    }

    QRect textRect(subElementRect(SE_CheckBoxContents, opt, widget));
    int hor(opt->direction==Qt::LeftToRight?Qt::AlignLeft:Qt::AlignRight);
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->text, widget&&widget->parentWidget()?widget->parentWidget()->foregroundRole():fg);

    QColor bgc(opt->palette.color(bg));
    if (dConf.pushbtn.tint.second > -1)
        bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);

    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);

    if (opt->state & (State_On|State_NoChange))
        bgc = sc;
    else if (option->ENABLED)
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, Steps-hl, hl);
    }

    QLinearGradient lg(0, 0, 0, Render::maskHeight(dConf.pushbtn.shadow, checkRect.height()));
    lg.setStops(DSP::Settings::gradientStops(dConf.pushbtn.gradient, bgc));
    QBrush mask(lg);
    Render::drawClickable(dConf.pushbtn.shadow, checkRect, painter, 3, dConf.shadows.opacity, widget, option, &mask);

    if (opt->state & (State_On|State_NoChange))
        Render::drawCheckMark(painter, opt->palette.color(fg), realCheckRect.adjusted(3, 0, 0, -3), opt->state & State_NoChange);
    return true;
}

bool
Style::drawCheckBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return true;
}

/* radiobuttons */

bool
Style::drawRadioButton(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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
//    Render::renderShadow(Raised, checkRect, painter);

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
        bgc = Color::mid(bgc, sc, Steps-hl, hl);
    }
    QLinearGradient lg(0, 0, 0, Render::maskHeight(dConf.pushbtn.shadow, checkRect.height()));
    lg.setStops(DSP::Settings::gradientStops(dConf.pushbtn.gradient, bgc));
    QBrush mask(lg);
    Render::drawClickable(dConf.pushbtn.shadow, checkRect, painter, MaxRnd, dConf.shadows.opacity, widget, option, &mask);

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
Style::drawRadioButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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
Style::drawMultiTabBarTab(const QStyleOption *opt, QPainter *p, const QWidget *mt) const
{
    const int hl(Anim::Basic::level(mt));
    const bool sunken(isSunken(opt));
    if (sunken || isMouseOver(opt) || hl)
    {
        QColor c(opt->palette.color(QPalette::ButtonText));

        c.setAlpha(sunken?63:(63.0f/Steps)*hl);
        p->fillRect(opt->rect, c);
    }
    p->save();
    p->setBrush(Qt::NoBrush);
    p->setPen(QColor(0, 0, 0, dConf.shadows.opacity*255.0f));
    if (mt->geometry().bottom() != mt->parentWidget()->rect().bottom())
        p->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
    if (mt->geometry().right() != mt->parentWidget()->rect().right())
        p->drawLine(opt->rect.topRight(), opt->rect.bottomRight());
    p->restore();
    return true;
}


bool
Style::drawToolButton(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
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
Style::drawToolButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return true;

    const bool sunken(opt->state & (State_Sunken | State_Selected | State_On));
    if (!widget || (dConf.toolbtn.flat && sunken))
    {
        Render::drawShadow(sunken?Sunken:Etched, option->rect, painter, dConf.toolbtn.rnd, All, dConf.shadows.opacity);
        return true;
    }
    if (widget->inherits("KMultiTabBarTab"))
        return drawMultiTabBarTab(opt, painter, widget);

    const QToolButton *btn = qobject_cast<const QToolButton *>(widget);
    const QToolBar *bar = widget?qobject_cast<const QToolBar *>(widget->parentWidget()):0;
    int hover[2];
    for (int i = 0; i < 2; ++i)
        hover[i] = Anim::ToolBtns::level(btn, i);

    if (sunken)
        hover[0] = Steps;

    if (dConf.toolbtn.flat||!bar)
    {
        if (hover[0] || sunken)
            Render::drawShadow(opt->SUNKEN?Sunken:Etched, option->rect, painter, dConf.toolbtn.rnd, All, (dConf.shadows.opacity/Steps)*hover[0]);
        return true;
    }

    Sides sides(All);
    bool nextSelected(false), prevSelected(false), isInTopToolBar(false);
    Ops::toolButtonData(btn, nextSelected, prevSelected, isInTopToolBar, sides);

    painter->save();
    QRect rect(opt->rect);

    if (dConf.dfmHacks && dConf.app == DSP::Settings::DFM && btn && (btn->toolTip() == "Go Back" || btn->toolTip() == "Go Forward"))
    {
        if (sides & Right)
            rect.translate(-1, 0);
        sides = All;
    }

    QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
    QRect mr(Render::maskRect(dConf.toolbtn.shadow, rect, sides));
    const bool hasMenu(Ops::hasMenu(btn, opt));
    const bool arrowPress(Handlers::ToolBar::isArrowPressed(btn));
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Button)), fg(Ops::fgRole(widget, QPalette::ButtonText));
    if (bar)
    {
        const int hor(bar->orientation() == Qt::Horizontal);
        if (hasMenu && arrow.isValid())
            painter->setClipRegion(QRegion(rect)-QRegion(arrow));

        QColor bc(option->palette.color(QPalette::Active, bg));
        if (dConf.toolbtn.tint.second > -1)
            bc = Color::mid(bc, dConf.toolbtn.tint.first, 100-dConf.toolbtn.tint.second, dConf.toolbtn.tint.second);
        QColor bca(bc);
        QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);

        ShadowStyle shadow(dConf.toolbtn.shadow);
        bool ns(false);
        if (sunken)
        {
            if (shadow == Etched)
                shadow = Sunken;
            if (!arrowPress)
            {
                if (dConf.toolbtn.invAct)
                    bc = Color::mid(bc, opt->palette.color(fg), 1, 2);
                else
                    bc = sc;
            }
        }
        else if (isEnabled(opt))
        {
            int hl(Anim::ToolBtns::level(btn, false));
            bc = Color::mid(bc, sc, Steps-hl, hl);
        }
        if (Color::luminosity(bc) < Color::luminosity(bar->palette().color(bar->backgroundRole()))*0.8f)
            ns = true;


        QLinearGradient lg(0, 0, !hor*Render::maskWidth(dConf.toolbtn.shadow, rect.width()), hor*Render::maskHeight(dConf.toolbtn.shadow, rect.height()));
        if (dConf.differentInactive && shadow == Yosemite && !Handlers::Window::isActiveWindow(bar->window()))
        {
            shadow = Rect;
            bc.setAlpha(127);
        }
        lg.setStops(DSP::Settings::gradientStops(dConf.toolbtn.gradient, bc));
        QBrush mask(lg);
        bc.setAlpha(255);
        Render::drawClickable(shadow, rect, painter, dConf.toolbtn.rnd, dConf.shadows.opacity, widget, opt, /*noMask?0:*/&mask, 0, sides);

        if (hasMenu)
        {
            if (sunken && arrowPress)
                bca = sc;
            else
            {
                const int hla(Anim::ToolBtns::level(btn, true));
                bca = Color::mid(bca, sc, Steps-hla, hla);
            }

            painter->setClipRect(arrow);
            QLinearGradient lga(0, 0, !hor*Render::maskWidth(dConf.toolbtn.shadow, rect.width()), hor*Render::maskHeight(dConf.toolbtn.shadow, rect.height()));
            lga.setStops(DSP::Settings::gradientStops(dConf.toolbtn.gradient, bca));
            QBrush amask(lga);
            Render::drawClickable(shadow, rect, painter, dConf.toolbtn.rnd, dConf.shadows.opacity, widget, opt, &amask, 0, sides);
        }

        if (sunken && dConf.toolbtn.shadow == Etched && Render::pos(sides, bar->orientation()) != Alone)
        {
            QPixmap pix(rect.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            const Sides inv(All-sides);
            const QRect mr(Render::maskRect(shadow, rect, sides));
            Render::drawShadow(shadow, mr, &pt, dConf.toolbtn.rnd, inv, dConf.shadows.opacity);
            pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            Render::drawShadow(shadow, mr, &pt, dConf.toolbtn.rnd, sides, dConf.shadows.opacity);
            pt.end();
            painter->drawTiledPixmap(rect, pix);
        }

        painter->setClipping(false);

        const int nextSide=hor?Right:Bottom;
        if (!(sides&nextSide) && !nextSelected)
        {
            painter->setPen(QColor(0, 0, 0, 32));
            QPoint first(hor?mr.topRight():mr.bottomLeft());
            QPoint second(hor?mr.bottomRight():mr.bottomRight());
            painter->drawLine(first, second);
        }
    }
    painter->restore();
    return true;
}

bool
Style::drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return true;

    painter->save();

    const QToolButton *btn = qobject_cast<const QToolButton *>(widget);
    const QToolBar *bar(widget?qobject_cast<const QToolBar *>(widget->parentWidget()):0);

    const bool hor(!bar||bar->orientation() == Qt::Horizontal);
    const bool multiTab(widget && widget->inherits("KMultiTabBarTab"));
    Sides sides = All;
    bool nextSelected(false), prevSelected(false), isInTopToolBar(false);
    Ops::toolButtonData(btn, nextSelected, prevSelected, isInTopToolBar, sides);

    QRect rect(opt->rect);

    if (dConf.dfmHacks && dConf.app == DSP::Settings::DFM && btn && (btn->toolTip() == "Go Back" || btn->toolTip() == "Go Forward"))
    {
        if (sides & Left)
            rect.translate(-1, 0);
        sides = All;
    }

    QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
    QRect mr(multiTab?rect:Render::maskRect(dConf.toolbtn.shadow, rect, sides));
    const bool hasMenu(Ops::hasMenu(btn, opt));
    const bool isFlat(dConf.toolbtn.flat);
    QPalette::ColorRole bg(Ops::bgRole(isFlat?bar:widget, isFlat?QPalette::Window:QPalette::Button)),
            fg(Ops::fgRole(isFlat?bar:widget, isFlat?QPalette::WindowText:QPalette::ButtonText));

    const bool arrowPress(Handlers::ToolBar::isArrowPressed(btn));

    if (hasMenu)
    {
        if (hor)
            mr.setRight(arrow.left());
        else
            mr.setBottom(arrow.top());
        if (!dConf.toolbtn.flat && bar)
        {
            painter->setPen(QColor(0, 0, 0, 32));
            QPoint first(hor?mr.topRight():mr.bottomLeft());
            QPoint second(hor?mr.bottomRight():mr.bottomRight());
            painter->drawLine(first, second);
        }
        Render::drawArrow(painter, opt->palette.color(fg), arrow, South, 7);
    }
    QRect ir(mr);
    const Pos rp(Render::pos(sides, bar?bar->orientation():Qt::Horizontal));
    if (!multiTab)
    switch (opt->toolButtonStyle)
    {
    case Qt::ToolButtonTextBesideIcon:
        if (hor)
        {
            if (rp == First || rp == Alone)
                ir.translate(6, 0);
            else
                ir.translate(4, 0);
            ir.setRight(ir.left()+opt->iconSize.width());
            mr.setLeft(ir.right());
        }
        else
        {
            ir.setHeight(opt->iconSize.height());
            ir.moveTop(ir.top()+4);
            mr.setTop(ir.bottom());
        }
        break;
    case Qt::ToolButtonTextUnderIcon:
        ir.setBottom(ir.top()+opt->iconSize.height());
        if (!hor)
            ir.moveTop(ir.top()+4);
        mr.setTop(ir.bottom()+(!hor*4));
        break;
    default: break;
    }

    if (!dConf.toolbtn.flat
            && dConf.toolbtn.shadow == Carved
            && opt->toolButtonStyle == Qt::ToolButtonIconOnly
            && bar && bar->orientation() == Qt::Horizontal)
    {
        if (rp == First)
            ir.translate(2, 0);
        else if (rp == Last && !hasMenu)
            ir.translate(-2, 0);
    }
    const bool inDock(widget&&widget->objectName().startsWith("qt_dockwidget"));
    QPalette::ColorRole textRole(QPalette::WindowText);
    if (bar)
        textRole = QPalette::ButtonText;
    else if (widget && widget->parentWidget())
        textRole = widget->parentWidget()->foregroundRole();
    if (multiTab)
    {
        textRole = QPalette::ButtonText;
        bg = QPalette::Button;
    }
    else if (isFlat || !bar)
    {
        textRole = QPalette::WindowText;
        bg = Ops::opposingRole(textRole);
    }
    else if (opt->SUNKEN && dConf.toolbtn.invAct && (!btn || !arrowPress))
    {
        textRole = bar?bg:fg;
        bg = Ops::opposingRole(textRole);
    }
    if (bar && dConf.toolbtn.shadow == Rect)
    {
        fg = textRole = opt->SUNKEN?QPalette::HighlightedText:QPalette::WindowText;
        bg = opt->SUNKEN?QPalette::Highlight:QPalette::Window;
    }
    if (!bar && widget && widget->parentWidget())
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
                    pix = FX::monochromized(pix, opt->palette.color(textRole), Inset, isDark);
                    s_list << check;
                    s_pix << pix;
                }
            }
            drawItemPixmap(painter, inDock?widget->rect():ir, Qt::AlignCenter, pix);
        }
    }
    if (opt->toolButtonStyle)
    {
        if (bar && !hor)
        {
            painter->translate(mr.center());
            painter->rotate(-90);
            painter->translate(-mr.center());
            const QRect tmp(mr);
            QSize sz(mr.size());
            sz.transpose();
            mr.setSize(sz);
            mr.moveCenter(tmp.center());
        }
        drawItemText(painter, mr, Qt::AlignCenter, opt->palette, opt->ENABLED, opt->text, textRole);
    }
    painter->restore();
    return true;
}
