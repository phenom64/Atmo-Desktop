
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
#include <QHBoxLayout>

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
    if (!opt || (widget&&widget->inherits("KColorButton")))
        return true;
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Button))/*, fg(Ops::fgRole(widget, QPalette::ButtonText))*/;
    int hl(0);
    if (isEnabled(opt))
        hl = Anim::Basic::level(widget);
    const bool sunken = isSunken(opt);
    if (opt->features & QStyleOptionButton::Flat)
    {
        const bool selected = isSelected(option);
        if (hl || sunken || selected)
        {
            QColor h(opt->palette.color(QPalette::Highlight));
            h.setAlpha(63/Steps*hl);
            GFX::drawClickable(sunken||selected?Sunken:-1, option->rect, painter, h, dConf.pushbtn.rnd);
        }
        return true;
    }
    QColor bc(option->palette.color(bg));

    if (sunken)
        bc = bc.darker(110);
    if (dConf.pushbtn.tint.second > -1)
        bc = Color::mid(bc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);
//    QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);

    if (sunken || opt->features & QStyleOptionButton::DefaultButton)
    {
        hl = qMax(Steps/2, hl);
//        bc = sc;
    }
//    else if (isEnabled(opt))
//        bc = Color::mid(bc, sc, Steps-hl, hl);
    QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
    lg.setStops(DSP::Settings::gradientStops(dConf.pushbtn.gradient, bc));
    GFX::drawClickable(dConf.pushbtn.shadow, opt->rect, painter, lg, dConf.pushbtn.rnd, hl, All, option, widget);
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
    const bool isDefault = opt->features & QStyleOptionButton::DefaultButton;
    if (!opt->text.isEmpty())
//        drawItemText(painter, r, Qt::AlignCenter, opt->palette, isEnabled(opt), opt->text, fg);
        drawText(r, painter, opt->text, opt, Qt::AlignCenter, fg, Qt::ElideNone, isDefault, isDefault);
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
    const QRect realCheckRect = subElementRect(SE_CheckBoxIndicator, opt, widget);
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

    QRect textRect = subElementRect(SE_CheckBoxContents, opt, widget);
    int hor(opt->direction==Qt::LeftToRight?Qt::AlignLeft:Qt::AlignRight);
    drawItemText(painter, textRect, hor|Qt::AlignVCenter, opt->palette, isEnabled(opt), opt->text, widget&&widget->parentWidget()?widget->parentWidget()->foregroundRole():fg);

    QColor bgc(opt->palette.color(bg));
    if (dConf.pushbtn.tint.second > -1)
        bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);

    QColor sc = Color::mid(bgc, opt->palette.color(fg), 10, 1);

    const int hl(Anim::Basic::level(widget));
    if (opt->state & (State_On|State_NoChange))
        bgc = smallTick?opt->palette.color(QPalette::Highlight):sc;
//    else if (isEnabled(option))
//        bgc = Color::mid(bgc, sc, Steps-hl, hl);

    QLinearGradient lg(checkRect.topLeft(), checkRect.bottomLeft());
    lg.setStops(Settings::gradientStops(dConf.pushbtn.gradient, bgc));

    GFX::drawClickable(dConf.pushbtn.shadow, checkRect, painter, lg, qMin<int>(3, dConf.pushbtn.rnd), hl, All, option, widget);

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

    if (isOn(opt) && smallTick)
    {
        fg = QPalette::HighlightedText;
        bg = QPalette::Highlight;
    }

    QColor bgc(opt->palette.color(bg));

    if (dConf.pushbtn.tint.second > -1)
        bgc = Color::mid(bgc, dConf.pushbtn.tint.first, 100-dConf.pushbtn.tint.second, dConf.pushbtn.tint.second);

    QColor sc = Color::mid(bgc, opt->palette.color(fg), 10, 1);

    const int hl = Anim::Basic::level(widget);
    if (isOn(opt))
        bgc = smallTick?opt->palette.color(QPalette::Highlight):sc;
