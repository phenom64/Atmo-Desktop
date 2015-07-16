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

#include "styleproject.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"
#include "config/settings.h"
#include "stylelib/render.h"

bool
StyleProject::drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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

    int leftMargin(isMenu?(opt->menuHasCheckableItems?24:6):0), rightMargin(isMenu?(hasMenu||isSeparator?24:6):0), h(opt->rect.height()), w(opt->rect.width());

    QRect button(0, opt->rect.top(), h, h);
    button.moveLeft(button.left()+(leftMargin/2 - h/2));
    QRect textRect(opt->rect.adjusted(leftMargin, 0, -rightMargin, 0));
    QRect arrow(textRect.right()+((rightMargin/2)-(h/2)), opt->rect.top(), h, h);

    if (isSeparator)
    {
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity*255.0f));
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
            drawItemText(painter, opt->rect, Qt::AlignCenter, pal, opt->ENABLED, opt->text, fg);

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
        painter->setBrush(pal.color(bg));
        painter->setPen(Qt::NoPen);
        const int rnd(isMenuBar*(qMin(qMin(h, opt->rect.width())/2, dConf.pushbtn.rnd)));
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawRoundedRect(opt->rect, rnd, rnd);
        painter->setRenderHint(QPainter::Antialiasing, false);
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

    if (opt->checked)
        Ops::drawCheckMark(painter, pal.color(fg), button.shrinked(3));
    else if (opt->state & (State_Selected | State_Sunken) && (hasCheckBox || hasRadioButton))
        Ops::drawCheckMark(painter, pal.color(fg), button.shrinked(3), true);

    if (isMenu && hasMenu)
        Ops::drawArrow(painter, pal.color(fg), arrow.adjusted(6, 6, -6, -6), Ops::Right, dConf.arrowSize);

    QStringList text(opt->text.split("\t"));
    const int align[] = { isSeparator?Qt::AlignCenter:Qt::AlignLeft|Qt::AlignVCenter, Qt::AlignRight|Qt::AlignVCenter };
    const bool enabled[] = { opt->state & State_Enabled, opt->state & State_Enabled && opt->state & (State_Selected | State_Sunken) };

    for (int i = 0; i < 2 && i < text.count(); ++i)
    {
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
        drawItemText(painter, isMenuBar?opt->rect:textRect, isMenuBar?Qt::AlignCenter:align[i], pal, enabled[i], text.at(i), fg);
    }
    painter->restore();
    return true;
}

bool
StyleProject::drawViewItemBg(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionViewItemV4 *opt = qstyleoption_cast<const QStyleOptionViewItemV4 *>(option);
    if (!opt)
        return true;

    if (dConf.app == DSP::Settings::Konversation && widget && widget->inherits("ViewTree"))
        return true;
    if (opt->backgroundBrush != Qt::NoBrush)
        painter->fillRect(opt->rect, opt->backgroundBrush);

    if (!(opt->SUNKEN || opt->HOVER))
        return true;

    painter->save();
    const QAbstractItemView *abstractView = qobject_cast<const QAbstractItemView *>(widget);
//    const QTreeView *treeView = qobject_cast<const QTreeView *>(widget);
    const QListView *listView = qobject_cast<const QListView *>(widget);
    const bool multiSelection(abstractView&&abstractView->selectionMode()>QAbstractItemView::SingleSelection);
    QColor h(opt->palette.color(QPalette::Highlight));
    if (!(opt->SUNKEN))
        h.setAlpha(64);

    QBrush brush(h);

    if (!multiSelection)
    {
        QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());
        lg.setStops(DSP::Settings::gradientStops(dConf.pushbtn.gradient, h));
        brush = lg;
    }

    const int rnd(qMin(6, dConf.input.rnd)); //too much roundness just looks silly
    if (listView && listView->viewMode() == QListView::IconMode)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(brush);
        painter->drawRoundedRect(opt->rect, rnd, rnd);
    }
    else
    {
        if (opt->viewItemPosition == QStyleOptionViewItemV4::Beginning && !(opt->SUNKEN))
            Render::renderMask(opt->rect, painter, brush, rnd, Render::All & ~Render::Right);
        else if (opt->viewItemPosition == QStyleOptionViewItemV4::End && !(opt->SUNKEN))
            Render::renderMask(opt->rect, painter, brush, rnd, Render::All & ~Render::Left);
        else
        {
            painter->fillRect(opt->rect, brush);
            if (opt->SUNKEN && /*opt->viewItemPosition == QStyleOptionViewItemV4::OnlyOne && */!multiSelection)
            {
                painter->save();
                painter->setPen(QColor(0, 0, 0, 64));
                painter->translate(0.0f, 0.5f);
                painter->drawLine(opt->rect.topLeft(), opt->rect.topRight());
                painter->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
                painter->restore();
            }
        }
    }
    painter->restore();
    return true;
}

