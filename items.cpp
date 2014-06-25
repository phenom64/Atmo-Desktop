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
#include <QTableView>

#include "styleproject.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"

bool
StyleProject::drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(MenuItem, opt, option);
    if (!opt)
        return true;

    painter->save();
    QFont f(opt->font);
    bool isMenuBar(false);
    if (castObj(const QMenuBar *, bar, widget))
    {
        isMenuBar = true;
        if (QAction *a = bar->actionAt(opt->rect.center()))
            f.setBold(a->font().bold());
    }
    painter->setFont(f);
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
        drawItemText(painter, textRect, isMenuBar?Qt::AlignCenter:align[i], opt->palette, enabled[i], text.at(i), fg);
    painter->restore();
    return true;
}

bool
StyleProject::drawViewItemBg(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ViewItemV4, opt, option);
    if (!opt)
        return true;

    const bool multiSelection(qobject_cast<const QAbstractItemView *>(widget)&&static_cast<const QAbstractItemView *>(widget)->selectionMode()>QAbstractItemView::SingleSelection);
    if (opt->SUNKEN || opt->HOVER)
    {
        QColor h(opt->palette.color(QPalette::Highlight));

        if (!(opt->SUNKEN))
            h.setAlpha(64);

        QBrush brush(h);

        if (!multiSelection)
        {
            QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
            lg.setColorAt(0.0f, Color::mid(h, QColor(255, 255, 255, h.alpha()), 3, 1));
            lg.setColorAt(1.0f, Color::mid(h, QColor(0, 0, 0, h.alpha()), 3, 1));
            brush = lg;
        }

        painter->fillRect(opt->rect, brush);

        if (opt->SUNKEN && !multiSelection)
        {
            painter->save();
            painter->setPen(QColor(0, 0, 0, 64));
            painter->translate(0.0f, 0.5f);
            painter->drawLine(opt->rect.topLeft(), opt->rect.topRight());
//            painter->translate(0.0f, -1.0f);
            painter->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
            painter->restore();
        }
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
    if (qMin(option->rect.width(), option->rect.height()) < 8)
        return true;
    painter->save();
    QPalette::ColorRole fg(option->state & State_Selected ? QPalette::HighlightedText : QPalette::Text),
            bg(option->state & State_Selected ? QPalette::Highlight : QPalette::Base);

    QColor fgc(Color::mid(option->palette.color(fg), option->palette.color(bg)));
    painter->setPen(fgc);
    int mid_h = option->rect.center().x();
    int mid_v = option->rect.center().y();

    if (option->state & State_Item)
    {
        if (option->direction == Qt::RightToLeft)
            painter->drawLine(option->rect.left(), mid_v, mid_h, mid_v);
        else
            painter->drawLine(mid_h, mid_v, option->rect.right(), mid_v);
    }
    if (option->state & State_Sibling)
        painter->drawLine(mid_h, mid_v, mid_h, option->rect.bottom());
    if (option->state & (State_Open | State_Children | State_Item | State_Sibling))
        painter->drawLine(mid_h, option->rect.y(), mid_h, mid_v);

    if (option->state & State_Children)
    {
        if (option->state & State_MouseOver && !(option->state & State_Selected))
            fgc = option->palette.color(QPalette::Highlight);
        painter->translate(bool(!(option->state & State_Open)), bool(!(option->state & State_Open))?-0.5f:0);
        Ops::drawArrow(painter, fgc, option->rect, option->state & State_Open ? Ops::Down : Ops::Right, Qt::AlignCenter, 9);
    }

    painter->restore();
    return true;
}

bool
StyleProject::drawHeader(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    drawHeaderSection(option, painter, widget);
    drawHeaderLabel(option, painter, widget);
    return true;
}

bool
StyleProject::drawHeaderSection(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Header, opt, option);
    if (!opt)
        return true;
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Base));
    if (opt->sortIndicator)
        bg = QPalette::Highlight;

    QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
    const QColor b(opt->palette.color(bg));
    lg.setColorAt(0.0f, Color::mid(b, QColor(255, 255, 255, b.alpha()), 4, 1));
    lg.setColorAt(1.0f, Color::mid(b, QColor(0, 0, 0, b.alpha()), 4, 1));
    painter->fillRect(opt->rect, lg);

    const QPen pen(painter->pen());

    painter->setPen(QColor(0, 0, 0, 127));
    painter->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
    if (!(widget && opt->rect.right() == widget->rect().right()) || opt->orientation == Qt::Vertical)
        painter->drawLine(opt->rect.topRight(), opt->rect.bottomRight());

    if (widget && qobject_cast<const QTableView *>(widget->parentWidget()))
        if (opt->position == QStyleOptionHeader::Beginning && opt->orientation == Qt::Horizontal)
            painter->drawLine(opt->rect.topLeft(), opt->rect.bottomLeft());

    painter->setPen(pen);
    return true;
}

bool
StyleProject::drawHeaderLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Header, opt, option);
    if (!opt)
        return true;


    const QRect tr(subElementRect(SE_HeaderLabel, opt, widget));
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::Text));
    if (opt->sortIndicator)
    {
        const QRect ar(subElementRect(SE_HeaderArrow, opt, widget));
        fg = QPalette::HighlightedText;
        Ops::drawArrow(painter, opt->palette.color(fg), ar, opt->sortIndicator==QStyleOptionHeader::SortUp?Ops::Up:Ops::Down);
    }
    drawItemText(painter, tr, opt->textAlignment, opt->palette, opt->state & State_Enabled, opt->text, fg);
    return true;
}