//    else if (isEnabled(opt))
//        bgc = Color::mid(bgc, sc, Steps-hl, hl);

    QLinearGradient lg(checkRect.topLeft(), checkRect.bottomLeft());
    lg.setStops(Settings::gradientStops(dConf.pushbtn.gradient, bgc));
    GFX::drawClickable(dConf.pushbtn.shadow, checkRect, painter, lg, MaxRnd, hl, All, option, widget);

    if (isOn(opt))
    {
        const int s((checkRect.width()+GFX::shadowMargin(dConf.pushbtn.shadow))/3);
        GFX::drawRadioMark(painter, opt->palette.color(smallTick?QPalette::HighlightedText:fg), checkRect.adjusted(s, s, -s, -s), isEnabled(opt));
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
Style::drawToolButtonArrow(const QStyleOptionToolButton *opt, QPainter *p, const QWidget *w) const
{
    switch (opt->arrowType)
    {
    case Qt::UpArrow: return drawArrowNorth(opt, p, w);
    case Qt::DownArrow: return drawArrowSouth(opt, p, w);
    case Qt::LeftArrow: return drawArrowWest(opt, p, w);
    case Qt::RightArrow: return drawArrowEast(opt, p, w);
    default: break;
    }
    return true;
}

static bool drawExtensionArrow(const QStyleOptionToolButton *opt, QPainter *p, const QWidget *w)
{
    const bool simple = dConf.simpleArrows;
    dConf.simpleArrows = false;
    QRect first(opt->rect);
    first.setRight(first.center().x());
    QRect second(opt->rect);
    second.setLeft(first.right());
    GFX::drawArrow(p, opt->palette.color(QPalette::WindowText), first.translated(-1, 0), East, dConf.arrowSize, Qt::AlignVCenter|Qt::AlignRight, true);
    GFX::drawArrow(p, opt->palette.color(QPalette::WindowText), second.translated(1, 0), East, dConf.arrowSize, Qt::AlignVCenter|Qt::AlignLeft, true);
    dConf.simpleArrows = simple;
    return true;
}

bool
Style::drawToolButton(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return true; 
    if (widget && widget->inherits("QToolBarExtension"))
        return drawExtensionArrow(opt, painter, widget);
    drawToolButtonBevel(option, painter, widget);
    drawToolButtonLabel(option, painter, widget);
    return true;
}

static Sides btnSides(const QAbstractButton *btn, QWidget *parent)
{
    Sides sides(All);
    if (qobject_cast<QAbstractButton *>(parent->childAt(btn->geometry().topLeft()-QPoint(1, 0))))
        sides&=~Left;
    if (qobject_cast<QAbstractButton *>(parent->childAt(btn->geometry().topRight()+QPoint(2, 0))))
        sides&=~Right;
    if (qobject_cast<QAbstractButton *>(parent->childAt(btn->geometry().topLeft()-QPoint(0, 1))))
        sides&=~Top;
    if (qobject_cast<QAbstractButton *>(parent->childAt(btn->geometry().bottomLeft()+QPoint(0, 2))))
        sides&=~Bottom;
    return sides;
}

static bool normalButton(const QAbstractButton *btn)
{
    if (!btn || !btn->parentWidget() || !btn->parentWidget()->layout())
        return false;

    QWidget *p = btn->parentWidget();
    if (qobject_cast<QTabBar *>(p)
            || p->inherits("TabBar")
            || p->property("DSP_konsoleTabBarParent").toBool()
            || (p->autoFillBackground() && p->backgroundRole() != QPalette::Window))
        return false;
    return true;
}

bool
Style::drawToolButtonBevel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return true;

    bool sunken(opt->state & (State_Sunken | State_Selected | State_On));
    if (!widget || (dConf.toolbtn.flat && sunken))
    {
        GFX::drawShadow(sunken?Sunken:Etched, option->rect, painter, isEnabled(opt), dConf.toolbtn.rnd);
        return true;
    }

    const QAbstractButton *btn = qobject_cast<const QAbstractButton *>(widget);
    const QToolButton *tbtn = qobject_cast<const QToolButton *>(widget);
    QToolBar *bar = qobject_cast<QToolBar *>(widget->parentWidget());
    quint8 hover = sunken ? Steps : tbtn ? Anim::ToolBtns::level(tbtn) : Anim::Basic::level(btn);
    const bool normal = normalButton(btn);

    if ((dConf.toolbtn.flat || !normal) && (hover || sunken))
    {
        QColor h(opt->palette.color(QPalette::Highlight));
        h.setAlpha(63/Steps*hover);
        GFX::drawClickable(sunken?Sunken:-1, option->rect, painter, h, dConf.toolbtn.rnd, hover);
    }

    if (!normal)
        return true;

    ///Begin actual toolbutton bevel painting, for real toolbuttons... in toolbars!!!!
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Button)), fg(Ops::fgRole(widget, QPalette::ButtonText));
    QRect rect(opt->rect);
    QColor bc(option->palette.color(QPalette::Active, bg));
    if (dConf.toolbtn.tint.second > -1)
        bc = Color::mid(bc, dConf.toolbtn.tint.first, 100-dConf.toolbtn.tint.second, dConf.toolbtn.tint.second);

    bool inActiveWindow;
    const bool uno = bar ? inUno(bar, &inActiveWindow) : false;
    if (dConf.differentInactive && uno && inActiveWindow)
        bc = bc.darker(110);

    QColor bca(bc);
    QColor sc = Color::mid(bc, opt->palette.color(QPalette::Highlight), 2, 1);
    const ShadowStyle shadow(dConf.differentInactive && shadow == Yosemite && !inActiveWindow ? Rect : dConf.toolbtn.shadow);
    const quint8 m = qMax<quint8>(1, GFX::shadowMargin(shadow));;

    Sides sides = bar ? Handlers::ToolBar::sides(tbtn) : btnSides(btn, widget->parentWidget());

    const bool hor(!widget
                   || (bar&&bar->orientation() == Qt::Horizontal)
                   || (widget->parentWidget()
                       && (!widget->parentWidget()->layout()
                           || qobject_cast<QHBoxLayout *>(widget->parentWidget()->layout())))
                   || widget->width() > widget->height());
    Sides restoreSide(0);
    if (opt->features & QStyleOptionToolButton::MenuButtonPopup)
    {
        const QRect arrow(subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget));
        if (sunken && Handlers::ToolBar::isArrowPressed(tbtn))
        {
            bca = sc;
            sunken = false;
            hover = 0;
        }
        else
        {
            const int hla(Anim::ToolBtns::level(tbtn, true));
            bca = Color::mid(bca, sc, Steps-hla, hla);
        }
        QLinearGradient lga(rect.topLeft(), hor?rect.bottomLeft():rect.topRight());
        lga.setStops(Settings::gradientStops(sunken?dConf.toolbtn.activeGradient:dConf.toolbtn.gradient, bca));
        GFX::drawClickable(shadow, arrow, painter, lga, dConf.toolbtn.rnd, hover, sides & ~(hor?Left:Top), opt, widget);
        GFX::drawShadow(Rect, arrow.adjusted(!hor*m, hor*m, -(!hor*m), -(hor*m)), painter, false, MaxRnd, hor?Left:Top); //line...
        if (hor)
            rect.setRight(arrow.left()-1);
        else
            rect.setBottom(arrow.top()-1);
        restoreSide = hor ? (sides & Right) : (sides & Bottom);
        sides &= ~restoreSide;
    }

    if (sunken)
    {
        if (dConf.toolbtn.invAct)
            bc = Color::mid(bc, opt->palette.color(fg), 1, 2);
        else
            bc = sc;
    }
