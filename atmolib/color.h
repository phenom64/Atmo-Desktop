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
#ifndef COLOR_H
#define COLOR_H

#include <QColor>
namespace NSE
{
class Q_DECL_EXPORT Color
{
public:
    static QColor mid(const QColor &c1, const QColor c2, int i1 = 1, int i2 = 1);
    static bool contrast(const QColor &c1, const QColor &c2);
    static void ensureContrast(QColor &c1, QColor &c2);
    static void setValue(const int value, QColor &c);
    static int lum(const QColor &c);
    static void shiftHue(QColor &c, int amount);
    static QColor complementary(QColor c);
    static QColor (&titleBarColors())[2];
//    static QColor *titleBarColors();
};
}

#endif // COLOR_H
