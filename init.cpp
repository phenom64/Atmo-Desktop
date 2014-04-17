#include <QApplication>
#include <QDebug>

#include "styleproject.h"
#include "stylelib/ops.h"
#include "render.h"

void
StyleProject::init()
{
    for (int i = 0; i < CCSize; ++i)
        m_cc[i] = 0;
    for (int i = 0; i < CESize; ++i)
        m_ce[i] = 0;
    for (int i = 0; i < PESize; ++i)
        m_pe[i] = 0;
    for (int i = 0; i < EVSize; ++i)
        m_ev[i] = 0;
    QPalette p(QApplication::palette());
    QColor c = Ops::mid(p.color(QPalette::Window), p.color(QPalette::WindowText), 4, 1);
    m_specialColor[0] = Ops::mid(c, Qt::white, 4, 1);
    m_specialColor[1] = Ops::mid(c, Qt::black, 4, 1);
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
    m_ce[CE_MenuBarEmptyArea] = method(controlSkipper);
    m_ce[CE_ItemViewItem] = method(drawViewItem); //draw item in views

    /* complex controls */
    m_cc[CC_ToolButton] = method(drawToolButton);
    m_cc[CC_ScrollBar] = method(drawScrollBar);
    m_cc[CC_Slider] = method(drawSlider);
    m_cc[CC_ComboBox] = method(drawComboBox);

    /* primitive elements */
    m_pe[PE_IndicatorToolBarSeparator] = method(primitiveSkipper);
    m_pe[PE_PanelLineEdit] = method(drawLineEdit);
    m_pe[PE_Frame] = method(drawFrame);
    m_pe[PE_PanelMenuBar] = method(primitiveSkipper);
    m_pe[PE_IndicatorDockWidgetResizeHandle] = method(drawSplitter);
    m_pe[PE_FrameStatusBarItem] = method(primitiveSkipper);
    m_pe[PE_PanelStatusBar] = method(drawStatusBar);
    m_pe[PE_Widget] = method(drawWindow);
    m_pe[PE_FrameWindow] = method(drawWindow);
    m_pe[PE_IndicatorToolBarHandle] = method(primitiveSkipper);
    m_pe[PE_FrameTabBarBase] = method(drawTabBar);
    m_pe[PE_FrameTabWidget] = method(drawTabWidget);
    m_pe[PE_IndicatorTabClose] = method(drawTabCloser);
    m_pe[PE_PanelItemViewItem] = method(drawViewItemBg); //drack BACKGROUND of item in views...

    /* events */
    m_ev[QEvent::Paint] = method(paintEvent);
    m_ev[QEvent::Resize] = method(resizeEvent);
}
