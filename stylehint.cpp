#include <QTabBar>
#include <QStyleOptionTabV3>
#include <QApplication>
#include <QMainWindow>

#include "dsp.h"
#include "stylelib/ops.h"
#include "config/settings.h"
#include "defines.h"

using namespace DSP;

int
Style::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *shret) const
{
    switch (sh)
    {
    case SH_TabBar_Alignment:
    {
        const QTabBar *tabBar = qobject_cast<const QTabBar *>(w);
        const QStyleOptionTabV3 *tab = qstyleoption_cast<const QStyleOptionTabV3 *>(opt);
        if (Ops::isSafariTabBar(tabBar))
            return Qt::AlignLeft;
        if ((tab && tab->documentMode) || (tabBar && tabBar->documentMode()))
            return Qt::AlignLeft;
        return Qt::AlignCenter;
    }
//    case SH_TabBar_PreferNoArrows: return true;
    case SH_TabBar_CloseButtonPosition: return dConf.tabs.closeButtonSide;
    case SH_TabBar_ElideMode: //WHAT THE FUCK?!!?!?!?!?!?!?!?! how the fuck does qt calculate the vertical tabbars?!?!?!?!?!?!?!?!?!
        //vertical tabs elided text gets somewhy calculated from the HORIZONTAL FKN SIZE??!?!?!? REALLY?!?!?!?!?!?!?!
        if (!opt || !w)
            return Qt::ElideNone;
        return isVertical(qstyleoption_cast<const QStyleOptionTabV3 *>(opt), qobject_cast<const QTabBar *>(w))?Qt::ElideNone:Qt::ElideRight;
    case SH_ScrollBar_MiddleClickAbsolutePosition: return true;
    case SH_ScrollBar_ScrollWhenPointerLeavesControl: return true;
    case SH_ItemView_PaintAlternatingRowColorsForEmptyArea: return true;
    case SH_DockWidget_ButtonsHaveFrame: return false;
    case SH_ToolBox_SelectedPageTitleBold: return true;
    case (QStyle::StyleHint)0xff000001: if (w && w->objectName() == "CE_CapacityBar") return CE_ProgressBar; //KCapacityBar
    default: break;
    }
#if DEBUG
    qDebug() << "currently unhandled stylehint:" << sh << opt << w << shret;
#endif
    return QCommonStyle::styleHint(sh, opt, w, shret);
}

