
#include "styleproject.h"

int
StyleProject::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric)
    {
    case PM_DefaultFrameWidth: return 2;
    case PM_ToolBarItemSpacing: return 0;
    case PM_ToolBarSeparatorExtent: return 8;
    default: break;
    }
    return QPlastiqueStyle::pixelMetric(metric, option, widget);
}
