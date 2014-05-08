#include <QMenu>
#include <QStyleOptionMenuItem>
#include <QDebug>
#include <QToolBar>
#include <QStyleOptionToolButton>
#include <QToolButton>
#include <QMainWindow>

#include "styleproject.h"
#include "stylelib/ops.h"
#include "stylelib/render.h"

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

static QSize menuItemSize(const QStyleOptionMenuItem *item, const QMenu *menu, QSize cs)
{
    if (!item)
        return cs;

    cs += QSize((item->menuHasCheckableItems?32:6)+32, 0); //just to add some decent width to the menu
    const bool isSeparator(item->menuItemType == QStyleOptionMenuItem::Separator);
    const bool hasText(!item->text.isEmpty());
    if (isSeparator)
    {
        if (hasText)
            cs += QSize(0, item->fontMetrics.height());
        else
            cs += QSize(0, 2);
    }

    if (hasText)
        cs += QSize(0, 4);

    return cs;
}

QSize
StyleProject::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *widget) const
{
    switch (ct)
    {
    case CT_TabBarTab:
    {
        castObj(const QTabBar *, bar, widget);
        if (!Ops::isSafariTabBar(bar))
            break;

        QSize sz = QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
        int margin(pixelMetric(PM_TabBarBaseOverlap)+pixelMetric(PM_TabBarTabHSpace));
        castObj(const QTabWidget *, tw, bar->parentWidget());
        if (tw)
        for (int i = Qt::TopLeftCorner; i <= Qt::TopRightCorner; ++i)
            if (QWidget *cw = tw->cornerWidget(Qt::Corner(i)))
                margin+=cw->width();
        if (bar->expanding())
            sz.setWidth(qMax((bar->width()-margin)/bar->count(), sz.width()));
        return sz;
    }
    case CT_ToolButton:
    {
        if (widget && !widget->parentWidget())
            return contentsSize;

        castOpt(ToolButton, optbtn, opt);
        if (!optbtn)
            return contentsSize;

        const QRect geo = widget->geometry();
        int x, y, r, b, h = widget->height(), hc = y+h/2, w = widget->width(), wc = x+w/2;
        int margin = pixelMetric(PM_ToolBarSeparatorExtent, opt, widget);
        geo.getCoords(&x, &y, &r, &b);
        castObj(const QToolBar *, bar, widget->parentWidget());
        bool isFull(bar && !(qobject_cast<const QToolButton *>(bar->childAt(r+margin, hc))
                    || qobject_cast<const QToolButton *>(bar->childAt(x-margin, hc))
                    || qobject_cast<const QToolButton *>(bar->childAt(wc, b+margin))
                    || qobject_cast<const QToolButton *>(bar->childAt(wc, y-margin))));

        QSize sz(contentsSize);
        bool hor(bar ? bar->orientation() == Qt::Horizontal : true);
        sz+=QSize(hor?8:4, hor?4:8);
        if (hor && isFull)
            sz.rwidth() += 8;
        if (sz.height() < 23)
            sz.setHeight(23);
        return sz;
    }
    case CT_LineEdit:
    {
        QSize sz(contentsSize);
        sz+=QSize(8, pixelMetric(PM_DefaultFrameWidth, opt, widget));
        if (sz.height() < 23)
            sz.setHeight(23);
        return sz;
    }
    case CT_MenuBarItem: return contentsSize;
    case CT_MenuItem: return menuItemSize(qstyleoption_cast<const QStyleOptionMenuItem *>(opt), qobject_cast<const QMenu *>(widget), contentsSize);
    case CT_Menu: return contentsSize;
    case CT_RadioButton:
    case CT_CheckBox:
    {
        castOpt(Button, btn, opt);
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
        castOpt(ComboBox, box, opt);
        if (!box)
            return contentsSize;
        int w(contentsSize.width()), h(contentsSize.height());

        if (box->frame)
        {
            w+=pixelMetric(PM_ComboBoxFrameWidth, opt, widget)*2;
            h+=pixelMetric(PM_ComboBoxFrameWidth, opt, widget)*2;
        }

        w+=12; //margins left/right for text...
        w+=h;
        return QSize(w, h);
    }
    default: break;
    }
    return QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
}
