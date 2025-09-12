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
#ifndef MASKS_H
#define MASKS_H

#include "namespace.h"
#include <QStyleOptionTab>
#include <QtGlobal>

class QRect;
class QPainter;

namespace NSE
{

class Q_DECL_EXPORT Mask
{
public:
    static void render(const QRectF &r, const QBrush &b, QPainter *p, float round, const Sides s = All, const QPoint &offset = QPoint());
    static float maxRnd(const QRectF &r, const Sides s, const float rnd = (float)MaxRnd);
    static quint8 maxRnd(const QRect &r, const Sides s, const quint8 rnd = MaxRnd);
    class Tab
    {
    public:
        static QPixmap *tabMask(const TabStyle s, const TabPos pos, const int shape = QTabBar::RoundedNorth, const bool outline = false);
        static QPixmap *tabShadow(const TabStyle s, const TabPos pos, const QColor &c, const int shape = QTabBar::RoundedNorth);
        static void drawTab(const TabStyle s, const TabPos pos, QRect r, QPainter *p, const QWidget *w, QBrush b, const QStyleOptionTab *opt, const quint8 hover = 0, const bool inUno = false);
        static QPainterPath tabPath(const TabStyle s, QRect r, const int shape = QTabBar::RoundedNorth);
        static QRect maskAdjusted(const QRect &r, const int shape = QTabBar::RoundedNorth);
        static QSize maskSize(const TabStyle s, const int shape = QTabBar::RoundedNorth);
        static void drawMask(QPainter *p, const QRect &r, const TabStyle s, const TabPos pos, const QBrush &b, const int shape = QTabBar::RoundedNorth, const bool outline = false);
        static void drawShadow(QPainter *p, const QRect &r, const TabStyle s, const TabPos pos, const QColor &c, const int shape = QTabBar::RoundedNorth);
        static void drawTiles(QPainter *p, const QRect &r, const QPixmap *tiles, const int shape = QTabBar::RoundedNorth);
        static QPoint eraseOffset(const QSize &sz, const TabPos pos, const int shape = QTabBar::RoundedNorth, const int overlap = 18);
        static void eraseSides(const QRect &r, QPainter *p, const QPainterPath &path, const TabPos pos, const int shape = QTabBar::RoundedNorth, const int overlap = 18);
        static QRect tabBarRect(const QRect &r, const TabPos pos, const int shape = QTabBar::RoundedNorth);
        static QRect tabRect(const QRect &r, const TabPos pos, const int shape);
        static void split(QPixmap *p, const QImage &img, const int sideSize, const int shape = QTabBar::RoundedNorth);
        static QPainterPath chromePath(QRect r, const int shape = QTabBar::RoundedNorth);
        static QPainterPath simplePath(QRect r, const int shape = QTabBar::RoundedNorth);
        static bool isVertical(const int shape);
        static QPixmap colorized(const QPixmap &pix, const QBrush &b, const QPoint &offSet = QPoint());
        static int overlap(const TabStyle s);
    };
};

}

#endif // MASKS_H