/**
enum StyleHint {
    SH_EtchDisabledText,
    SH_DitherDisabledText,
    SH_ScrollBar_MiddleClickAbsolutePosition,
    SH_ScrollBar_ScrollWhenPointerLeavesControl,
    SH_TabBar_SelectMouseType,
    SH_TabBar_Alignment,
    SH_Header_ArrowAlignment,
    SH_Slider_SnapToValue,
    SH_Slider_SloppyKeyEvents,
    SH_ProgressDialog_CenterCancelButton,
    SH_ProgressDialog_TextLabelAlignment,
    SH_PrintDialog_RightAlignButtons,
    SH_MainWindow_SpaceBelowMenuBar,
    SH_FontDialog_SelectAssociatedText,
    SH_Menu_AllowActiveAndDisabled,
    SH_Menu_SpaceActivatesItem,
    SH_Menu_SubMenuPopupDelay,
    SH_ScrollView_FrameOnlyAroundContents,
    SH_MenuBar_AltKeyNavigation,
    SH_ComboBox_ListMouseTracking,
    SH_Menu_MouseTracking,
    SH_MenuBar_MouseTracking,
    SH_ItemView_ChangeHighlightOnFocus,
    SH_Widget_ShareActivation,
    SH_Workspace_FillSpaceOnMaximize,
    SH_ComboBox_Popup,
    SH_TitleBar_NoBorder,
    SH_Slider_StopMouseOverSlider,
    SH_ScrollBar_StopMouseOverSlider = SH_Slider_StopMouseOverSlider, // obsolete
    SH_BlinkCursorWhenTextSelected,
    SH_RichText_FullWidthSelection,
    SH_Menu_Scrollable,
    SH_GroupBox_TextLabelVerticalAlignment,
    SH_GroupBox_TextLabelColor,
    SH_Menu_SloppySubMenus,
    SH_Table_GridLineColor,
    SH_LineEdit_PasswordCharacter,
    SH_DialogButtons_DefaultButton,
    SH_ToolBox_SelectedPageTitleBold,
    SH_TabBar_PreferNoArrows,
    SH_ScrollBar_LeftClickAbsolutePosition,
    SH_Q3ListViewExpand_SelectMouseType,
    SH_UnderlineShortcut,
    SH_SpinBox_AnimateButton,
    SH_SpinBox_KeyPressAutoRepeatRate,
    SH_SpinBox_ClickAutoRepeatRate,
    SH_Menu_FillScreenWithScroll,
    SH_ToolTipLabel_Opacity,
    SH_DrawMenuBarSeparator,
    SH_TitleBar_ModifyNotification,
    SH_Button_FocusPolicy,
    SH_MenuBar_DismissOnSecondClick,
    SH_MessageBox_UseBorderForButtonSpacing,
    SH_TitleBar_AutoRaise,
    SH_ToolButton_PopupDelay,
    SH_FocusFrame_Mask,
    SH_RubberBand_Mask,
    SH_WindowFrame_Mask,
    SH_SpinControls_DisableOnBounds,
    SH_Dial_BackgroundRole,
    SH_ComboBox_LayoutDirection,
    SH_ItemView_EllipsisLocation,
    SH_ItemView_ShowDecorationSelected,
    SH_ItemView_ActivateItemOnSingleClick,
    SH_ScrollBar_ContextMenu,
    SH_ScrollBar_RollBetweenButtons,
    SH_Slider_AbsoluteSetButtons,
    SH_Slider_PageSetButtons,
    SH_Menu_KeyboardSearch,
    SH_TabBar_ElideMode,
    SH_DialogButtonLayout,
    SH_ComboBox_PopupFrameStyle,
    SH_MessageBox_TextInteractionFlags,
    SH_DialogButtonBox_ButtonsHaveIcons,
    SH_SpellCheckUnderlineStyle,
    SH_MessageBox_CenterButtons,
    SH_Menu_SelectionWrap,
    SH_ItemView_MovementWithoutUpdatingSelection,
    SH_ToolTip_Mask,
    SH_FocusFrame_AboveWidget,
    SH_TextControl_FocusIndicatorTextCharFormat,
    SH_WizardStyle,
    SH_ItemView_ArrowKeysNavigateIntoChildren,
    SH_Menu_Mask,
    SH_Menu_FlashTriggeredItem,
    SH_Menu_FadeOutOnHide,
    SH_SpinBox_ClickAutoRepeatThreshold,
    SH_ItemView_PaintAlternatingRowColorsForEmptyArea,
    SH_FormLayoutWrapPolicy,
    SH_TabWidget_DefaultTabPosition,
    SH_ToolBar_Movable,
    SH_FormLayoutFieldGrowthPolicy,
    SH_FormLayoutFormAlignment,
    SH_FormLayoutLabelAlignment,
    SH_ItemView_DrawDelegateFrame,
    SH_TabBar_CloseButtonPosition,
    SH_DockWidget_ButtonsHaveFrame,
    SH_ToolButtonStyle,
    SH_RequestSoftwareInputPanel,
    // Add new style hint values here

#ifdef QT3_SUPPORT
    SH_GUIStyle = 0x00000100,
    SH_ScrollBar_BackgroundMode,
    // Add other compat values here

    SH_UnderlineAccelerator = SH_UnderlineShortcut,
#endif
    SH_CustomBase = 0xf0000000
};
*/
