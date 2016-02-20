#include <QMenu>
#include <QStyleOptionMenuItem>
#include <QDebug>
#include <QToolBar>
#include <QStyleOptionToolButton>
#include <QStyleOptionButton>
#include <QToolButton>
#include <QMainWindow>
#include <QMenuBar>
#include <QLineEdit>
#include <QApplication>
#include <QSpinBox>
#include <QAbstractItemView>
#include <QProgressBar>

#include "dsp.h"
#include "stylelib/ops.h"
#include "stylelib/gfx.h"
#include "config/settings.h"
#include "stylelib/handlers.h"

/** enum ContentsType {
    CT_PushButton,
    CT_CheckBox,
    CT_RadioButton,
    CT_ToolButton,
    CT_ComboBox,
    CT_Splitter,
    CT_Q3DockWindow,
    CT_ProgressBar,
    CT_MenuItem,
    CT_MenuBarItem,
    CT_MenuBar,
    CT_Menu,
    CT_TabBarTab,
    CT_Slider,
    CT_ScrollBar,
    CT_Q3Header,
    CT_LineEdit,
    CT_SpinBox,
    CT_SizeGrip,
    CT_TabWidget,
    CT_DialogButtons,
    CT_HeaderSection,
    CT_GroupBox,
    CT_MdiControls,
    CT_ItemViewItem,
    // do not add any values below/greater than this
    CT_CustomBase = 0xf0000000
}; */

using namespace DSP;

static QSize menuItemSize(const QStyleOptionMenuItem *item, const QMenu *menu, QSize cs)
{
    if (!item)
        return cs;

    if (menu)
    {
        const QList<QAction *> actions = menu->actions();
        for (int i = 0; i < actions.size(); ++i)
        {
            const QAction *a(actions.at(i));
            if (a->text() == item->text && a->font().bold())
                cs.rwidth() += (QFontMetrics(a->font()).width(item->text) - QFontMetrics(item->font).width(item->text));
        }
    }

    cs += QSize((item->menuHasCheckableItems?32:6)+32, 0); //just to add some decent width to the menu
    if (dConf.menues.icons)
        cs.rwidth()+= 20;
    const bool isSeparator(item->menuItemType == QStyleOptionMenuItem::Separator);
    const bool hasText(!item->text.isEmpty());
    if (isSeparator && !hasText)
        cs += QSize(0, 2);
    else if (isSeparator)
        cs.rheight() += item->fontMetrics.height();
    else
        cs += QSize(0, 6);

    return cs;
}

