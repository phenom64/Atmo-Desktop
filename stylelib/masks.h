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
    static void render(const QRectF &r, const QBrush &b, QPainter *p, const float round, const Sides s = All, const QPoint &offset = QPoint());
};

}

#endif // MASKS_H
