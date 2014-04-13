#include <QLineEdit>
#include <QTabBar>
#include <QStyleOption>

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
//        if (castObj(const QToolBar *, toolBar, widget))
//            if (toolBar->findChild<const QTabBar *>())
//                return 0;
        return 4;
    }
    case PM_MenuBarItemSpacing: return 4;
//    case PM_SliderThickness: return 12;
    case PM_SliderLength:
    case PM_SliderControlThickness: return qMin(option->rect.height(), option->rect.width());
//    case PM_SliderSpaceAvailable:
    default: break;
    }
    return QCommonStyle::pixelMetric(metric, option, widget);
}

//QStyle::PM_SliderThickness	11	Total slider thickness.
//QStyle::PM_SliderControlThickness	12	Thickness of the slider handle.
//QStyle::PM_SliderLength	13	Length of the slider.
//QStyle::PM_SliderTickmarkOffset	14	The offset between the tickmarks and the slider.
//QStyle::PM_SliderSpaceAvailable