QSize
Style::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *widget) const
{
    switch (ct)
    {
    case CT_HeaderSection:
    {
        QSize sz(contentsSize);
        if (sz.height() < opt->fontMetrics.height())
            sz.rheight() = opt->fontMetrics.height();
        sz.rheight()+=4;
        return sz;
    }
    case CT_PushButton:
    {
        QSize sz(contentsSize);
        sz+=QSize(8, pixelMetric(PM_ButtonMargin, opt, widget));
        if (sz.height() < dConf.baseSize)
            sz.setHeight(dConf.baseSize);
        const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt);
        if (btn && !btn->text.isEmpty())
            if (sz.width() < 75)
                sz.setWidth(75);
        return sz;
    }
    case CT_TabBarTab:
    {
        const QStyleOptionTabV3 *tab = qstyleoption_cast<const QStyleOptionTabV3 *>(opt);
        if (!tab)
            break;

        const QTabBar *bar = qobject_cast<const QTabBar *>(widget);
        const bool safBar = Ops::isSafariTabBar(bar);
        const bool vertical = isVertical(tab, bar);
        QSize sz(contentsSize);
        const quint8 bs = dConf.baseSize + (!safBar && (tab->documentMode || dConf.tabs.regular)) * TabBarBottomSize;
        if (sz.height() < bs)
            sz.setHeight(bs);
        if (sz.width() < bs)
            sz.setWidth(bs);
        if (safBar && bar->expanding())
        {
            int w(bar->width());
            if (const QTabWidget *tw = qobject_cast<const QTabWidget *>(bar->parentWidget()))
            {
                for (int i = Qt::TopLeftCorner; i <= Qt::TopRightCorner; ++i)
                    if (QWidget *cw = tw->cornerWidget(Qt::Corner(i)))
                        w-=cw->width();
            }
            else
            {
                QList<const QWidget *> children = bar->findChildren<const QWidget *>();
                for (int i = 0; i < bar->count(); ++i)
                    children.removeOne(bar->tabButton(i, (QTabBar::ButtonPosition)styleHint(SH_TabBar_CloseButtonPosition, opt, widget)));

                for (int i = 0; i < children.count(); ++i)
                {
                    const QWidget *wi(children.at(i));
                    if (wi->isVisible() && wi->parentWidget() == bar)
                        w-=wi->width();
                }
            }
            sz.setWidth(bar->count() == 1 ? w : (w/bar->count())-1);
            return sz;
        }

        if (vertical)
            sz.rheight() += safBar ? (dConf.tabs.safrnd<<1) : TabPadding;
        else
            sz.rwidth() += safBar ? (dConf.tabs.safrnd<<1) : TabPadding;

        if (!safBar && tab->documentMode)
        {
            if (tab->position != QStyleOptionTabV3::Middle)
            {
                sz.rwidth() += !vertical * TabDocModePadding;
                sz.rheight() += vertical * TabDocModePadding;
            }
            if (bar && bar->expanding())
            {
                static const int docTabSize = 200;
                if (vertical)
                    sz.setHeight(docTabSize);
                else
                    sz.setWidth(docTabSize);
            }
        }
        return sz;
    }
    case CT_ToolButton:
    {
        if (!widget || (widget && !widget->parentWidget()))
            return contentsSize;

        const QStyleOptionToolButton *optbtn = qstyleoption_cast<const QStyleOptionToolButton *>(opt);
        if (!optbtn)
            return contentsSize;

        const QToolButton *btn = qobject_cast<const QToolButton *>(widget);
        QToolBar *bar = qobject_cast<QToolBar *>(widget->parentWidget());

        QSize sz(contentsSize);
        bool hor(bar ? bar->orientation() == Qt::Horizontal : true);
        if (!hor)
            sz.transpose();
        if (!hor && bar && bar->toolButtonStyle() == Qt::ToolButtonTextUnderIcon)
        {
            sz.setWidth(bar->iconSize().width());
            sz.rheight()+=16;
        }
        static const int add(qFloor(dConf.baseSize*0.4f)), hAdd(qFloor(add*0.5f));
        sz+=QSize(hor?add:hAdd, hor?hAdd:add);

        if (!dConf.toolbtn.flat)
        {
            Sides sides = All;
            if (bar)
            {
                if (Handlers::ToolBar::isDirty(bar))
                    Handlers::ToolBar::queryToolBarLater(bar);
                sides = Handlers::ToolBar::sides(btn);
            }
            const bool isFull(sides == All);

            int *hvsz = hor?&sz.rwidth():&sz.rheight();
            int ends = hor?(Left|Right):(Top|Bottom);

            if (bar && optbtn->toolButtonStyle == Qt::ToolButtonIconOnly)
            {
                static quint8 m = qMax<quint8>(2, GFX::shadowMargin(dConf.toolbtn.shadow));
                if (isFull)
                    *hvsz += qCeil(dConf.baseSize*0.66f);
                else if (sides & ends)
                    *hvsz += m;
                if (btn && btn->group())
                    *hvsz += 8;
                if (btn && btn->isCheckable() && !isFull)
                    *hvsz += 2;
            }
            static const int minSz(dConf.baseSize);
            if (hor && sz.height() < minSz)
                sz.setHeight(minSz);
            else if (!hor && sz.width() < minSz)
                sz.setWidth(minSz);

            if ((optbtn->features & QStyleOptionToolButton::MenuButtonPopup) || ((optbtn->features & QStyleOptionToolButton::Arrow) && optbtn->arrowType && optbtn->toolButtonStyle != Qt::ToolButtonIconOnly))
                *hvsz+=16;
        }
        return sz;
    }
    case CT_LineEdit:
    {
        QSize sz(contentsSize);
        sz+=QSize(GFX::shadowMargin(dConf.input.shadow)*2, pixelMetric(PM_DefaultFrameWidth, opt, widget));
        if (sz.height() < dConf.baseSize)
            sz.setHeight(dConf.baseSize);
        return sz;
    }
    case CT_SpinBox:
    {
        const QStyleOptionSpinBox *box = qstyleoption_cast<const QStyleOptionSpinBox *>(opt);
//        const QSpinBox *spinBox = qobject_cast<const QSpinBox *>(widget);
        QSize sz(contentsSize);
        sz.rwidth()+=7;
        sz.rwidth()+=(GFX::shadowMargin(dConf.input.shadow)*2)+pixelMetric(PM_SpinBoxSliderHeight, opt, widget);
        sz.setHeight(qMax<int>(dConf.baseSize, sz.height()));
        if (box && box->frame)
            sz.rwidth()+=pixelMetric(PM_SpinBoxFrameWidth)*2;
        return sz;
    }
    case CT_MenuBarItem:
    {
        const QStyleOptionMenuItem *item = qstyleoption_cast<const QStyleOptionMenuItem *>(opt);
        if (!item)
            return contentsSize;

        QSize sz(contentsSize+QSize(8, 0));
        if (widget)
        {
            const QList<QAction *> actions = widget->actions();
            for (int i = 0; i < actions.size(); ++i)
            {
                const QAction *a(actions.at(i));
                if (a->text() == item->text && a->font().bold())
                {
                    /*
                     * Here we need to get the difference between
                     * the unbold font and the bold one, and this
                     * /has/ to be widget->font() cause if css is
                     * used its the /only/ one that has the font
                     * set in the styleshit.
                     */
                    QFont wf(widget->font());
                    const int ww(QFontMetrics(wf).width(item->text));
                    wf.setBold(true);
                    const int bw(QFontMetrics(wf).width(item->text));
                    sz.rwidth() += bw - ww;
                }
            }
        }
        return sz;
    }
    case CT_MenuItem: return menuItemSize(qstyleoption_cast<const QStyleOptionMenuItem *>(opt), qobject_cast<const QMenu *>(widget), contentsSize);
    case CT_Menu: return contentsSize;
    case CT_RadioButton:
    case CT_CheckBox:
    {
        const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt);
        if (!btn)
            return contentsSize;
        int w(contentsSize.width());
        int h(contentsSize.height());

        if (ct==CT_CheckBox)
        {
            w+=pixelMetric(PM_IndicatorWidth, opt, widget);
            w+=pixelMetric(PM_CheckBoxLabelSpacing, opt, widget);
            h=qMax(h, pixelMetric(PM_IndicatorHeight, opt, widget));
        }
        else
        {
            w+=pixelMetric(PM_ExclusiveIndicatorWidth, opt, widget);
            w+=pixelMetric(PM_RadioButtonLabelSpacing, opt, widget);
            h=qMax(h, pixelMetric(PM_ExclusiveIndicatorHeight, opt, widget));
        }
        return QSize(w, h);
    }
    case CT_ComboBox:
    {
        const QStyleOptionComboBox *box = qstyleoption_cast<const QStyleOptionComboBox *>(opt);
        if (!box)
            return contentsSize+QSize(16, 0);
        if (box->editable)
        {
            QSize sz(contentsSize);
            int add(dConf.input.rnd?dConf.input.rnd/2:0);
            sz+=QSize(add*2+40, pixelMetric(PM_ComboBoxFrameWidth, opt, widget));
            if (sz.height() < dConf.baseSize)
                sz.setHeight(dConf.baseSize);
            return sz;
        }

        int w(contentsSize.width()), h(contentsSize.height());

        if (box->frame)
        {
            w+=pixelMetric(PM_ComboBoxFrameWidth, opt, widget)*2;
            h+=pixelMetric(PM_ComboBoxFrameWidth, opt, widget)*2;
        }

        w+=12; //margins left/right for text...
        w+=h;
        h = qMax<int>(dConf.baseSize, h);
        return QSize(w, h);
    }
    case CT_ScrollBar:
    {
        return contentsSize;
    }
    case CT_ItemViewItem:
    {
        QSize sz(QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget));
        static const int sm = GFX::shadowMargin(dConf.views.itemShadow)*2 + dConf.views.itemRnd;
        sz.rheight() += sm;
