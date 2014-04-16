#include <QLineEdit>
#include <QTabBar>
#include <QStyleOption>
#include <QMainWindow>

#include "styleproject.h"
#include "overlay.h"

int
StyleProject::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric)
    {
    case PM_DockWidgetSeparatorExtent: return 1;
    case PM_SplitterWidth: return 1;
    case PM_DefaultFrameWidth:
    {
        if (!widget)
            return 2;
        if (qobject_cast<const QLineEdit *>(widget))
            return 3;
        if (const QFrame *frame = qobject_cast<const QFrame *>(widget))
            if (frame->frameShadow() == QFrame::Sunken && frame->findChild<OverLay *>())
                return 0;
        return 2;
    }
    case PM_ToolBarItemSpacing: return 0;
    case PM_ToolBarSeparatorExtent: return 8;
    case PM_ToolBarFrameWidth:
    {
        if (castObj(const QToolBar *, toolBar, widget))
            if (castObj(QMainWindow *, win, toolBar->parentWidget()))
//                if (win->toolBarArea(toolBar) == Qt::TopToolBarArea)
                {
                    QPoint topLeft = toolBar->mapTo(win, toolBar->rect().topLeft());
                    QRect winRect = win->rect();
                    QRect widgetRect = QRect(topLeft, toolBar->size());
                    if (winRect.top() <= widgetRect.top())
                        return 2;
                }
        return 4;
    }
    case PM_MenuBarItemSpacing: return 4;
//    case PM_SliderThickness: return 12;
    case PM_SliderLength:
    case PM_SliderControlThickness: return qMin(option->rect.height(), option->rect.width());
//    case PM_SliderSpaceAvailable:
    case PM_ToolTipLabelFrameWidth:
        return 8;
    default: break;
    }
    return QCommonStyle::pixelMetric(metric, option, widget);
}

/** enum PixelMetric {
    PM_ButtonMargin,
    PM_ButtonDefaultIndicator,
    PM_MenuButtonIndicator,
    PM_ButtonShiftHorizontal,
    PM_ButtonShiftVertical,

    PM_DefaultFrameWidth,
    PM_SpinBoxFrameWidth,
    PM_ComboBoxFrameWidth,

    PM_MaximumDragDistance,

    PM_ScrollBarExtent,
    PM_ScrollBarSliderMin,

    PM_SliderThickness,             // total slider thickness
    PM_SliderControlThickness,      // thickness of the business part
    PM_SliderLength,                // total length of slider
    PM_SliderTickmarkOffset,        //
    PM_SliderSpaceAvailable,        // available space for slider to move

    PM_DockWidgetSeparatorExtent,
    PM_DockWidgetHandleExtent,
    PM_DockWidgetFrameWidth,

    PM_TabBarTabOverlap,
    PM_TabBarTabHSpace,
    PM_TabBarTabVSpace,
    PM_TabBarBaseHeight,
    PM_TabBarBaseOverlap,

    PM_ProgressBarChunkWidth,

    PM_SplitterWidth,
    PM_TitleBarHeight,

    PM_MenuScrollerHeight,
    PM_MenuHMargin,
    PM_MenuVMargin,
    PM_MenuPanelWidth,
    PM_MenuTearoffHeight,
    PM_MenuDesktopFrameWidth,

    PM_MenuBarPanelWidth,
    PM_MenuBarItemSpacing,
    PM_MenuBarVMargin,
    PM_MenuBarHMargin,

    PM_IndicatorWidth,
    PM_IndicatorHeight,
    PM_ExclusiveIndicatorWidth,
    PM_ExclusiveIndicatorHeight,
    PM_CheckListButtonSize,
    PM_CheckListControllerSize,

    PM_DialogButtonsSeparator,
    PM_DialogButtonsButtonWidth,
    PM_DialogButtonsButtonHeight,

    PM_MdiSubWindowFrameWidth,
    PM_MDIFrameWidth = PM_MdiSubWindowFrameWidth,            //obsolete
    PM_MdiSubWindowMinimizedWidth,
    PM_MDIMinimizedWidth = PM_MdiSubWindowMinimizedWidth,    //obsolete

    PM_HeaderMargin,
    PM_HeaderMarkSize,
    PM_HeaderGripMargin,
    PM_TabBarTabShiftHorizontal,
    PM_TabBarTabShiftVertical,
    PM_TabBarScrollButtonWidth,

    PM_ToolBarFrameWidth,
    PM_ToolBarHandleExtent,
    PM_ToolBarItemSpacing,
    PM_ToolBarItemMargin,
    PM_ToolBarSeparatorExtent,
    PM_ToolBarExtensionExtent,

    PM_SpinBoxSliderHeight,

    PM_DefaultTopLevelMargin,
    PM_DefaultChildMargin,
    PM_DefaultLayoutSpacing,

    PM_ToolBarIconSize,
    PM_ListViewIconSize,
    PM_IconViewIconSize,
    PM_SmallIconSize,
    PM_LargeIconSize,

    PM_FocusFrameVMargin,
    PM_FocusFrameHMargin,

    PM_ToolTipLabelFrameWidth,
    PM_CheckBoxLabelSpacing,
    PM_TabBarIconSize,
    PM_SizeGripSize,
    PM_DockWidgetTitleMargin,
    PM_MessageBoxIconSize,
    PM_ButtonIconSize,

    PM_DockWidgetTitleBarButtonMargin,

    PM_RadioButtonLabelSpacing,
    PM_LayoutLeftMargin,
    PM_LayoutTopMargin,
    PM_LayoutRightMargin,
    PM_LayoutBottomMargin,
    PM_LayoutHorizontalSpacing,
    PM_LayoutVerticalSpacing,
    PM_TabBar_ScrollButtonOverlap,

    PM_TextCursorWidth,

    PM_TabCloseIndicatorWidth,
    PM_TabCloseIndicatorHeight,

    PM_ScrollView_ScrollBarSpacing,
    PM_SubMenuOverlap,

    // do not add any values below/greater than this
    PM_CustomBase = 0xf0000000
};*/

//QStyle::PM_SliderThickness	11	Total slider thickness.
//QStyle::PM_SliderControlThickness	12	Thickness of the slider handle.
//QStyle::PM_SliderLength	13	Length of the slider.
//QStyle::PM_SliderTickmarkOffset	14	The offset between the tickmarks and the slider.
//QStyle::PM_SliderSpaceAvailable
