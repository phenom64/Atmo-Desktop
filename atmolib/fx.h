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
#ifndef FX_H
#define FX_H

#include <QtGlobal>
#include <QSize>
#include <qnamespace.h>
#include "namespace.h"
class QImage;
class QRect;
class QPixmap;
class QBrush;
class QColor;

namespace NSE
{

class Q_DECL_EXPORT FX
{
public:
    FX();
    static QImage blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly = false);
    static void expblur(QImage &img, int radius, Qt::Orientations o = Qt::Horizontal|Qt::Vertical );
    static QPixmap mid(const QPixmap &p1, const QBrush &b, const int a1 = 1, const int a2 = 1, const QSize &sz = QSize());
    static QPixmap mid(const QPixmap &p1, const QPixmap &p2, const int a1 = 1, const int a2 = 1, const QSize &sz = QSize());
    static void colortoalpha(float *a1, float *a2, float *a3, float *a4, float c1, float c2, float c3);
    static QPixmap sunkenized(const QRect &r, const QPixmap &source, const bool isDark = false, const int shadowOpacity = 127);
    static QPixmap monochromized(const QPixmap &source, const QColor &color, const Effect effect = Noeffect, bool isDark = false);
    static int stretch(const int v, const float n = 1.5f);
    static int pushed(const float v, const float inlo, const float inup, const float outlo = 0.0f, const float outup = 255.0f);
    static QImage stretched(QImage img);
    static QImage stretched(QImage img, const QColor &c);
    static void autoStretch(QImage &img);
    static void colorizePixmap(QPixmap &pix, const QBrush &b);
    static QPixmap colorized(QPixmap pix, const QBrush &b);
};

}

#endif // FX_H