//    else if (isEnabled(opt))
//        bc = Color::mid(bc, sc, Steps-hover, hover);

    static QMap<quint64, QPixmap> s_map;
    const quint64 check(bc.rgba() | (quint64)(hor?rect.height():rect.width())<<32 | (quint64)(hor?Qt::Horizontal:Qt::Vertical)<<40);
    if (!s_map.contains(check))
    {
        QPixmap pix(!hor?rect.width():1, hor?rect.height():1);
        if (bc.alpha() != 0xff)
            pix.fill(Qt::transparent);
        QPainter p(&pix);
        QLinearGradient lg(pix.rect().topLeft(), hor?pix.rect().bottomLeft():pix.rect().topRight());
        lg.setStops(DSP::Settings::gradientStops(sunken?dConf.toolbtn.activeGradient:dConf.toolbtn.gradient, bc));
        p.fillRect(pix.rect(), lg);
        s_map.insert(check, pix);
    }
    GFX::drawClickable(shadow, rect, painter, s_map.value(check), dConf.toolbtn.rnd, hover, sides, opt, widget, QPoint(), true);
    sides |= restoreSide;

    bool nextSelected(false);
    if (QWidget *p = widget->parentWidget())
    {
        if (hor && !(sides & Right))
            if (const QAbstractButton *next = qobject_cast<const QAbstractButton *>(p->childAt(btn->geometry().topRight()+QPoint(2, 0))))
                nextSelected = next->isChecked();
        if (!hor && !(sides & Bottom))
            if (const QAbstractButton *next = qobject_cast<const QAbstractButton *>(p->childAt(btn->geometry().bottomRight()+QPoint(0, 2))))
                nextSelected = next->isChecked();
    }

    const int nextSide=hor?Right:Bottom;
    if (!sunken && !(sides&nextSide))
    {
        QRect r = opt->rect.sAdjusted(m, m, -m, -m);
        if (hor)
            r.setLeft(r.right()); //right() actually returns x+w-1
        else
            r.setTop(r.bottom()); //as right()
        const int v = nextSelected ? 255 : 0;
        const int o = nextSelected ? dConf.shadows.illumination : (dConf.shadows.opacity>>(!isEnabled(opt)));
        painter->fillRect(r, QColor(v,v,v,o));
    }
    if (!(sides & (hor ? Left : Top)) && !sunken)
    {
        QRect r = opt->rect.sAdjusted(m, m, -m, -m);
        if (hor)
            r.setWidth(1);
        else
            r.setHeight(1);
        painter->fillRect(r, QColor(255,255,255,dConf.shadows.illumination));
    }
    return true;
}

