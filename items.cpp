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
#include <QListView>
#include <QTableView>
#include <QApplication>

#include "dsp.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "config/settings.h"
#include "stylelib/gfx.h"

using namespace DSP;

bool
Style::drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionMenuItem *opt = qstyleoption_cast<const QStyleOptionMenuItem *>(option);
    if (!opt)
        return true;

    painter->save();
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::Text)), bg(Ops::bgRole(widget, QPalette::Base));

    const bool isMenuBar(qobject_cast<const QMenuBar *>(widget));
//    const bool isPlasmaGMenu(dConf.app == Settings::Plasma && widget->inherits("BE::GMenu"));
    const bool isMenu(qobject_cast<const QMenu *>(widget));
    const bool isSeparator(opt->menuItemType == QStyleOptionMenuItem::Separator);
    const bool hasText(!opt->text.isEmpty());
    const bool hasRadioButton(opt->checkType == QStyleOptionMenuItem::Exclusive);
    const bool hasCheckBox(opt->checkType == QStyleOptionMenuItem::NonExclusive);
    const bool hasMenu(opt->menuItemType == QStyleOptionMenuItem::SubMenu);
    const QPalette pal(opt->palette);

    int leftMargin(isMenu?(opt->menuHasCheckableItems||hasCheckBox||hasRadioButton?24:6):0), rightMargin(isMenu?(hasMenu||isSeparator?24:6):0), h(opt->rect.height()), w(opt->rect.width());

    QRect button(0, opt->rect.top(), 9, 9);
    button.moveLeft(button.left()+qMax(6, (leftMargin/2 - h/2)));
    button.moveTop(button.top()+((h/2)-(button.height()/2)));
    QRect textRect(opt->rect.adjusted(qMax(button.right()+1, leftMargin), 0, -rightMargin, 0));
    QRect arrow(textRect.right()+((rightMargin/2)-(h/2)), opt->rect.top(), h, h);

    if (isSeparator)
    {
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        painter->translate(0, 0.5f);
        const int top(opt->rect.top());
        const int y(top+(h/2));
        QFont f(painter->font());
        const QFont sf(f);
        f.setBold(true);
        painter->setFont(f);
        if (hasText)
            painter->setClipRegion(QRegion(opt->rect)-QRegion(itemTextRect(painter->fontMetrics(), opt->rect, Qt::AlignCenter, opt->ENABLED, opt->text)));
        painter->drawLine(0, y, w, y);
        if (hasText)
            painter->setClipping(false);

        painter->translate(0, -0.5f);
        if (hasText)
            drawItemText(painter, opt->rect, Qt::AlignCenter, pal, isEnabled(opt), opt->text, fg);

        painter->setFont(sf);
        painter->restore();
        return true;
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

        QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
        lg.setStops(DSP::Settings::gradientStops(dConf.menues.itemGradient, pal.color(bg)));

        Sides sides(All);
        if (!isMenuBar)
            sides &= ~(Left|Right);
        GFX::drawClickable(dConf.menues.itemShadow, opt->rect, painter, lg, dConf.pushbtn.rnd, sides);
    }

    if (dConf.menues.icons && isMenu)
    {
        QAction *a(static_cast<const QMenu *>(widget)->actionAt(opt->rect.topLeft()));
        if (a && !a->icon().isNull())
        {
            const QRect iconRect(textRect.adjusted(0, 0, -(textRect.width()-16), 0));
            textRect.setLeft(textRect.left()+20);
            const QPixmap icon(a->icon().pixmap(16, opt->ENABLED?QIcon::Normal:QIcon::Disabled));
            drawItemPixmap(painter, iconRect, Qt::AlignCenter, icon);
        }
    }

    QColor fgc(pal.color(fg));
    if (opt->checked)
        GFX::drawCheckMark(painter, fgc, button);
    else if (opt->state & (State_Selected | State_Sunken) && (hasCheckBox || hasRadioButton))
    {
        fgc.setAlpha(127);
        GFX::drawCheckMark(painter, fgc, button);
    }

    if (isMenu && hasMenu)
        GFX::drawArrow(painter, pal.color(fg), arrow.shrinked(6), East, dConf.arrowSize, Qt::AlignCenter, true);

    QStringList text(opt->text.split("\t"));
    const int align[] = { isSeparator?Qt::AlignCenter:Qt::AlignLeft|Qt::AlignVCenter, Qt::AlignRight|Qt::AlignVCenter };
    const bool enabled[] = { opt->state & State_Enabled, opt->state & State_Enabled && opt->state & (State_Selected | State_Sunken) };

    for (int i = 0; i < text.count(); ++i)
    {
        const QFont font(painter->font());
        if (isMenuBar)
        {
            const QMenuBar *bar(static_cast<const QMenuBar *>(widget));
            drawMenuBar(option, painter, widget);
            if (QAction *a = bar->actionAt(opt->rect.topLeft()))
                if (a->font().bold())
                {
                    QFont f(widget->font());
                    int regularW(QFontMetrics(f).width(text.at(i)));
                    f.setBold(true);
                    int boldW(QFontMetrics(f).width(text.at(i)));
                    if (boldW > opt->rect.width())
                    {
                        int stretch(regularW*100/boldW);
                        f.setStretch(stretch);
                    }
                    f.setBold(true);
                    painter->setFont(f);
                }
        }
        else
        {
            QFont f(font);
            f.setBold(opt->state & (State_Selected | State_Sunken));
            painter->setFont(f);
        }
        drawItemText(painter, isMenuBar?opt->rect:textRect, isMenuBar?Qt::AlignCenter:align[i], pal, enabled[i], text.at(i), fg);
        painter->setFont(font);
    }
    painter->restore();
    return true;
}

