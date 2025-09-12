/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**************************************************************************
*   Based on styleproject, Copyright (C) 2013 by Robert Metsaranta        *
*   therealestrob@gmail.com                                              *
***************************************************************************/
#include "shapes.h"

using namespace NSE;

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
