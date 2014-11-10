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
#include <QGroupBox>
#include <QStyleOptionProgressBarV2>
#include <QStyleOptionDockWidget>
#include <QStyleOptionDockWidgetV2>
#include <QProgressBar>
#include <QStyleOptionToolButton>

#include "styleproject.h"
#include "stylelib/progresshandler.h"
#include "stylelib/ops.h"
#include "stylelib/settings.h"
#include "stylelib/render.h"

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
    if (!opt)
        return QRect();
    switch (r)
    {
    case SE_LineEditContents:
    {
        QRect r(Render::maskRect(Settings::conf.input.shadow, opt->rect));
        int h(r.height());
        int hor(qMin(Settings::conf.input.rnd, (h/2))/2);
        r.adjust(hor, 0, -hor, 0);
        return r;
    }
    case SE_HeaderArrow:
    {
        castOpt(Header, hdr, opt);
        QRect r(opt->rect);
        const int extraMargin(16); // some space for good measure...
        const int margin(pixelMetric(PM_HeaderMarkSize)+extraMargin);
        if (hdr->textAlignment & Qt::AlignRight)
            r.setRight(r.left()+margin);
        else
            r.setLeft(r.right()-margin);
        return r;
    }
    case SE_HeaderLabel:
    {
        castOpt(Header, hdr, opt);
        QRect r(opt->rect);
        int space(0);
        if (opt->state & State_Sunken)
            space = pixelMetric(PM_HeaderMarkSize);

        if (hdr->textAlignment & Qt::AlignRight)
            r.setLeft(r.left()+space);
        else
            r.setRight(r.right()-space);

        int m(4);
        r.adjust(m, 0, -m, 0);
        return r;
    }
    case SE_TabBarTabLeftButton:
    case SE_TabBarTabRightButton:
    case SE_TabBarTabText:
    {
        castOpt(TabV3, tab, opt);
        castObj(const QTabBar *, bar, widget);
        if (!tab)
            return QRect();

        const bool safBar(Ops::isSafariTabBar(bar));
        int o(safBar?Settings::conf.tabs.safrnd:4);
        int ladd((safBar && tab->position == QStyleOptionTab::Beginning || tab->position == QStyleOptionTab::OnlyOneTab)*o);
        int radd((safBar && (tab->position == QStyleOptionTab::End || tab->position == QStyleOptionTab::OnlyOneTab) && bar->expanding())*(o));
        QRect rect(tab->rect.adjusted(ladd, 0, -radd, 0));
        QRect textRect(rect);
        QRect leftRect;

        const bool east(tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast);
        const bool west(tab->shape == QTabBar::RoundedWest || tab->shape == QTabBar::TriangularWest);
        const bool leftClose(styleHint(SH_TabBar_CloseButtonPosition, opt, widget) == QTabBar::LeftSide);
        const bool closable(bar && bar->tabsClosable());

        if (tab->leftButtonSize.isValid() || ((closable && !leftClose && tab->rightButtonSize.isValid()) || !tab->icon.isNull()))
        {
            const QSize sz(tab->leftButtonSize.isValid()?tab->leftButtonSize:tab->iconSize);
            leftRect.setSize(sz);
            leftRect.moveCenter(rect.center());
            if (west)
                leftRect.moveBottom(rect.bottom()-o);
            else if (east)
                leftRect.moveTop(rect.top()+o);
            else
                leftRect.moveLeft(rect.left()+o);
        }

        if (leftRect.isValid())
        {
            if (west)
                textRect.setBottom(leftRect.top());
            else if (east)
                textRect.setTop(leftRect.bottom());
            else
                textRect.setLeft(leftRect.right());
        }

        QRect rightRect;
        if (tab->rightButtonSize.isValid() || (closable && leftClose && tab->leftButtonSize.isValid() && !tab->icon.isNull()))
        {
            const QSize sz(tab->leftButtonSize.isValid()?tab->leftButtonSize:tab->iconSize);
            rightRect.setSize(sz);
            rightRect.moveCenter(rect.center());
            if (west)
                rightRect.moveBottom(rect.bottom()-o);
            else if (east)
                rightRect.moveTop(rect.top()+o);
            else
                rightRect.moveRight(rect.right()-o);
        }

        if (rightRect.isValid())
        {
            if (west)
                textRect.setBottom(rightRect.top());
            else if (east)
                textRect.setTop(rightRect.bottom());
            else
                textRect.setRight(rightRect.left());
        }

        if (tab->text.isEmpty())
            textRect = QRect();

        switch (r)
        {
        case SE_TabBarTabLeftButton:
            return visualRect(tab->direction, tab->rect, leftRect);
        case SE_TabBarTabRightButton:
            return visualRect(tab->direction, tab->rect, rightRect);
        case SE_TabBarTabText:
            return visualRect(tab->direction, tab->rect, textRect);
        default: return QCommonStyle::subElementRect(r, opt, widget);
        }
    }
    case SE_ProgressBarLabel:
    case SE_ProgressBarGroove:
    case SE_ProgressBarContents: return opt->rect;
    case SE_ViewItemCheckIndicator:
    case SE_RadioButtonIndicator:
    case SE_CheckBoxIndicator:
    {
        QRect r(opt->rect.topLeft(), QPoint(opt->rect.topLeft()+QPoint(pixelMetric(PM_IndicatorWidth), pixelMetric(PM_IndicatorHeight))));
        return visualRect(opt->direction, opt->rect, r);
    }
    case SE_RadioButtonContents:
    case SE_CheckBoxContents:
    {
        const int pmInd(pixelMetric(PM_IndicatorWidth)), pmSpc(pixelMetric(PM_CheckBoxLabelSpacing)), w(opt->rect.width()-(pmInd+pmSpc));
        return visualRect(opt->direction, opt->rect, QRect(opt->rect.left()+pmInd+pmSpc, opt->rect.top(), w, opt->rect.height()));
    }
    case SE_DockWidgetCloseButton:
    case SE_DockWidgetFloatButton:
    case SE_DockWidgetTitleBarText:
    case SE_DockWidgetIcon:
    {
        castOpt(DockWidget, dw, opt);
        if (!dw)
            return QRect();

        QRect rect(dw->rect);

        QRect cr, fr;
        int left(rect.left());
        if (dw->closable)
        {
            cr = rect;
            cr.setWidth(cr.height());
            cr.moveLeft(left);
            left += cr.width();
        }
        if (dw->floatable)
        {
            fr = rect;
            fr.setWidth(fr.height());
            fr.moveLeft(left);
            left += cr.width();
        }
        switch (r)
        {
        case SE_DockWidgetCloseButton:
            return visualRect(dw->direction, rect, cr);
        case SE_DockWidgetFloatButton:
            return visualRect(dw->direction, rect, fr);
        case SE_DockWidgetTitleBarText:
            return visualRect(dw->direction, rect, QRect(left, rect.top(), qMax(0, rect.width()-left), rect.height()));
        case SE_DockWidgetIcon:
            return QRect(); // ever needed?
        default: return QRect();
        }
    }
    default: break;
    }
    return QCommonStyle::subElementRect(r, opt, widget);
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
    const int arrowSize(12);
    switch (sc)
    {
    case SC_ComboBoxListBoxPopup: //what kinda rect should be returned here? seems only topleft needed...
    case SC_ComboBoxFrame: ret = cb->rect; break;
    case SC_ComboBoxArrow: ret = cb->rect.adjusted(cb->rect.width()-arrowSize, 0, 0, 0); break;
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
    int gs((int)((slider->pageStep*grooveSize)));
    int ss((range+slider->pageStep));
    int sliderSize(0);
    if (ss && gs)
        sliderSize = gs/ss;
    if (sliderSize < pixelMetric(PM_ScrollBarSliderMin, opt, w))
        sliderSize = pixelMetric(PM_ScrollBarSliderMin, opt, w);
    int pos = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition, grooveSize-sliderSize, slider->upsideDown);

    switch (sc)
    {
    case SC_ScrollBarGroove: ret = r; break;
    case SC_ScrollBarSlider:
    {
        if (!range)
            break;
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

QRect
StyleProject::sliderRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
    castOpt(Slider, slider, opt);
    if (!slider)
        return ret;

    int tm(pixelMetric(PM_SliderTickmarkOffset, opt, w));
    const QSlider::TickPosition tp(slider->tickPosition);
    if (tp == QSlider::TicksBothSides)
        tm/=2;
    QRect r(slider->rect);
    bool hor(slider->orientation == Qt::Horizontal);
    if (slider->tickPosition)
    {
        const bool needLeft(!hor&&(tp==QSlider::TicksLeft||tp==QSlider::TicksBothSides)),
                needRight(!hor&&(tp==QSlider::TicksRight||tp==QSlider::TicksBothSides)),
                needTop(hor&&(tp==QSlider::TicksAbove||tp==QSlider::TicksBothSides)),
                needBottom(hor&&(tp==QSlider::TicksBelow||tp==QSlider::TicksBothSides));
        r.adjust(needLeft*tm, needTop*tm, -(needRight*tm), -(needBottom*tm));
    }
    int grooveSize(hor ? r.width() : r.height());
    unsigned int range(slider->maximum-slider->minimum);
    int sliderSize(pixelMetric(PM_SliderLength, opt, w));

    int pos = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderPosition, grooveSize-sliderSize, slider->upsideDown);

    switch (sc)
    {
    case SC_SliderGroove: ret = r; break;
    case SC_SliderHandle:
    {
        if (!range)
            break;
        if (hor)
            ret = QRect(pos, r.height()/2-sliderSize/2, sliderSize, sliderSize);
        else
            ret = QRect(r.width()/2-sliderSize/2, pos, sliderSize, sliderSize);
        break;
    }
    default: return QCommonStyle::subControlRect(CC_ScrollBar, opt, sc, w);
    }
    return visualRect(slider->direction, slider->rect, ret);
}

