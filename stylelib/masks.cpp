#include "masks.h"
#include <QPainterPath>
#include <QPainter>
#include <QBrush>
#include <QRect>

using namespace DSP;

void
Mask::render(const QRect &r, const QBrush &b, QPainter *p, const quint8 round, const Sides s)
{
    if (!r.isValid())
        return;
    int x, y, x2, y2;
    r.getCoords(&x, &y, &x2, &y2);
    ++x2; ++y2; //que?
    QPainterPath path;
    if ((s & (Top|Left)) == (Top|Left))
    {
        path.moveTo(x, y+round);
        path.quadTo(x, y, x+round, y);
    }
    else
        path.moveTo(r.topLeft());
    if ((s & (Top|Right)) == (Top|Right))
    {
        path.lineTo(x2-round, y);
        path.quadTo(x2, y, x2, y+round);
    }
    else
        path.lineTo(x2, y);
    if ((s & (Bottom|Right)) == (Bottom|Right))
    {
        path.lineTo(x2, y2-round);
        path.quadTo(x2, y2, x2-round, y2);
    }
    else
        path.lineTo(x2, y2);
    if ((s & (Bottom|Left)) == (Bottom|Left))
    {
        path.lineTo(x+round, y2);
        path.quadTo(x, y2, x, y2-round);
    }
    else
        path.lineTo(x, y2);

    const bool hadAA(p->testRenderHint(QPainter::Antialiasing));
    p->setRenderHint(QPainter::Antialiasing);
    const QBrush brush(p->brush());
    p->setBrush(b);
    const QPen pen(p->pen());
    p->setPen(Qt::NoPen);
    p->drawPath(path);
    p->setBrush(brush);
    p->setPen(pen);
    p->setRenderHint(QPainter::Antialiasing, hadAA);
}
