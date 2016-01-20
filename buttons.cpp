
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
#include "stylelib/gfx.h"
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
        const bool sunken = isSunken(opt);
        if (sunken)
            bc = bc.darker(150);
        if (dConf.pushbtn.tint.second > -1)
            bc = Color::mid(bc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);
        QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);

        if (sunken || opt->features & QStyleOptionButton::DefaultButton)
            bc = sc;
        else if (isEnabled(opt))
        {
            int hl(Anim::Basic::level(widget));
            bc = Color::mid(bc, sc, Steps-hl, hl);
        }
        QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
        lg.setStops(DSP::Settings::gradientStops(dConf.pushbtn.gradient, bc));
        GFX::drawClickable(dConf.pushbtn.shadow, opt->rect, painter, lg, dConf.pushbtn.rnd, All, option, widget);
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
        drawItemText(painter, r, Qt::AlignCenter, opt->palette, isEnabled(opt), opt->text, fg);
    else
        drawItemPixmap(painter, r, Qt::AlignCenter, opt->icon.pixmap(opt->iconSize, isEnabled(opt)?QIcon::Normal:QIcon::Disabled));
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
    const bool smallTick(dConf.pushbtn.shadow == Yosemite||dConf.pushbtn.shadow == ElCapitan);
    QRect checkRect(realCheckRect.shrinked((smallTick?2:3)));
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
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, isEnabled(opt), opt->text, widget&&widget->parentWidget()?widget->parentWidget()->foregroundRole():fg);

    QColor bgc(opt->palette.color(bg));
    if (dConf.pushbtn.tint.second > -1)
        bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);

    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);

    if (opt->state & (State_On|State_NoChange))
        bgc = smallTick?opt->palette.color(QPalette::Highlight):sc;
    else if (isEnabled(option))
    {
        const int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, Steps-hl, hl);
    }

    QLinearGradient lg(checkRect.topLeft(), checkRect.bottomLeft());
    lg.setStops(Settings::gradientStops(dConf.pushbtn.gradient, bgc));

    GFX::drawClickable(dConf.pushbtn.shadow, checkRect, painter, lg, 3, All, option, widget);

    if (smallTick)
        checkRect.shrink(3);
    else
        checkRect.translate(2, -1);

    if (opt->state & (State_On|State_NoChange))
        GFX::drawCheckMark(painter, opt->palette.color(smallTick?QPalette::HighlightedText:fg), checkRect, opt->state & State_NoChange, isEnabled(opt));
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

    const QRect realCheckRect(subElementRect(SE_RadioButtonIndicator, opt, widget));
    const bool smallTick(dConf.pushbtn.shadow == Yosemite||dConf.pushbtn.shadow == ElCapitan);
    QRect checkRect(realCheckRect.shrinked((smallTick?2:3)));
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
        checkRect.setBottom(qMin(checkRect.bottom(), widget->rect().bottom()));
        checkRect.setWidth(checkRect.height());
    }

    QRect textRect(subElementRect(SE_RadioButtonContents, opt, widget));
    int hor(opt->direction==Qt::LeftToRight?Qt::AlignLeft:Qt::AlignRight);
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, isEnabled(opt), opt->text, widget&&widget->parentWidget()?widget->parentWidget()->foregroundRole():fg);
//    Render::renderShadow(Raised, checkRect, painter);

    if (isOn(opt))
    {
        fg = QPalette::HighlightedText;
        bg = QPalette::Highlight;
    }

    QColor bgc(opt->palette.color(bg));
    if (dConf.pushbtn.tint.second > -1)
        bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (isEnabled(opt) && !isOn(opt))
    {
        int hl(Anim::Basic::level(widget));
        bgc = Color::mid(bgc, sc, Steps-hl, hl);
    }

    QLinearGradient lg(checkRect.topLeft(), checkRect.bottomLeft());
    lg.setStops(Settings::gradientStops(dConf.pushbtn.gradient, bgc));
    GFX::drawClickable(dConf.pushbtn.shadow, checkRect, painter, lg, MaxRnd, All, option, widget);

    if (isOn(opt))
    {
        const int s((checkRect.width()+GFX::shadowMargin(dConf.pushbtn.shadow))/3);
        GFX::drawRadioMark(painter, opt->palette.color(fg), checkRect.adjusted(s, s, -s, -s), isEnabled(opt));
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
    p->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
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
        GFX::drawShadow(sunken?Sunken:Etched, option->rect, painter, isEnabled(opt), dConf.toolbtn.rnd);
        return true;
    }
    if (widget->inherits("KMultiTabBarTab"))
        return drawMultiTabBarTab(opt, painter, widget);

    const QToolButton *btn = qobject_cast<const QToolButton *>(widget);
    const QToolBar *bar = widget?qobject_cast<const QToolBar *>(widget->parentWidget()):0;
    const quint8 hover(sunken?Steps:Anim::ToolBtns::level(btn));

    if (dConf.toolbtn.flat || !bar)
    {
        if (hover)
        {
            QColor h(opt->palette.color(QPalette::Highlight));
            h.setAlpha(63/Steps*hover);
            GFX::drawMask(option->rect.adjusted(0, 0, -0, -1), painter, h, dConf.toolbtn.rnd);
        }
        if (sunken)
            GFX::drawShadow(Sunken, option->rect, painter, isEnabled(opt), dConf.toolbtn.rnd);
        return true;
    }

    if (!bar)
        return true;

    Sides sides(Handlers::ToolBar::sides(btn));
    QRect rect(opt->rect);

    if (dConf.dfmHacks && dConf.app == DSP::Settings::DFM && btn && (btn->toolTip() == "Go Back" || btn->toolTip() == "Go Forward"))
    {
        if (sides & Right)
            rect.translate(-1, 0);
        sides = All;
    }

    QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
    const bool hasMenu(Ops::hasMenu(btn, opt));
    const bool arrowPress(Handlers::ToolBar::isArrowPressed(btn));
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Button)), fg(Ops::fgRole(widget, QPalette::ButtonText));

    const bool hor(bar->orientation() == Qt::Horizontal);
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
        bc = Color::mid(bc, sc, Steps-hover, hover);
    if (Color::luminosity(bc) < Color::luminosity(bar->palette().color(bar->backgroundRole()))*0.8f)
        ns = true;

    if (dConf.differentInactive && shadow == Yosemite && !Handlers::Window::isActiveWindow(bar->window()))
    {
        shadow = Rect;
        bc.setAlpha(127);
    }

    static QMap<quint64, QPixmap> s_map;
    const quint64 check(bc.rgba() | (hor?rect.height():rect.width())<<32 | bar->orientation()<<40);
    if (!s_map.contains(check))
    {
        QPixmap pix(!hor?rect.width():1, hor?rect.height():1);
        if (bc.alpha() != 0xff)
            pix.fill(Qt::transparent);
        QPainter p(&pix);
        QLinearGradient lg(pix.rect().topLeft(), hor?pix.rect().bottomLeft():pix.rect().topRight());
        lg.setStops(DSP::Settings::gradientStops(dConf.toolbtn.gradient, bc));
        p.fillRect(pix.rect(), lg);
        s_map.insert(check, pix);
    }
    GFX::drawClickable(shadow, rect, painter, s_map.value(check), dConf.toolbtn.rnd, sides, opt, widget);

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
        QLinearGradient lga(rect.topLeft(), hor?rect.bottomLeft():rect.topRight());
        lga.setStops(Settings::gradientStops(dConf.toolbtn.gradient, bca));
        GFX::drawClickable(shadow, rect, painter, lga, dConf.toolbtn.rnd, sides, opt, widget);
    }

    if (sunken && dConf.toolbtn.shadow == Etched && sides != All)
    {
        QPixmap pix(rect.size());
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        const Sides saved(sides);
        const quint8 sm = GFX::shadowMargin(Sunken);
        QRect mr(rect.sAdjusted(sm, sm, -sm, -sm));
        sides = All-sides;
        mr.sAdjust(-sm, -sm, sm, sm);
        sides = saved;
        GFX::drawShadow(Sunken, mr, &pt, isEnabled(opt), dConf.toolbtn.rnd, All-sides);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        GFX::drawShadow(Sunken, mr, &pt, isEnabled(opt), dConf.toolbtn.rnd, sides);
        pt.end();
        painter->drawTiledPixmap(rect, pix);
    }

    painter->setClipping(false);

    bool nextSelected(false);
    if (bar->orientation() == Qt::Horizontal && !(sides & Right))
        if (const QToolButton *next = qobject_cast<const QToolButton *>(bar->childAt(btn->geometry().topRight()+QPoint(2, 0))))
            nextSelected = next->isChecked();
    if (bar->orientation() == Qt::Vertical && !(sides & Bottom))
        if (const QToolButton *next = qobject_cast<const QToolButton *>(bar->childAt(btn->geometry().bottomRight()+QPoint(0, 2))))
            nextSelected = next->isChecked();

    const int nextSide=hor?Right:Bottom;
    if (!(sides&nextSide) && !(nextSelected&&dConf.toolbtn.invAct))
    {
        const QPen &pen(painter->pen());
        painter->setPen(QColor(0, 0, 0, 32));
        const quint8 m = GFX::shadowMargin(shadow);
        const QRect mr(opt->rect.sAdjusted(m, m, -m, -m));
        QPoint first(hor?mr.topRight():mr.bottomLeft());
        QPoint second(hor?mr.bottomRight():mr.bottomRight());
        painter->drawLine(first, second);
        painter->setPen(pen);
    }
    return true;
}

