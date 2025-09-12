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
#ifndef SHAPES_H
#define SHAPES_H

#include <QPainterPath>
namespace NSE
{
class Q_DECL_EXPORT Shapes
{
public:
    static QPainterPath xMark(const QRectF &r, const float width = 2.0f, const Qt::Alignment &align = Qt::AlignCenter);
    static QPainterPath downArrow(const QRectF &r, const float width = 2.0f, const Qt::Alignment &align = Qt::AlignCenter);
    static QPainterPath upArrow(const QRectF &r, const float width = 2.0f, const Qt::Alignment &align = Qt::AlignCenter);
};
}

#endif // SHAPES_H
