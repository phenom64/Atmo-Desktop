#include <QStylePlugin>
#include <QWidget>
#include <QStyleOption>
#include <QStyleOptionComplex>
#include <QStyleOptionSlider>
#include <QStyleOptionComboBox>
#include <QDebug>
#include <qmath.h>
#include <QEvent>
#include <QToolBar>
#include <QAbstractItemView>
#include <QMainWindow>

#include "styleproject.h"

QRect
StyleProject::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    if (cc < CCSize && m_sc[cc])
        return (this->*m_sc[cc])(opt, sc, w);

    switch (cc)
    {
    default: return QCommonStyle::subControlRect(cc, opt, sc, w);
    }
}

QRect
StyleProject::subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget) const
{
    switch (r)
    {
    case SE_RadioButtonIndicator:
    case SE_CheckBoxIndicator:
    {
        castOpt(Button, btn, opt);
        if (!btn)
            return QRect();
        const int size(btn->rect.height());
        QRect r(0, 0, size, size);
        return visualRect(opt->direction, btn->rect, r);
    }
    case SE_RadioButtonContents:
    case SE_CheckBoxContents:
    {
        castOpt(Button, btn, opt);
        if (!btn)
            return QRect();
        const int m(btn->rect.height()), w(btn->rect.width()-m);
        return visualRect(opt->direction, btn->rect, QRect(m, 0, w, m));
    }
    default: return QCommonStyle::subElementRect(r, opt, widget);
    }
}

/**
enum SubElement {
    SE_PushButtonContents,
    SE_PushButtonFocusRect,

    SE_CheckBoxIndicator,
    SE_CheckBoxContents,
    SE_CheckBoxFocusRect,
    SE_CheckBoxClickRect,

    SE_RadioButtonIndicator,
    SE_RadioButtonContents,
    SE_RadioButtonFocusRect,
    SE_RadioButtonClickRect,

    SE_ComboBoxFocusRect,

    SE_SliderFocusRect,

    SE_Q3DockWindowHandleRect,

    SE_ProgressBarGroove,
    SE_ProgressBarContents,
    SE_ProgressBarLabel,

    // ### Qt 5: These values are unused; eliminate them
    SE_DialogButtonAccept,
    SE_DialogButtonReject,
    SE_DialogButtonApply,
    SE_DialogButtonHelp,
    SE_DialogButtonAll,
    SE_DialogButtonAbort,
    SE_DialogButtonIgnore,
    SE_DialogButtonRetry,
    SE_DialogButtonCustom,

    SE_ToolBoxTabContents,

    SE_HeaderLabel,
    SE_HeaderArrow,

    SE_TabWidgetTabBar,
    SE_TabWidgetTabPane,
    SE_TabWidgetTabContents,
    SE_TabWidgetLeftCorner,
    SE_TabWidgetRightCorner,

    SE_ViewItemCheckIndicator,
    SE_ItemViewItemCheckIndicator = SE_ViewItemCheckIndicator,

    SE_TabBarTearIndicator,

    SE_TreeViewDisclosureItem,

    SE_LineEditContents,
    SE_FrameContents,

    SE_DockWidgetCloseButton,
    SE_DockWidgetFloatButton,
    SE_DockWidgetTitleBarText,
    SE_DockWidgetIcon,

    SE_CheckBoxLayoutItem,
    SE_ComboBoxLayoutItem,
    SE_DateTimeEditLayoutItem,
    SE_DialogButtonBoxLayoutItem, // ### remove
    SE_LabelLayoutItem,
    SE_ProgressBarLayoutItem,
    SE_PushButtonLayoutItem,
    SE_RadioButtonLayoutItem,
    SE_SliderLayoutItem,
    SE_SpinBoxLayoutItem,
    SE_ToolButtonLayoutItem,

    SE_FrameLayoutItem,
    SE_GroupBoxLayoutItem,
    SE_TabWidgetLayoutItem,

    SE_ItemViewItemDecoration,
    SE_ItemViewItemText,
    SE_ItemViewItemFocusRect,

    SE_TabBarTabLeftButton,
    SE_TabBarTabRightButton,
    SE_TabBarTabText,

    SE_ShapedFrameContents,

    SE_ToolBarHandle,

    // do not add any values below/greater than this
    SE_CustomBase = 0xf0000000
};
*/

QRect
StyleProject::comboBoxRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
    castOpt(ComboBox, cb, opt);
    if (!cb)
        return ret;
    const int arrowSize(cb->rect.height());
    switch (sc)
    {
    case SC_ComboBoxListBoxPopup:
    case SC_ComboBoxFrame: ret = cb->rect; break;
    case SC_ComboBoxArrow: ret = QRect(cb->rect.width()-arrowSize, 0, arrowSize, arrowSize); break;
    case SC_ComboBoxEditField: ret = cb->rect.adjusted(0, 0, -arrowSize, 0); break;
    default: ret = QRect(); break;
    }
    return visualRect(cb->direction, cb->rect, ret);
}

QRect
StyleProject::scrollBarRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
    castOpt(Slider, slider, opt);
    if (!slider)
        return ret;

    QRect r(slider->rect);
    switch (sc)
    {
    case SC_ScrollBarAddLine:
    case SC_ScrollBarSubLine:
        return ret;
    default: break;
    }

    bool hor(slider->orientation == Qt::Horizontal);
    int grooveSize(hor ? r.width() : r.height());
    unsigned int range(slider->maximum-slider->minimum);
    int sliderSize(qMax(pixelMetric(PM_ScrollBarSliderMin, opt, w), (int)((slider->pageStep*grooveSize) / (range+slider->pageStep))));
    int pos = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition, grooveSize-sliderSize, slider->upsideDown);

    switch (sc)
    {
    case SC_ScrollBarGroove: ret = r; break;
    case SC_ScrollBarSlider:
    {
        if (hor)
            ret = QRect(pos, 0, sliderSize, r.height());
        else
            ret = QRect(0, pos, r.width(), sliderSize);
        break;
    }
    case SC_ScrollBarLast:
    case SC_ScrollBarAddPage:
        if (hor)
            ret = QRect(pos+sliderSize, 0, r.width()-(pos+sliderSize), r.height());
        else
            ret = QRect(0, pos+sliderSize, r.width(), r.height()-(pos+sliderSize));
        break;
    case SC_ScrollBarFirst:
    case SC_ScrollBarSubPage:
        if (hor)
            ret = QRect(0, 0, pos, r.height());
        else
            ret = QRect(0, 0, r.width(), pos);
        break;
    default: return QCommonStyle::subControlRect(CC_ScrollBar, opt, sc, w);
    }
    return visualRect(slider->direction, slider->rect, ret);
}