#if 0
        const QStyleOptionViewItemV4 *item = qstyleoption_cast<const QStyleOptionViewItemV4 *>(opt);
        if (item && !item->text.isEmpty())
        {
            static const int m(2);
            if (item->decorationPosition < QStyleOptionViewItem::Top) //horizontal item layout
            {
                sz.setHeight(item->icon.isNull()?item->fontMetrics.height():item->decorationSize.height() + m);
                sz.setWidth(item->decorationSize.width() + m + item->fontMetrics.width(item->text));
            }
            else
            {
                sz.setHeight(item->decorationSize.height() + item->fontMetrics.height() + m);
                sz.setWidth(qMax(item->decorationSize.height(), item->fontMetrics.height())+ m);
            }
        }
        else
        {
            sz.setHeight(qMax(16, sz.height()));
            sz.setWidth(qMax(128, sz.width()));
        }
#endif
        return sz;
    }
    case CT_ProgressBar:
    {
        QSize sz(contentsSize);
        const QStyleOptionProgressBarV2 *bar = qstyleoption_cast<const QStyleOptionProgressBarV2 *>(opt);
        const bool hor(!bar || bar->orientation==Qt::Horizontal);
        if (bar && dConf.progressbars.textPos == 1)
        {
            const quint16 add = bar->fontMetrics.width(bar->text);
            if (hor)
                sz.rwidth() += add;
            else
                sz.rheight() += add;
            return sz;
        }
        if (hor && sz.height() > dConf.baseSize)
            sz.setHeight(dConf.baseSize);
        else if (!hor && sz.width() > dConf.baseSize)
            sz.setWidth(dConf.baseSize);

        if (hor)
            sz.rwidth() += dConf.progressbars.rnd;
        else
            sz.rheight() += dConf.progressbars.rnd;
        return sz;
    }
    default: break;
    }
    return QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
}
