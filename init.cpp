
#include "styleproject.h"

void
StyleProject::init()
{
    for (int i = 0; i < CCSize; ++i)
        m_cc[i] = 0;
    for (int i = 0; i < CESize; ++i)
        m_ce[i] = 0;
    for (int i = 0; i < PESize; ++i)
        m_pe[i] = 0;
}

/* here we assign functions to perform the
 * drawing of the elements/widgets/controls.
 *
 * we will add more as we go
 */

void
StyleProject::assignMethods()
{
    /* controls */
    m_ce[CE_PushButton] = &StyleProject::drawPushButton;
    m_ce[CE_PushButtonBevel] = &StyleProject::drawPushButtonBevel;
    m_ce[CE_PushButtonLabel] = &StyleProject::drawPushButtonLabel;
    m_ce[CE_CheckBox] = &StyleProject::drawCheckBox;
    m_ce[CE_CheckBoxLabel] = &StyleProject::drawCheckBoxLabel;
    m_ce[CE_RadioButton] = &StyleProject::drawRadioButton;
    m_ce[CE_RadioButtonLabel] = &StyleProject::drawRadioButtonLabel;
    m_ce[CE_ToolButtonLabel] = &StyleProject::drawToolButtonLabel;
    m_ce[CE_ToolBar] = &StyleProject::controlSkipper;
    m_ce[CE_Splitter] = &StyleProject::controlSkipper;
    m_ce[CE_SizeGrip] = &StyleProject::controlSkipper;

    /* complex controls */
    m_cc[CC_ToolButton] = &StyleProject::drawToolButton;

    /* primitives */
    m_pe[PE_IndicatorToolBarSeparator] = &StyleProject::primitiveSkipper;
    m_pe[PE_PanelLineEdit] = &StyleProject::drawLineEdit;
}