bool
StyleProject::drawViewItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionViewItemV4 *opt = qstyleoption_cast<const QStyleOptionViewItemV4 *>(option);
    if (!opt)
        return true;
    drawViewItemBg(option, painter, widget);
    painter->save();

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
        painter->setFont(opt->font);
        if (opt->SUNKEN)
        {
            fg = QPalette::HighlightedText;
            BOLD;
        }
        const QFontMetrics fm(painter->fontMetrics());
        const QString text(fm.elidedText(opt->text, opt->textElideMode, textRect.width()));
        drawItemText(painter, textRect, opt->displayAlignment, opt->palette, opt->ENABLED, text, fg);
    }
    painter->restore();
    return true;
}

bool
StyleProject::drawTree(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!option)
        return true;
    if (qMin(option->rect.width(), option->rect.height()) < 8)
        return true;
//    if (option->SUNKEN)

    castObj(const QAbstractItemView *, view, widget);
//    QPoint pos;
//    if (view && view->viewport())
//        pos = view->viewport()->mapFromGlobal(QCursor::pos());
//    if (!pos.isNull())
//        if (pos.y() >= option->rect.y() && pos.y() <= option->rect.bottom())
//            const_cast<QStyleOption *>(option)->state |= State_MouseOver;
    drawViewItemBg(option, painter, widget);
    painter->save();
    QPalette::ColorRole fg(option->state & State_Selected ? QPalette::HighlightedText : QPalette::Text),
            bg(option->state & State_Selected ? QPalette::Highlight : QPalette::Base);

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
        if (option->state & State_MouseOver && !(option->state & State_Selected))
            fgc = option->palette.color(QPalette::Highlight);
        else
        {

            fgc = option->palette.color(fg);
            if (!dConf.views.treelines)
                fgc = Color::mid(fgc, option->palette.color(bg));
        }
        painter->translate(bool(!(option->state & State_Open)), bool(!(option->state & State_Open))?-0.5f:0);
        Ops::drawArrow(painter, fgc, option->rect, option->state & State_Open ? Ops::Down : Ops::Right, dConf.views.treelines?7:dConf.arrowSize);
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
    lg.setStops(DSP::Settings::gradientStops(dConf.pushbtn.gradient, b));
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
StyleProject::drawHeaderLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Header, opt, option);
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
//        Ops::drawArrow(painter, opt->palette.color(fg), ar, opt->sortIndicator==QStyleOptionHeader::SortUp?Ops::Up:Ops::Down, Qt::AlignCenter, 7);
        Ops::drawArrow(painter, fg, option->palette, option->ENABLED, ar, opt->sortIndicator==QStyleOptionHeader::SortUp?Ops::Up:Ops::Down, pixelMetric(PM_HeaderMarkSize));
    }
    const QFontMetrics fm(painter->fontMetrics());
    const QString text(fm.elidedText(opt->text, Qt::ElideRight, tr.width()));
    drawItemText(painter, tr, opt->textAlignment, opt->palette, opt->state & State_Enabled, text, fg);
    painter->restore();
    return true;
}