bool
Style::drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return true;

    const bool sunken(opt->state & (State_Sunken | State_Selected | State_On));

    painter->save();

    const QToolButton *btn = qobject_cast<const QToolButton *>(widget);
    const QToolBar *bar(widget?qobject_cast<const QToolBar *>(widget->parentWidget()):0);

    const bool hor(!bar||bar->orientation() == Qt::Horizontal);
    const bool multiTab(widget && widget->inherits("KMultiTabBarTab"));
    Sides sides(Handlers::ToolBar::sides(btn));
    QRect rect(opt->rect);

    if (dConf.dfmHacks && dConf.app == DSP::Settings::DFM && btn && (btn->toolTip() == "Go Back" || btn->toolTip() == "Go Forward"))
    {
        if (sides & Left)
            rect.translate(-1, 0);
        sides = All;
    }

    QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
    const quint8 sm = GFX::shadowMargin(dConf.toolbtn.shadow);
    QRect mr(multiTab?rect:rect.sAdjusted(sm, sm, -sm, -sm));
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
        GFX::drawArrow(painter, opt->palette.color(fg), arrow, South, 7, Qt::AlignCenter, true);
    }
    QRect ir(mr);
    if (!multiTab)
    switch (opt->toolButtonStyle)
    {
    case Qt::ToolButtonTextBesideIcon:
        if (hor)
        {
            if ((sides & Left && !(sides & Right)) || sides == All)
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
        if ((sides & Left && !(sides & Right)))
            ir.translate(2, 0);
        else if ((sides & Right && !(sides & Left)) && !hasMenu)
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
    else if (sunken && dConf.toolbtn.invAct && (!btn || !arrowPress))
    {
        textRole = bar?bg:fg;
        bg = Ops::opposingRole(textRole);
    }
    if (bar && dConf.toolbtn.shadow == Rect)
    {
        fg = textRole = sunken?QPalette::HighlightedText:QPalette::WindowText;
        bg = sunken?QPalette::Highlight:QPalette::Window;
    }
    if (!bar && widget && widget->parentWidget())
    {
        fg = textRole = widget->parentWidget()->foregroundRole();
        bg = widget->parentWidget()->backgroundRole();
    }

    if (opt->toolButtonStyle != Qt::ToolButtonTextOnly)
    {
        QPixmap pix = opt->icon.pixmap(opt->iconSize, isEnabled(opt) ? QIcon::Normal : QIcon::Disabled);
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
        drawItemText(painter, mr, Qt::AlignCenter, opt->palette, isEnabled(opt), opt->text, textRole);
    }
    painter->restore();
    return true;
}
