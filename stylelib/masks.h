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
    static void render(const QRect &r, const QBrush &b, QPainter *p, const quint8 round, const Sides s = All);
};

}

#endif // MASKS_H
