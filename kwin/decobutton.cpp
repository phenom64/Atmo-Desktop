#include "decobutton.h"
#include "kwinclient2.h"
#include "../stylelib/color.h"

#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <QPainter>
#include <QDebug>

///-------------------------------------------------------------------------------------------------

namespace DSP
{

Button::Button(QObject *parent, const QVariantList &args)
    : KDecoration2::DecorationButton(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<KDecoration2::Decoration*>(), parent)
    , ButtonBase((ButtonBase::Type)args.at(0).value<KDecoration2::DecorationButtonType>())
{
    setGeometry(QRectF(0, 0, 16, 16));
}

Button::Button(KDecoration2::DecorationButtonType type, Deco *decoration, QObject *parent)
    : KDecoration2::DecorationButton(type, decoration, parent)
    , ButtonBase((ButtonBase::Type)type)
{
    setGeometry(QRectF(0, 0, 16, 16));
}

Button
*Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    if (Deco *d = qobject_cast<Deco *>(decoration))
        return new Button(type, d, parent);
    return 0;
}

void
Button::paint(QPainter *painter, const QRect &repaintArea)
{
    ButtonBase::paint(*painter);
}

const bool
Button::isActive() const
{
    return decoration()->client().data()->isActive();
}

const bool
Button::isMaximized() const
{
    return decoration()->client().data()->isMaximized();
}

const bool
Button::keepBelow() const
{
    return decoration()->client().data()->isKeepBelow();
}

const bool
Button::keepAbove() const
{
    return decoration()->client().data()->isKeepAbove();
}

const bool
Button::onAllDesktops() const
{
    return decoration()->client().data()->isOnAllDesktops();
}

const bool
Button::shade() const
{
    return decoration()->client().data()->isShaded();
}

const bool
Button::isDark() const
{
    return Color::luminosity(color(ButtonBase::Fg)) > Color::luminosity(color(ButtonBase::Bg));
}

const QColor
Button::color(const ColorRole &c) const
{
    if (c == Mid)
    {
        const QColor &fg = static_cast<Deco *>(decoration().data())->fgColor();
        const QColor &bg = static_cast<Deco *>(decoration().data())->bgColor();
        const bool dark(Color::luminosity(fg) > Color::luminosity(bg));
        return Color::mid(fg, bg, 1, (!dark*4)+(!isActive()*(dark?2:8)));
    }
    if (c == ButtonBase::Highlight)
        return decoration()->client().data()->palette().color(QPalette::Highlight);
    if (c == ButtonBase::Fg)
        return static_cast<Deco *>(decoration().data())->fgColor();
    if (c == ButtonBase::Bg)
        return static_cast<Deco *>(decoration().data())->bgColor();
    return QColor();
}

void
Button::hoverEnterEvent(QHoverEvent *event)
{
    KDecoration2::DecorationButton::hoverEnterEvent(event);
    hover();
    update();
}

void
Button::hoverLeaveEvent(QHoverEvent *event)
{
    KDecoration2::DecorationButton::hoverLeaveEvent(event);
    unhover();
    update();
}

void
Button::mouseReleaseEvent(QMouseEvent *event)
{
    KDecoration2::DecorationButton::mouseReleaseEvent(event);
    update();
}

} //DSP