bool
Style::drawViewItemBg(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!option)
        return true;
    const QStyleOptionViewItemV4 *opt = qstyleoption_cast<const QStyleOptionViewItemV4 *>(option);
    if (dConf.app == DSP::Settings::Konversation && widget && widget->inherits("ViewTree"))
        return true;
    bool sunken(isSelected(option));
    if (!sunken && !isMouseOver(option))
        return true;

    const QAbstractItemView *abstractView = qobject_cast<const QAbstractItemView *>(widget);
    const QWidget *vp(abstractView?abstractView->viewport():0);
    const bool full(vp && vp->rect().x() == option->rect.x() && vp->rect().right() <= option->rect.right());
//    const QTreeView *treeView = qobject_cast<const QTreeView *>(widget);
//    const QListView *listView = qobject_cast<const QListView *>(widget);
//    const bool multiSelection(abstractView&&abstractView->selectionMode()>QAbstractItemView::SingleSelection);
    const bool selectRows(abstractView&&abstractView->selectionBehavior()==QAbstractItemView::SelectRows);
//    const bool iconMode(listView && listView->viewMode() == QListView::IconMode);

    if (!selectRows && (option->state & State_Item))
        return true;

    QColor h(option->palette.color(QPalette::Highlight));
    if (!sunken)
        h.setAlpha(64);

    QBrush brush(h);
    if (opt && opt->backgroundBrush != Qt::NoBrush)
        brush = opt->backgroundBrush;
    else if (sunken)
    {
        QLinearGradient lg(option->rect.topLeft(), option->rect.bottomLeft());
        lg.setStops(DSP::Settings::gradientStops(dConf.views.itemGradient, h));
        brush = lg;
    }

    QRect rect(option->rect);
    Sides sides(All);
    if (full || (sunken && selectRows))
        sides &= ~(Right|Left);
    else if (opt&&opt->viewItemPosition == QStyleOptionViewItemV4::Beginning)
        sides &= ~Right;
    else if (opt&&opt->viewItemPosition == QStyleOptionViewItemV4::End)
        sides &= ~Left;
    else if (opt&&opt->viewItemPosition == QStyleOptionViewItemV4::Middle)
        sides &= ~(Right|Left);
    else if (opt)
        rect.setRight(subElementRect(SE_ItemViewItemText, opt, widget).right());
    GFX::drawClickable(sunken?dConf.views.itemShadow:-1, rect, painter, brush, dConf.views.itemRnd, sides);
    return true;
}

bool
Style::drawViewItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionViewItemV4 *opt = qstyleoption_cast<const QStyleOptionViewItemV4 *>(option);
    if (!opt)
        return true;
    drawViewItemBg(option, painter, widget);

    if (opt->features & QStyleOptionViewItemV2::HasCheckIndicator)
    {
        QStyleOptionButton btn;
//        btn.QStyleOption::operator =(*option);
        btn.rect = subElementRect(SE_ItemViewItemCheckIndicator, opt, widget);
        if (opt->checkState)
            btn.state |= (opt->checkState==Qt::PartiallyChecked?State_NoChange:State_On);
        drawCheckBox(&btn, painter, 0);
    }
    QRect textRect(subElementRect(SE_ItemViewItemText, opt, widget));
    if (!opt->icon.isNull())
    {
        QRect iconRect(subElementRect(SE_ItemViewItemDecoration, opt, widget));
        const QPixmap pix(opt->icon.pixmap(opt->decorationSize));
        drawItemPixmap(painter, iconRect, opt->decorationAlignment, pix);
    }
    if (!opt->text.isEmpty())
    {
        QPalette::ColorRole fg(QPalette::Text);
        const QFont font(painter->font());
        QFont f(opt->font);
        painter->setFont(f);
        if (opt->SUNKEN)
        {
            fg = QPalette::HighlightedText;
            f.setBold(true);
            painter->setFont(f);
        }
        const QFontMetrics fm(painter->fontMetrics());
        const QString text(fm.elidedText(opt->text, opt->textElideMode, textRect.width()));
        drawItemText(painter, textRect, opt->displayAlignment, opt->palette, opt->ENABLED, text, fg);
        painter->setFont(font);
    }
    return true;
}