QRect
StyleProject::groupBoxRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
    castOpt(GroupBox, box, opt);
    if (!box)
        return ret;
    ret = box->rect;
    const int top(qMax(16, opt->fontMetrics.height()));
    const int left(8);
    switch (sc)
    {
    case SC_GroupBoxCheckBox:
        ret = QRect(ret.left()+left, ret.top(), top, top);
        break;
    case SC_GroupBoxContents:
        ret.setTop(ret.top()+top);
        break;
    case SC_GroupBoxFrame:
        break;
    case SC_GroupBoxLabel:
        ret = QRect(ret.left()+left+(box->subControls&SC_GroupBoxCheckBox?top:0), ret.top(), ret.width()-top, top);
        break;
    default: return QCommonStyle::subControlRect(CC_GroupBox, opt, sc, w);
    }
    return visualRect(box->direction, opt->rect, ret);
}

QRect
StyleProject::toolButtonRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    QRect ret;
    castOpt(ToolButton, tb, opt);
    if (!tb)
        return ret;
    switch (sc)
    {
    case SC_ToolButton: ret = tb->rect; break;
    case SC_ToolButtonMenu:
    {
        ret = tb->rect;
        ret.setLeft(ret.right()-16);
        break;
    }
    default: break;
    }
    return visualRect(tb->direction, tb->rect, ret);
}

QRect
StyleProject::spinBoxRect(const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    castOpt(SpinBox, sb, opt);
    if (!sb)
        return QRect();

    QRect r(QCommonStyle::subControlRect(CC_SpinBox, opt, sc, w));
    int a(qMin(Settings::conf.input.rnd/2, r.height()/4));
    if (sc == SC_SpinBoxEditField)
        r.translate(sb->direction==Qt::RightToLeft?-a:a, 0);
    return r;
}

