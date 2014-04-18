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
#include <QMenu>

#include "styleproject.h"
#include "stylelib/ops.h"

bool
StyleProject::drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(MenuItem, opt, option);
    if (!opt)
        return true;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QPalette::ColorRole fg(QPalette::Text), bg(QPalette::Base);
    if (widget)
    {
        fg = widget->foregroundRole();
        bg = widget->backgroundRole();
    }

    const bool isMenu(qobject_cast<const QMenu *>(widget));
    const bool isSeparator(opt->menuItemType == QStyleOptionMenuItem::Separator);
    const bool hasText(!opt->text.isEmpty());
    const bool hasRadioButton(opt->checkType == QStyleOptionMenuItem::Exclusive);
    const bool hasCheckBox(opt->checkType == QStyleOptionMenuItem::NonExclusive);
    const bool hasMenu(opt->menuItemType == QStyleOptionMenuItem::SubMenu);

    const int leftMargin(isMenu?(opt->menuHasCheckableItems?32:6):0), rightMargin(isMenu?(hasMenu?32:6):0), square(opt->rect.height());

    if (isSeparator)
    {
        painter->setPen(opt->palette.color(QPalette::Disabled, fg));
        painter->translate(0, 0.5f);
        const int top(opt->rect.top()), height(opt->rect.height()), width(opt->rect.width());
        const int y(top+(height/2));
        if (hasText)
        {
            painter->drawLine(0, y, leftMargin, y);
            painter->drawLine(width-rightMargin, y, width, y);
        }
        else
        {
            painter->drawLine(0, y, width, y);
            painter->restore();
            return true;
        }
        painter->translate(0, -0.5f);
    }

    if (!hasText)
    {
        painter->restore();
        return true;
    }

    /** For some reason 'selected' here means hover
     *  and sunken means pressed.
     */
    if (opt->state & (State_Selected | State_Sunken))
    {
        QColor h(opt->palette.color(QPalette::Highlight));
        if (!(opt->state & State_Sunken))
            h.setAlpha(64);
        else
        {
            fg = QPalette::HighlightedText;
            bg = QPalette::Highlight;
        }

        painter->fillRect(opt->rect, h);
    }

    QRect button(leftMargin/2 - square/2, opt->rect.top(), square, square);
    QRect textRect(opt->rect.adjusted(leftMargin, 0, -rightMargin, 0));
    QRect arrow(textRect.right()+((rightMargin/2)-(square/2)), opt->rect.top(), square, square);

    if (hasRadioButton)
    {
        QRect copy(button.adjusted(2, 2, -2, -2));
        painter->setBrush(opt->palette.color(fg));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(copy);
        copy.adjust(2, 2, -2, -2);
        painter->setBrush(opt->palette.color(bg));
        painter->drawEllipse(copy);
        if (opt->checked)
        {
            painter->setBrush(opt->palette.color(fg));
            copy.adjust(2, 2, -2, -2);
            painter->drawEllipse(copy);
        }
    }
    else if (hasCheckBox)
    {
        QRect copy(button.adjusted(2, 2, -2, -2));
        painter->setBrush(opt->palette.color(fg));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(copy, 3, 3);
        copy.adjust(2, 2, -2, -2);
        painter->setBrush(opt->palette.color(bg));
        painter->drawRoundedRect(copy, 1, 1);
        if (opt->checked)
        {
            copy.adjust(1, 1, -1, -1);
            Ops::drawCheckMark(painter, opt->palette.color(fg), copy);
        }
    }

    if (hasMenu)
        Ops::drawArrow(painter, opt->palette.color(fg), arrow.adjusted(6, 6, -6, -6), Ops::Right);

    QStringList text(opt->text.split("\t"));
    const int align[] = { isSeparator?Qt::AlignCenter:Qt::AlignLeft|Qt::AlignVCenter, Qt::AlignRight|Qt::AlignVCenter };
    const bool enabled[] = { opt->state & State_Enabled, false };

    for (int i = 0; i < 2 && i < text.count(); ++i)
        drawItemText(painter, textRect, align[i], opt->palette, enabled[i], text.at(i), fg);
    painter->restore();
    return true;
}

bool
StyleProject::drawViewItemBg(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ViewItemV4, opt, option);
    if (!opt)
        return true;

    if (opt->SUNKEN || opt->HOVER)
    {
        QColor h(opt->palette.color(QPalette::Highlight));
        if (!(opt->SUNKEN))
            h.setAlpha(64);

        painter->fillRect(opt->rect, h);
    }
    return true;
}

bool
StyleProject::drawViewItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ViewItemV4, opt, option);
    castObj(const QAbstractItemView *, view, widget);
    drawViewItemBg(option, painter, widget);

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
