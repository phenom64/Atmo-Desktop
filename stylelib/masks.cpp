#include "masks.h"
#include <QPainterPath>
#include <QPainter>
#include <QBrush>
#include <QRect>
#include <math.h>

using namespace DSP;

static void addRoundedRect(const QRectF &rect, const qreal radius, const Sides s, QPainterPath &path)
{
    QRectF r = rect.normalized();
    if (r.isNull())
        return;

    const qreal x = r.x();
    const qreal y = r.y();
    const qreal w = r.width();
    const qreal h = r.height();
    const qreal rad = radius*2;
    const bool tl((s & (Top|Left)) == (Top|Left));
    const bool tr((s & (Top|Right)) == (Top|Right));
    const bool bl((s & (Bottom|Left)) == (Bottom|Left));
    const bool br((s & (Bottom|Right)) == (Bottom|Right));

    path.moveTo(x+w, y+(rad*tr));
    if (tr)
        path.arcTo(x+w-rad, y, rad, rad, 0.0, 90.0); //topright
    path.lineTo(x+(rad*tl), y);
    if (tl)
        path.arcTo(x, y, rad, rad, 90.0, 90.0); //topleft
    path.lineTo(x, y+h-(rad*bl));
    if (bl)
        path.arcTo(x, y+h-rad, rad, rad, 180.0, 90.0); //bottomleft
    path.lineTo(x+w-(rad*br), y+h);
    if (br)
        path.arcTo(x+w-rad, y+h-rad, rad, rad, 270.0, 90.0); //bottomright
    path.closeSubpath();
}


void
Mask::render(const QRect &r, const QBrush &b, QPainter *p, const quint8 round, const Sides s)
{
    if (!r.isValid())
        return;
    int x, y, x2, y2;
    r.getCoords(&x, &y, &x2, &y2);
    ++x2; ++y2; //que?
    QPainterPath path;
    addRoundedRect(r, round, s, path);
    const bool hadAA(p->testRenderHint(QPainter::Antialiasing));
    p->setRenderHint(QPainter::Antialiasing);
    p->fillPath(path, b);
    p->setRenderHint(QPainter::Antialiasing, hadAA);
}
