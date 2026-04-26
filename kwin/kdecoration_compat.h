#ifndef ATMO_KDECORATION_COMPAT_H
#define ATMO_KDECORATION_COMPAT_H

#include "../defines.h"

#include <QMargins>
#include <QRectF>
#include <QRect>
#include <QPoint>
#include <QMarginsF>
#include <QPointF>

#include <KDecoration3/Decoration>
#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/DecorationButton>
#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/DecorationSettings>
#include <KDecoration3/DecorationShadow>
#include <KDecoration3/ScaleHelpers>

using AtmoDecoRect = QRectF;
using AtmoDecoMargins = QMarginsF;
using AtmoDecoPoint = QPointF;
using AtmoDecoratedWindow = KDecoration3::DecoratedWindow;
inline QRect atmoToRect(const AtmoDecoRect &rect) { return rect.toRect(); }
inline QPoint atmoToPoint(const AtmoDecoPoint &point) { return point.toPoint(); }

#endif // ATMO_KDECORATION_COMPAT_H
