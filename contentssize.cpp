#include <QMenu>
#include <QStyleOptionMenuItem>
#include <QDebug>
#include <QToolBar>
#include <QStyleOptionToolButton>
#include <QStyleOptionButton>
#include <QToolButton>
#include <QMainWindow>
#include <QMenuBar>

#include "styleproject.h"
#include "stylelib/ops.h"
#include "stylelib/render.h"
#include "stylelib/settings.h"

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
    if (Settings::conf.menues.icons)
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
StyleProject::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *widget) const
{
    switch (ct)
    {
    case CT_HeaderSection:
    {
        QSize sz(contentsSize);
        if (sz.height() < opt->fontMetrics.height())
            sz.rheight() = opt->fontMetrics.height();
        sz.rheight()+=2;
        return sz;
    }
    case CT_PushButton:
    {
        QSize sz(contentsSize);
        sz+=QSize(8, pixelMetric(PM_ButtonMargin, opt, widget));
        if (sz.height() < 23)
            sz.setHeight(23);
        castOpt(Button, btn, opt);
        if (btn && !btn->text.isEmpty())
            if (sz.width() < 75)
                sz.setWidth(75);
        return sz;
    }
    case CT_TabBarTab:
    {
        castOpt(TabV3, tab, opt);

        if (!tab)
            break;

        castObj(const QTabBar *, bar, widget);
        QSize sz(contentsSize);
        if (Ops::isSafariTabBar(bar))
        {
            if (bar->expanding())
            {
                int w(bar->width());
                if (castObj(const QTabWidget *, tw, bar->parentWidget()))
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
                sz.setWidth(w/bar->count());
            }
            else if (tab->position == QStyleOptionTab::Beginning || tab->position == QStyleOptionTab::OnlyOneTab)
                sz.rwidth() += pixelMetric(PM_TabBarTabOverlap, opt, widget);
        }
        if (bar && !bar->expanding())
        {
            const QString s(tab->text);
            QFont f(bar->font());
            int nonBoldWidth(QFontMetrics(f).boundingRect(s).width());
            f.setBold(true);
            int boldWidth(QFontMetrics(f).boundingRect(s).width());
            int add(boldWidth-nonBoldWidth);
//            add+=4; //some extra padding for good measure...
            if (!isVertical(tab, bar))
                sz.rwidth() += add;
            else
                sz.rheight() += add;
        }
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
        castObj(const QToolButton *, btn, widget);
        castObj(const QToolBar *, bar, widget->parentWidget());
        Render::Sides sides = Render::All;
        if (bar&&qobject_cast<const QToolButton *>(bar->childAt(r+margin, hc)))
            sides &= ~Render::Right;
        if (bar&&qobject_cast<const QToolButton *>(bar->childAt(x-margin, hc)))
            sides &= ~Render::Left;
//        if (qobject_cast<const QToolButton *>(bar->childAt(wc, b+margin)))
//            sides &= ~Render::Bottom;
//        if (qobject_cast<const QToolButton *>(bar->childAt(wc, y-margin)))
//            sides &= ~Render::Top;

        const bool isFull(sides == Render::All);

        QSize sz(contentsSize);
        bool hor(bar ? bar->orientation() == Qt::Horizontal : true);
        sz+=QSize(hor?8:4, hor?4:8);
        if (bar && hor && optbtn->toolButtonStyle == Qt::ToolButtonIconOnly)
        {
            if (isFull)
                sz.rwidth() += 16;
            else if (sides & (Render::Left|Render::Right))
                sz.rwidth() += (2+(Settings::conf.toolbtn.shadow==Render::Carved)*2);
            if (btn && btn->group())
                sz.rwidth() += 8;
            if (btn && btn->isCheckable() && !isFull)
                sz.rwidth() += 2;
        }
        if (sz.height() < 23)
            sz.setHeight(23);

        if (Ops::hasMenu(btn, optbtn))
            sz.rwidth()+=16;
        return sz;
    }
    case CT_LineEdit:
    {
//        if (widget && widget->objectName() == "qt_spinbox_lineedit")
//            break;

        QSize sz(contentsSize);
//        int add(Settings::conf.input.rnd/2);
//        switch (Settings::conf.input.shadow)
//        {
//        case Render::Sunken:
//        case Render::Etched:
//        case Render::Raised:
//            add+=3;
//            break;
//        case Render::Simple:
//            break;
//        case Render::Carved:
//            add+=6;
//            break;
//        default: break;
//        }
//        sz+=QSize(add, add);
        sz+=QSize(8, pixelMetric(PM_DefaultFrameWidth, opt, widget));
        if (sz.height() < 23)
            sz.setHeight(23);
        return sz;
    }
//        case CT_MenuItem:
    case CT_MenuBarItem:
    {
        castOpt(MenuItem, item, opt);
        if (!item)
            return contentsSize;

        QSize sz(contentsSize+QSize(8, 0));
        if (castObj(const QMenuBar *, bar, widget))
        {
            const QList<QAction *> actions = bar->actions();
            for (int i = 0; i < actions.size(); ++i)
            {
                const QAction *a(actions.at(i));
                if (a->text() == item->text && a->font().bold())
                    sz.rwidth() += (QFontMetrics(a->font()).width(item->text) - QFontMetrics(item->font).width(item->text));
            }
        }
        return sz;
    }
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
        if (box->editable)
        {
            QSize sz(contentsSize);
            sz+=QSize(16, pixelMetric(PM_ComboBoxFrameWidth, opt, widget));
            if (sz.height() < 23)
                sz.setHeight(23);
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
        h = qMax(23, h);
        return QSize(w, h);
    }
    default: break;
    }
    return QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
}
