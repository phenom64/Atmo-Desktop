#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QFileInfo>

#include "styleproject.h"
#include "stylelib/ops.h"
#include "stylelib/render.h"
#include "stylelib/handlers.h"
#include "stylelib/settings.h"

void
StyleProject::init()
{
    for (int i = 0; i < CCSize; ++i)
    {
        m_cc[i] = 0;
        m_sc[i] = 0;
    }
    for (int i = 0; i < CESize; ++i)
        m_ce[i] = 0;
    for (int i = 0; i < PESize; ++i)
        m_pe[i] = 0;
    for (int i = 0; i < EVSize; ++i)
        m_ev[i] = 0;
}

/* here we assign functions to perform the
 * drawing of the elements/widgets/controls.
 *
 * we will add more as we go
 */

#define method(_method_) &StyleProject::_method_

void
StyleProject::assignMethods()
{
    /* control elements */
    m_ce[CE_PushButton] = method(drawPushButton);
    m_ce[CE_PushButtonBevel] = method(drawPushButtonBevel);
    m_ce[CE_PushButtonLabel] = method(drawPushButtonLabel);
    m_ce[CE_CheckBox] = method(drawCheckBox);
    m_ce[CE_CheckBoxLabel] = method(drawCheckBoxLabel);
    m_ce[CE_RadioButton] = method(drawRadioButton);
    m_ce[CE_RadioButtonLabel] = method(drawRadioButtonLabel);
    m_ce[CE_ToolButtonLabel] = method(drawToolButtonLabel);
    m_ce[CE_ToolBar] = method(drawToolBar);
    m_ce[CE_Splitter] = method(drawSplitter);
    m_ce[CE_SizeGrip] = method(controlSkipper);
    m_ce[CE_MenuBarItem] = method(drawMenuItem);
    m_ce[CE_MenuItem] = method(drawMenuItem);
    m_ce[CE_TabBarTab] = method(drawTab);
    m_ce[CE_TabBarTabShape] = method(drawTabShape);
    m_ce[CE_TabBarTabLabel] = method(drawTabLabel);
    m_ce[CE_MenuBarEmptyArea] = method(drawMenuBar);
    m_ce[CE_ItemViewItem] = method(drawViewItem); //draw item in views
    m_ce[CE_ComboBoxLabel] = method(drawComboBoxLabel);
    m_ce[CE_ProgressBar] = method(drawProgressBar);
    m_ce[CE_ProgressBarGroove] = method(drawProgressBarGroove);
    m_ce[CE_ProgressBarContents] = method(drawProgressBarContents);
    m_ce[CE_ProgressBarLabel] = method(drawProgressBarLabel);
    m_ce[CE_Header] = method(drawHeader);
    m_ce[CE_HeaderSection] = method(drawHeaderSection);
    m_ce[CE_HeaderLabel] = method(drawHeaderLabel);
    m_ce[CE_DockWidgetTitle] = method(drawDockTitle);
    m_ce[CE_ToolBoxTab] = method(drawToolBoxTab);
    m_ce[CE_ToolBoxTabShape] = method(drawToolBoxTabShape);
    m_ce[CE_ToolBoxTabLabel] = method(drawToolBoxTabLabel);
    m_ce[CE_ShapedFrame] = method(drawFrame);

    /* complex controls */
    m_cc[CC_ToolButton] = method(drawToolButton);
    m_cc[CC_SpinBox] = method(drawSpinBox);
    m_cc[CC_ScrollBar] = method(drawScrollBar);
    m_cc[CC_Slider] = method(drawSlider);
    m_cc[CC_ComboBox] = method(drawComboBox);
    m_cc[CC_GroupBox] = method(drawGroupBox);
//    m_cc[CC_MdiControls] = method(complexSkipper);

    /* primitive elements */
    m_pe[PE_FrameStatusBarItem] = method(primitiveSkipper);
    m_pe[PE_IndicatorToolBarSeparator] = method(primitiveSkipper);
    m_pe[PE_PanelLineEdit] = method(drawLineEdit);
    m_pe[PE_Frame] = method(drawFrame);
    m_pe[PE_PanelMenuBar] = method(drawMenuBar);
    m_pe[PE_IndicatorDockWidgetResizeHandle] = method(drawSplitter);
    m_pe[PE_PanelStatusBar] = method(drawStatusBar);
    m_pe[PE_Widget] = method(primitiveSkipper);
    m_pe[PE_FrameWindow] = method(drawWindow);
    m_pe[PE_IndicatorToolBarHandle] = method(primitiveSkipper);
    m_pe[PE_FrameTabBarBase] = method(drawTabBar);
    m_pe[PE_FrameTabWidget] = method(drawTabWidget);
    m_pe[PE_IndicatorTabClose] = method(drawTabCloser);
    m_pe[PE_PanelItemViewItem] = method(drawViewItemBg); //draw BACKGROUND of item in views...
    m_pe[PE_PanelMenu] = method(drawMenu);
    m_pe[PE_FrameMenu] = method(drawMenu);
    m_pe[PE_PanelScrollAreaCorner] = method(drawScrollAreaCorner);
    m_pe[PE_PanelTipLabel] = method(drawToolTip);
    m_pe[PE_IndicatorBranch] = method(drawTree);
    m_pe[PE_FrameFocusRect] = method(primitiveSkipper);
    m_pe[PE_IndicatorArrowDown] = method(drawArrowDown);
    m_pe[PE_IndicatorArrowLeft] = method(drawArrowLeft);
    m_pe[PE_IndicatorArrowRight] = method(drawArrowRight);
    m_pe[PE_IndicatorArrowUp] = method(drawArrowUp);
    m_pe[PE_FrameButtonTool] = method(drawToolButtonBevel);
    m_pe[PE_PanelButtonTool] = method(drawToolButtonBevel);

    /* events */
    m_ev[QEvent::Paint] = method(paintEvent);
    m_ev[QEvent::Resize] = method(resizeEvent);
    m_ev[QEvent::Show] = method(showEvent);

    /* subcontrol rects */
    m_sc[CC_ScrollBar] = method(scrollBarRect);
    m_sc[CC_ComboBox] = method(comboBoxRect);
    m_sc[CC_GroupBox] = method(groupBoxRect);
    m_sc[CC_ToolButton] = method(toolButtonRect);
    m_sc[CC_SpinBox] = method(spinBoxRect);
    m_sc[CC_Slider] = method(sliderRect);
}