bool
Style::drawTree(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!option)
        return true;
//    if (qMin(option->rect.width(), option->rect.height()) < 8) //what? why? cant remember...
//        return true;
//    if (option->SUNKEN)

//    const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(widget);
//    QPoint pos;
//    if (view && view->viewport())
//        pos = view->viewport()->mapFromGlobal(QCursor::pos());
//    if (!pos.isNull())
//        if (pos.y() >= option->rect.y() && pos.y() <= option->rect.bottom())
//            const_cast<QStyleOption *>(option)->state |= State_MouseOver;

    const QAbstractItemView *abstractView = qobject_cast<const QAbstractItemView *>(widget);
    const bool selectRows(abstractView&&abstractView->selectionBehavior()==QAbstractItemView::SelectRows);
    const bool selected(selectRows && isSelected(option));
    if (selected)
        drawViewItemBg(option, painter, widget);
    painter->save();
    QPalette::ColorRole fg(selected ? QPalette::HighlightedText : QPalette::Text),
            bg(selected ? QPalette::Highlight : QPalette::Base);

    QColor fgc(Color::mid(option->palette.color(fg), option->palette.color(bg), 1, 2));
    painter->setPen(fgc);
    int mid_h = option->rect.center().x();
    int mid_v = option->rect.center().y();

    if (dConf.views.treelines)
    {
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
    }

    if (option->state & State_Children)
    {
        if ((option->state & State_MouseOver) && !selected)
            fgc = option->palette.color(QPalette::Highlight);
        else
        {

            fgc = option->palette.color(fg);
            if (!dConf.views.treelines)
                fgc = Color::mid(fgc, option->palette.color(bg));
        }
        painter->translate(bool(!(option->state & State_Open)), bool(!(option->state & State_Open))?-0.5f:0);
        GFX::drawArrow(painter, fgc, option->rect, option->state & State_Open ? South : East, dConf.views.treelines?7:dConf.arrowSize, Qt::AlignCenter, true);
    }

    painter->restore();
    return true;
}

bool
Style::drawHeader(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    drawHeaderSection(option, painter, widget);
    drawHeaderLabel(option, painter, widget);
    return true;
}

bool
Style::drawHeaderSection(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionHeader *opt = qstyleoption_cast<const QStyleOptionHeader *>(option);
    if (!opt)
        return true;
    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Base));
    if (opt->sortIndicator)
        bg = QPalette::Highlight;

    QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
    const QColor b(opt->palette.color(bg));
    lg.setStops(DSP::Settings::gradientStops(dConf.views.headerGradient, b));
    painter->fillRect(opt->rect, lg);

    const QPen pen(painter->pen());

    painter->setPen(QColor(0, 0, 0, 32));
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
Style::drawHeaderLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionHeader *opt = qstyleoption_cast<const QStyleOptionHeader *>(option);
    if (!opt)
        return true;

    painter->save();
    const QRect tr(subElementRect(SE_HeaderLabel, opt, widget));
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::Text));

    if (opt->sortIndicator)
    {
        BOLD;
        const QRect ar(subElementRect(SE_HeaderArrow, opt, widget));
        fg = QPalette::HighlightedText;
        GFX::drawArrow(painter, option->palette.color(fg), ar, opt->sortIndicator==QStyleOptionHeader::SortUp?North:South, dConf.arrowSize, Qt::AlignCenter, true);
    }
    const QFontMetrics fm(painter->fontMetrics());
    const QString text(fm.elidedText(opt->text, Qt::ElideRight, tr.width()));
    drawItemText(painter, tr, opt->textAlignment, opt->palette, opt->state & State_Enabled, text, fg);
    painter->restore();
    return true;
}
