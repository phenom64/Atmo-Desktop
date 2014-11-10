#include <QLineEdit>
#include <QTabBar>
#include <QStyleOption>
#include <QMainWindow>
#include <QComboBox>
#include <QAbstractButton>
#include <QLayout>
#include <QDockWidget>
#include <Q3GroupBox>

#include "styleproject.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/settings.h"

int
StyleProject::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric)
    {
    case PM_HeaderMarkSize:
        return 9;
    case PM_ButtonMargin:
        return 2;
    case PM_SpinBoxFrameWidth:
        return 0;
    case PM_TabCloseIndicatorHeight:
    case PM_TabCloseIndicatorWidth:
        return 16;
    case PM_TabBarTabOverlap:	//19	Number of pixels the tabs should overlap. (Currently only used in styles, not inside of QTabBar)
        return Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget))?Settings::conf.tabs.safrnd+4:0;
    case PM_TabBarTabHSpace:	//20	Extra space added to the tab width.
//        if (Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget)))
//        {
//            castOpt(TabV3, tab, option);
//            if (tab && tab->position == QStyleOptionTab::Beginning)
//                return 128;
//        }
        return 16;
    case PM_TabBarTabVSpace:	//21	Extra space added to the tab height.
        return Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget))?8:4;
//    case PM_TabBarBaseHeight:	//22	Height of the area between the tab bar and the tab pages.
//    case PM_TabBarBaseOverlap:	//23	Number of pixels the tab bar overlaps the tab bar base.
//    case PM_TabBarScrollButtonWidth:	//?
    case PM_TabBarTabShiftHorizontal:	//?	Horizontal pixel shift w hen a tab is selected.
    case PM_TabBarTabShiftVertical:
        return 0;
    case PM_IndicatorHeight:
    case PM_IndicatorWidth:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        return 16;
    case PM_CheckBoxLabelSpacing:
    case PM_RadioButtonLabelSpacing:
        return 4;
    case PM_MenuVMargin: return 6;
    case PM_MenuHMargin: return 0;
    case PM_MenuBarPanelWidth:
    case PM_MenuPanelWidth: return 0;
    case PM_MenuBarItemSpacing: return 8;
    case PM_DockWidgetSeparatorExtent: return 1;
    case PM_SplitterWidth:
    {
        if (!widget)
            return 1;
        const QWidget *parent = widget->parentWidget();
        if (!parent)
            return 1;

        const QMargins m(parent->layout()?parent->layout()->contentsMargins():parent->contentsMargins());
        if (m.left() || m.right() || m.top() || m.bottom())
            return 8;
        return 1;
    }
    case PM_DockWidgetTitleBarButtonMargin: return 0;
    case PM_DefaultFrameWidth:
    {
        if (!widget)
            return 2;
//        if (qobject_cast<const QLineEdit *>(widget)||qobject_cast<const QComboBox *>(widget))
//            return 2;
        const QFrame *frame = qobject_cast<const QFrame *>(widget);
        if (OverLay::hasOverLay(frame))
            return 0;
        if (qobject_cast<const QGroupBox *>(widget))
            return 4;

//        if (!qobject_cast<const QAbstractButton *>(frame) && !qstyleoption_cast<const QStyleOptionButton *>(option))
        if (frame && frame->frameShadow() == QFrame::Raised)
            return 8;
        if (option && option->state & State_Raised) //the buttons in qtcreator....
            return 0;
        return 2;
    }
    case PM_ToolBarItemSpacing: return 0;
    case PM_ToolBarSeparatorExtent: return 8;
    case PM_ToolBarFrameWidth: return 2;
    case PM_ToolBarHandleExtent: return 8;
//    case PM_SliderThickness: return 12;
    case PM_ScrollBarExtent: return Settings::conf.scrollers.size;
    case PM_SliderThickness:
    case PM_SliderLength:
    case PM_SliderControlThickness: return Settings::conf.sliders.size/*qMin(option->rect.height(), option->rect.width())*/;
    case PM_SliderTickmarkOffset: return 8;
    case PM_ScrollBarSliderMin: return 32;
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

//case PM_SliderThickness	11	Total slider thickness.
//case PM_SliderControlThickness	12	Thickness of the slider handle.
//case PM_SliderLength	13	Length of the slider.
//case PM_SliderTickmarkOffset	14	The offset between the tickmarks and the slider.
//case PM_SliderSpaceAvailable
