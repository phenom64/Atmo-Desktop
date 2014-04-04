
#include "styleproject.h"

int
StyleProject::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return QPlastiqueStyle::pixelMetric(metric, option, widget);
}
