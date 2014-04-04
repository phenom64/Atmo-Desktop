
#include "styleproject.h"

void
StyleProject::init()
{
    for (int i = 0; i < CCSize; ++i)
        m_complexControl[i] = 0;
    for (int i = 0; i < CSize; ++i)
        m_control[i] = 0;
    for (int i = 0; i < PSize; ++i)
        m_primitive[i] = 0;
}

/* here we assign functions to perform the
 * drawing of the elements/widgets/controls.
 *
 * we will add more as we go
 */

void
StyleProject::assignMethods()
{
    m_control[CE_PushButton] = &StyleProject::drawPushButton;
    m_control[CE_PushButtonBevel] = &StyleProject::drawPushButtonBevel;
    m_control[CE_PushButtonLabel] = &StyleProject::drawPushButtonLabel;
    m_control[CE_CheckBox] = &StyleProject::drawCheckBox;
    m_control[CE_CheckBoxLabel] = &StyleProject::drawCheckBoxLabel;
    m_control[CE_RadioButton] = &StyleProject::drawRadioButton;
    m_control[CE_RadioButtonLabel] = &StyleProject::drawRadioButtonLabel;
}
