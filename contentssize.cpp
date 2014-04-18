#include <QMenu>
#include <QStyleOptionMenuItem>
#include <QDebug>

#include "styleproject.h"

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
    case CT_ToolButton: return contentsSize+QSize(10, 8);
    case CT_MenuBarItem: return contentsSize;
    case CT_MenuItem: return menuItemSize(qstyleoption_cast<const QStyleOptionMenuItem *>(opt), qobject_cast<const QMenu *>(widget), contentsSize);
    case CT_Menu: return contentsSize;
    default: break;
    }

    return QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
}
