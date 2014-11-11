#include <QLineEdit>
#include <QTabBar>
#include <QStyleOption>
#include <QMainWindow>
#include <QComboBox>
#include <QAbstractButton>
#include <QLayout>
#include <QDockWidget>
#include <Q3GroupBox>
#include <QDebug>

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
    case PM_SpinBoxSliderHeight:
        return option->rect.height()/2;
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
        const QFrame *frame = qobject_cast<const QFrame *>(widget);
        if (OverLay::hasOverLay(frame))
            return 0;
        if (qobject_cast<const QGroupBox *>(widget))
            return 4;

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
