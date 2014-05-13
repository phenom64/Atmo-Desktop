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
#include <QTreeView>

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
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::Text)), bg(Ops::bgRole(widget, QPalette::Base));

    const bool isMenu(qobject_cast<const QMenu *>(widget));
    const bool isSeparator(opt->menuItemType == QStyleOptionMenuItem::Separator);
    const bool hasText(!opt->text.isEmpty());
    const bool hasRadioButton(opt->checkType == QStyleOptionMenuItem::Exclusive);
    const bool hasCheckBox(opt->checkType == QStyleOptionMenuItem::NonExclusive);
    const bool hasMenu(opt->menuItemType == QStyleOptionMenuItem::SubMenu);

    const int leftMargin(isMenu?(opt->menuHasCheckableItems?32:6):0), rightMargin(isMenu?(hasMenu||isSeparator?32:6):0), square(opt->rect.height());

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
        fg = QPalette::HighlightedText;
        bg = QPalette::Highlight;
        painter->fillRect(opt->rect, opt->palette.color(bg));
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
    const bool enabled[] = { opt->state & State_Enabled, opt->state & State_Enabled && opt->state & (State_Selected | State_Sunken) };

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
    if (!opt)
        return true;
    castObj(const QAbstractItemView *, view, widget);
    drawViewItemBg(option, painter, widget);

    QPixmap pix(opt->icon.pixmap(opt->decorationSize));

    if (opt->features & QStyleOptionViewItemV2::HasCheckIndicator)
    {
        QStyleOptionButton btn;
//        btn.QStyleOption::operator =(*option);
        btn.rect = subElementRect(SE_ItemViewItemCheckIndicator, opt, widget);
        if (opt->checkState)
            btn.state |= (opt->checkState==Qt::PartiallyChecked?State_NoChange:State_On);
        drawCheckBox(&btn, painter, 0);
    }

    QRect iconRect(subElementRect(SE_ItemViewItemDecoration, opt, widget));
    QRect textRect(subElementRect(SE_ItemViewItemText, opt, widget));

    QPalette::ColorRole fg(opt->SUNKEN ? QPalette::HighlightedText : QPalette::Text);
    drawItemPixmap(painter, iconRect, opt->decorationAlignment, pix);
    int align(opt->displayAlignment);
    if (opt->fontMetrics.boundingRect(opt->text).width() > opt->rect.width())
        align &= ~Qt::AlignHCenter;
    drawItemText(painter, textRect, align, opt->palette, opt->ENABLED, opt->text, fg);
    return true;
}

bool
StyleProject::drawTree(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return !qobject_cast<const QTreeView *>(widget);
}
