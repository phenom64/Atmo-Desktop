#include "masks.h"
#include <QPainterPath>
#include <QPainter>
#include <QBrush>
#include <QRect>

using namespace DSP;

static void addRoundedRect(const QRectF &rect, const qreal radius, const Sides s, QPainterPath &path)
{
    QRectF r = rect.normalized();

    if (r.isNull())
        return;

    qreal xRadius = radius;
    qreal yRadius = radius;

    qreal w = r.width() / 2;
    qreal h = r.height() / 2;

    if (w == 0) {
        xRadius = 0;
    } else {
        xRadius = 100 * qMin(xRadius, w) / w;
    }
    if (h == 0) {
        yRadius = 0;
    } else {
        yRadius = 100 * qMin(yRadius, h) / h;
    }

    if (xRadius <= 0 || yRadius <= 0) {             // add normal rectangle
        path.addRect(r);
        return;
    }

    qreal x = r.x();
    qreal y = r.y();
    w = r.width();
    h = r.height();
    qreal rxx2 = w*xRadius/100;
    qreal ryy2 = h*yRadius/100;
    if ((s & (Top|Left)) == (Top|Left))
    {
        path.arcMoveTo(x, y, rxx2, ryy2, 180);
        path.arcTo(x, y, rxx2, ryy2, 180, -90);
    }
    else
        path.moveTo(x, y);
    if ((s & (Top|Right)) == (Top|Right))
        path.arcTo(x+w-rxx2, y, rxx2, ryy2, 90, -90);
    else
        path.lineTo(x+w, y);
    if ((s & (Bottom|Right)) == (Bottom|Right))
        path.arcTo(x+w-rxx2, y+h-ryy2, rxx2, ryy2, 0, -90);
    else
        path.lineTo(x+w, y+h);
    if ((s & (Bottom|Left)) == (Bottom|Left))
        path.arcTo(x, y+h-ryy2, rxx2, ryy2, 270, -90);
    else
        path.lineTo(x, y+h);
    path.closeSubpath();
}

void
Mask::render(const QRect &r, const QBrush &b, QPainter *p, const quint8 round, const Sides s)
{
    if (!r.isValid())
        return;
    QPainterPath path;
    addRoundedRect(r, round, s, path);
    const bool hadAA(p->testRenderHint(QPainter::Antialiasing));
    p->setRenderHint(QPainter::Antialiasing);
    p->fillPath(path, b);
    p->setRenderHint(QPainter::Antialiasing, hadAA);
}

void
Mask::renderF(const QRectF &r, const QBrush &b, QPainter *p, const quint8 round, const Sides s)
{
    if (!r.isValid())
        return;
    QPainterPath path;
    addRoundedRect(r, round, s, path);
    const bool hadAA(p->testRenderHint(QPainter::Antialiasing));
    p->setRenderHint(QPainter::Antialiasing);
    p->fillPath(path, b);
    p->setRenderHint(QPainter::Antialiasing, hadAA);
}
