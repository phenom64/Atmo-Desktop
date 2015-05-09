#include "decobutton.h"
#include "kwinclient2.h"
#include "../stylelib/color.h"

///-------------------------------------------------------------------------------------------------

DSPButton::DSPButton(QObject *parent, const QVariantList &args)
    : ButtonBase((ButtonBase::Type)args.at(0).value<KDecoration2::DecorationButtonType>())
    , DecorationButton(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<KDecoration2::Decoration*>(), parent)
{
}

DSPButton::DSPButton(KDecoration2::DecorationButtonType type, const QPointer<KDecoration2::Decoration> &decoration, QObject *parent)
    : ButtonBase((ButtonBase::Type)type)
    , KDecoration2::DecorationButton(type, decoration, parent)
{
}

DSPButton *DSPButton::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    if (DSPDeco *d = qobject_cast<DSPDeco *>(decoration))
        return new DSPButton(type, d, parent);
    return 0;
}

void
DSPButton::paint(QPainter *painter, const QRect &repaintArea)
{
    ButtonBase::paint(*painter);
}

const bool
DSPButton::isActive() const
{
    return decoration()->client().data()->isActive();
}

const bool
DSPButton::isMaximized() const
{
    return decoration()->client().data()->isMaximized();
}

const bool
DSPButton::isDark() const
{
    KDecoration2::DecoratedClient *client = decoration()->client().data();
    if (client)
    {
        const QColor &bg = client->palette().color(QPalette::Window);
        const QColor &fg = client->palette().color(QPalette::WindowText);
        return Color::luminosity(fg) > Color::luminosity(bg);
    }
    return false;
}

const QColor
DSPButton::color(const ColorRole &c) const
{
    KDecoration2::DecoratedClient *client = decoration()->client().data();
    if (client)
    {
        if (c == Highlight)
            return client->palette().color(QPalette::Highlight);
        const QColor &fgc = client->palette().color(QPalette::WindowText);
        //    if (client->m_custcol[Fg].isValid())
        //        fgc = m_client->m_custcol[Fg];
        if (c == Fg)
            return fgc;
        const QColor &bgc = client->palette().color(QPalette::Window);
        //    if (m_client->m_custcol[Bg].isValid())
        //        bgc = m_client->m_custcol[Bg];
        if (c == Bg)
            return bgc;
        const bool isd(isDark());
        return Color::mid(fgc, bgc, 1, (!isd*4)+(!isActive()*(isd?2:8)));
    }
    return QColor();
}
