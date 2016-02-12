#ifndef MASKS_H
#define MASKS_H

#include "namespace.h"
#include <QtGlobal>

class QRect;
class QPainter;

namespace DSP
{

class Q_DECL_EXPORT Mask
{
public:
    static void render(const QRectF &r, const QBrush &b, QPainter *p, float round, const Sides s = All, const QPoint &offset = QPoint());
    static float maxRnd(const QRectF &r, const Sides s, const float rnd = (float)MaxRnd);
    static quint8 maxRnd(const QRect &r, const Sides s, const quint8 rnd = MaxRnd);
};

}

#endif // MASKS_H
