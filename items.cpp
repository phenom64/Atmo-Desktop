#include <QMenuBar>
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionMenuItem>
#include <QStyleOptionViewItem>
#include <QStyleOptionViewItemV2>
#include <QStyleOptionViewItemV3>
#include <QStyleOptionViewItemV4>
#include <QAbstractItemView>

#include "styleproject.h"

bool
StyleProject::drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(MenuItem, opt, option);
    QPalette::ColorRole fg(QPalette::Window);
    if (widget)
        fg = widget->foregroundRole();
    if (!opt)
        return true;

    /** For some reason 'selected' here means hover
     *  and sunken means pressed.
     */
    if (opt->state & (State_Selected | State_Sunken))
    {
        QColor h(opt->palette.color(QPalette::Highlight));
        if (!(opt->state & State_Sunken))
            h.setAlpha(64);

        painter->fillRect(opt->rect, h);
    }

    if (opt->text.isEmpty())
        return true;
    QStringList text(opt->text.split("\t"));
    while (text.count() < 2)
        text << QString();
    int align[] = { Qt::AlignLeft|Qt::AlignVCenter, Qt::AlignRight|Qt::AlignVCenter };

    for (int i = 0; i < 2; ++i)
        drawItemText(painter, opt->rect, align[i], opt->palette, opt->state & State_Enabled, text.at(i), fg);
    return true;
}

bool
StyleProject::drawViewItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ViewItemV4, opt, option);
    castObj(const QAbstractItemView *, view, widget);

    if (!opt)
        return true;

    if (opt->SUNKEN || opt->HOVER)
    {
        QColor h(opt->palette.color(QPalette::Highlight));
        if (!(opt->SUNKEN))
            h.setAlpha(64);

        painter->fillRect(opt->rect, h);
    }

    QPixmap pix(opt->icon.pixmap(opt->decorationSize));
    QRect iconRect(itemPixmapRect(opt->rect, opt->decorationAlignment, pix));
    QRect textRect(opt->rect);
    int m(4);
    switch (opt->decorationPosition)
    {
    case QStyleOptionViewItem::Left: textRect.setLeft(iconRect.right()); break;
    case QStyleOptionViewItem::Right: textRect.setRight(iconRect.left()); break;
    case QStyleOptionViewItem::Top: textRect.setBottom(iconRect.top()); break;
    case QStyleOptionViewItem::Bottom: textRect.setTop(iconRect.bottom()); break;
    default: break;
    }

    if (opt->displayAlignment & Qt::AlignLeft)
        textRect.setLeft(textRect.left()+m);
    if (opt->displayAlignment & Qt::AlignTop)
        textRect.setTop(textRect.top()+m);
    if (opt->displayAlignment & Qt::AlignRight)
        textRect.setRight(textRect.right()-m);
    if (opt->displayAlignment & Qt::AlignBottom)
        textRect.setBottom(textRect.bottom()-m);

    QPalette::ColorRole fg(QPalette::Text);
    if (opt->SUNKEN)
        fg = QPalette::HighlightedText;

    drawItemPixmap(painter, iconRect, opt->decorationAlignment, pix);
    drawItemText(painter, textRect, opt->displayAlignment, opt->palette, opt->ENABLED, opt->text, fg);
    return true;
}
