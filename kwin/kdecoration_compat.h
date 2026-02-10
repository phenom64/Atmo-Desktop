#ifndef ATMO_KDECORATION_COMPAT_H
#define ATMO_KDECORATION_COMPAT_H

#include "../defines.h"

#include <QMargins>
#include <QRectF>
#include <QRect>
#include <QPoint>
#include <QMarginsF>
#include <QPointF>

#if defined(ATMO_USE_KDECORATION3)
#include <KDecoration3/Decoration>
#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/DecorationButton>
#include <KDecoration3/DecorationButtonGroup>
#include <KDecoration3/DecorationSettings>
#include <KDecoration3/DecorationShadow>

namespace KDecoration2 = KDecoration3;

using AtmoDecoRect = QRectF;
using AtmoDecoMargins = QMarginsF;
using AtmoDecoPoint = QPointF;
using AtmoDecoratedWindow = KDecoration3::DecoratedWindow;
inline QRect atmoToRect(const AtmoDecoRect &rect) { return rect.toRect(); }
inline QPoint atmoToPoint(const AtmoDecoPoint &point) { return point.toPoint(); }
#else
#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButton>
#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>

using AtmoDecoRect = QRect;
using AtmoDecoMargins = QMargins;
using AtmoDecoPoint = QPoint;
using AtmoDecoratedWindow = KDecoration2::DecoratedClient;
inline QRect atmoToRect(const AtmoDecoRect &rect) { return rect; }
inline QPoint atmoToPoint(const AtmoDecoPoint &point) { return point; }
#endif

#endif // ATMO_KDECORATION_COMPAT_H