bool
Style::drawToolButtonLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolButton *opt = qstyleoption_cast<const QStyleOptionToolButton *>(option);
    if (!opt)
        return true;


    static const bool isFlat(dConf.toolbtn.flat);
    static const quint8 sm = GFX::shadowMargin(dConf.toolbtn.shadow);

    const QAbstractButton *btn = qobject_cast<const QAbstractButton *>(widget);
    const QToolButton *tbtn = qobject_cast<const QToolButton *>(widget);
    const bool normal = normalButton(btn);
    const QToolBar *bar = !isFlat && widget ? qobject_cast<const QToolBar *>(widget->parentWidget()) : 0;

    const bool hor = !bar || bar->orientation() == Qt::Horizontal;
    const bool sunken = opt->state & (State_Sunken | State_Selected | State_On);
    const bool multiTab = widget && widget->inherits("KMultiTabBarTab");
    const bool hasMenu = opt->features & QStyleOptionToolButton::MenuButtonPopup;

    Sides sides = tbtn ? Handlers::ToolBar::sides(tbtn) : btnSides(btn, btn->parentWidget());
    QRect rect = opt->rect;
    QRect arrow = subControlRect(CC_ToolButton, opt, SC_ToolButtonMenu, widget);
    QRect mr = multiTab?rect:(rect.sAdjusted(sm, sm, -sm, -sm));

    QPalette::ColorRole bg = normal||multiTab ? QPalette::Button : QPalette::Window;
    QPalette::ColorRole fg = normal||multiTab ? QPalette::ButtonText : QPalette::WindowText;
    if (widget)
    {
        fg = widget->foregroundRole();
        bg = widget->backgroundRole();
        QWidget *parent = widget->parentWidget();
        if (!normal && parent && parent != bar
                && parent->autoFillBackground()
                && parent->backgroundRole() != QPalette::Window)
        {
            bg = parent->backgroundRole();
            fg = parent->foregroundRole();
        }
    }

    if (hasMenu)
    {
        if (hor)
            mr.setRight(arrow.left());
        else
            mr.setBottom(arrow.top());
        GFX::drawArrow(painter, opt->palette.color(fg), arrow, South, 7, Qt::AlignCenter, isEnabled(opt));
    }

    QRect ir(mr);
    const bool hasIndicator((opt->features & QStyleOptionToolButton::Arrow) && opt->arrowType);
    if (hasIndicator)
    {
        ir.setRight(hor?ir.left()+opt->iconSize.width():opt->rect.right());
        mr.setLeft(hor?ir.right():opt->rect.left());
    }
    if (!multiTab && opt->toolButtonStyle == Qt::ToolButtonTextBesideIcon)
    {
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
    }
    else if (opt->toolButtonStyle == Qt::ToolButtonTextUnderIcon)
    {
        ir.setBottom(ir.top()+opt->iconSize.height());
        if (!hor)
            ir.moveTop(ir.top()+4);
        mr.setTop(ir.bottom()+(!hor*4));
    }
    if (!dConf.toolbtn.flat
            && dConf.toolbtn.shadow == Carved
            && opt->toolButtonStyle == Qt::ToolButtonIconOnly
            && hor)
    {
        if ((sides & Left) && !(sides & Right))
            ir.translate(2, 0);
        else if ((sides & Right && !(sides & Left)) && !hasMenu)
            ir.translate(-2, 0);
    }
    const bool inDock(widget&&widget->objectName().startsWith("qt_dockwidget"));
    if (sunken && dConf.toolbtn.invAct && (!btn || !Handlers::ToolBar::isArrowPressed(tbtn)))
    {
        fg = bar||normal?bg:fg;
        bg = Ops::opposingRole(fg);
    }

    if (opt->toolButtonStyle != Qt::ToolButtonTextOnly)
    {
        QPixmap orgPix;
        QPixmap pix = orgPix = opt->icon.pixmap(opt->iconSize, isEnabled(opt) ? QIcon::Normal : QIcon::Disabled);
        if (!pix.isNull())
        {
            if (dConf.toolbtn.folCol && bar)
            {
                const bool isDark(Color::lum(opt->palette.color(fg)) > Color::lum(opt->palette.color(bg)));
                pix = opt->icon.pixmap(opt->iconSize, QIcon::Normal);
                static QList<IconCheck> s_list;
                static QList<QPixmap> s_pix;

                const IconCheck check(widget, opt->palette.color(fg), opt->iconSize.height(), opt->icon.name());
                if (s_list.contains(check))
                    pix = s_pix.at(s_list.indexOf(check));
                else
                {
                    const QColor &c = isDark ? opt->palette.color(fg) : Color::mid(opt->palette.color(fg), opt->palette.color(bg), 3, 1);
                    pix = FX::monochromized(pix, c, Inset, isDark);
                    s_list << check;
                    s_pix << pix;
                }
            }

            const quint8 hover = (dConf.toolbtn.folCol&&dConf.toolbtn.morph)*Anim::ToolBtns::level(tbtn);
            if (hover && !inDock)
            {
                const int alpha = (255/Steps) * hover;
                QPainter p(&orgPix);
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                p.fillRect(orgPix.rect(), QColor(0, 0, 0, alpha));
                p.end();
                p.begin(&pix);
                p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
                p.fillRect(pix.rect(), QColor(0, 0, 0, alpha));
                p.end();

            }
            drawItemPixmap(painter, inDock?widget->rect():ir, Qt::AlignCenter, pix);
            if (hover && !inDock)
                drawItemPixmap(painter, ir, Qt::AlignCenter, orgPix);
        }
    }
    if (hasIndicator)
    {
        QStyleOptionToolButton copy(*opt);
        copy.rect = ir;
        drawToolButtonArrow(&copy, painter, widget);
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
//        drawItemText(painter, mr, Qt::AlignCenter, opt->palette, isEnabled(opt), opt->text, fg);
        drawText(mr, painter, opt->text, opt, Qt::AlignCenter, fg, Qt::ElideNone, sunken, sunken);
        painter->resetTransform();
    }
    return true;
}
