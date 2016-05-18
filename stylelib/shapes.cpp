#include "shapes.h"

using namespace DSP;

QPainterPath
Shapes::xMark(const QRectF &r, const float width, const Qt::Alignment &align)
{
    qreal x,y,w,h;
    r.getRect(&x,&y,&w,&h);
    qreal cx(x+(w/2.0f)), cy(y+(h/2.0f));
    const qreal split(width/2.0f);
    QPainterPath path;
    path.moveTo(x,y+split);
    path.lineTo(cx-split, cy);
    path.lineTo(x, (y+h)-split);
    path.lineTo(x+split, y+h);
    path.lineTo(cx, cy+split);
    path.lineTo((x+w)-split, y+h);
    path.lineTo(x+w, (y+h)-split);
    path.lineTo(cx+split, cy);
    path.lineTo(x+w, y+split);
    path.lineTo((x+w)-split, y);
    path.lineTo(cx, cy-split);
    path.lineTo(x+split, y);
    path.closeSubpath();
    return path;
}

QPainterPath
Shapes::downArrow(const QRectF &r, const float width, const Qt::Alignment &align)
{
    QRectF rect(0,0,r.width(),r.width()/2.0f+1.0f);
    rect.moveCenter(r.center());
    qreal x,y,w,h;
    rect.getRect(&x,&y,&w,&h);
    qreal cx(x+(w/2.0f));
    const qreal split(width/2.0f);
    QPainterPath path;
    path.moveTo(x,y+split);
    path.lineTo(cx, y+h);
    path.lineTo(x+w, y+split);
    path.lineTo((x+w)-split, y);
    path.lineTo(cx, (y+h)-width);
    path.lineTo(x+split, y);
    path.closeSubpath();
    return path;
}

QPainterPath
Shapes::upArrow(const QRectF &r, const float width, const Qt::Alignment &align)
{
    QRectF rect(0,0,r.width(),r.width()/2.0f+1.0f);
    rect.moveCenter(r.center());
    qreal x,y,w,h;
    rect.getRect(&x,&y,&w,&h);
    qreal cx(x+(w/2.0f));
    const qreal split(width/2.0f);
    QPainterPath path;
    path.moveTo(x,(y+h)-split);
    path.lineTo(cx, y);
    path.lineTo(x+w, (y+h)-split);
    path.lineTo((x+w)-split, y+h);
    path.lineTo(cx, y+width);
    path.lineTo(x+split, y+h);
    path.closeSubpath();
    return path;
}
