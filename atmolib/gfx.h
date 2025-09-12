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
#ifndef GFX_H
#define GFX_H

//#include <QPixmap>
//#include <QPainter>
#include <QtGlobal>
#include <QtGui>
#include <QPalette>
#include <QStyleOptionTab>
#include "../namespace.h"

class QStyleOption;
class QPainter;
class QPixmap;
class QRect;
class QPoint;
class QBrush;
class QImage;
class QWidget;

namespace NSE
{
class Shadow;
class Q_DECL_EXPORT GFX
{
public:
    static void generateData();
    static void drawMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess = MaxRnd, const Sides sides = All, const QPoint &offset = QPoint());
    static void drawShadow(const ShadowStyle shadow, const QRect &rect, QPainter *painter, const bool isEnabled = true, int roundNess = MaxRnd, const Sides sides = All);
    static void drawTab(const QRect &r, QPainter *p, const TabPos t, QPainterPath *path = 0);
    static void drawTabBarShadow(QPainter *p, QRect r);
    static inline QPixmap noise(const bool bg = false) { return s_noise[bg]; }
    static QPixmap noisePix(const qint8 style, const QString &fileName = QString());
    static void drawClickable(ShadowStyle s,
                              QRect r,
                              QPainter *p,
                              QBrush mask,
                              int rnd,
                              int hover = 0,
                              Sides sides = All,
                              const QStyleOption *opt = 0,
                              const QWidget *w = 0,
                              QPoint offset = QPoint(),
                              const bool invertedSides = false);
    static quint8 shadowMargin(const ShadowStyle s);
    static void makeNoise();
    static void drawCheckMark(QPainter *p, const QColor &c, const QRect &r, const bool tristate = false, const bool bevel = false);
    static void drawRadioMark(QPainter *p, const QColor &c, const QRect &r, const bool bevel = false);
    static void drawArrow(QPainter *p, const QColor &c, QRect r, const Direction d, int size, const Qt::Alignment align = Qt::AlignCenter, const bool bevel = false);
    static void drawHandle(QPainter *p, const QRect &r, const bool hor);
    static QRect subRect(const QRect &r, const int flags, const QRect &sr);
    static void drawWindowBg(QPainter *p, const QWidget *w, const QColor &bg, const QRect &rect, QPoint offset = QPoint());

protected:
    static void initShadows();
    static void initTabs();

private:
    static QPixmap *s_tab;
    static QPixmap *s_noise;
    static Shadow *s_shadow[MaxRnd+1][ShadowCount][2];
};

} //namespace

#endif // GFX_H
