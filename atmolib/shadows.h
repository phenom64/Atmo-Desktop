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
#ifndef SHADOWS_H
#define SHADOWS_H

#include "namespace.h"

namespace NSE
{

//enum ShadowType { Sunken = 0,
//                  Etched,   //maclike toolbar shadow pre-yosemite
//                  Raised,   //pretty much a normal pushbutton like shadow
//                  Yosemite, //yosemite simple shadow that reacts differently if widget inside toolbar...
//                  Carved,   //rhino like
//                  Rect,     //simple rounded rectangle, no honky-ponky
//                  ElCapitan,
//                  ShadowCount };

class Q_DECL_EXPORT Shadow
{
public:
    Shadow(const ShadowStyle t, const quint8 r, const quint8 o, const quint8 i);
    void render(const QRect &r, QPainter *p, const Sides s = All);
    static void render(QPixmap *shadow, const QRect &r, QPainter *p, const Sides s = All);
    static quint8 margin(const ShadowStyle s);
    inline ShadowStyle type() const {return m_type;}
    inline quint8 round() const { return m_round; }
    inline quint8 opacity() const { return m_opacity; }

protected:
    void genShadow();
    static QPixmap *split(const QPixmap &pix, const quint8 size, const quint8 cornerSize);

private:
    ShadowStyle m_type;
    quint8 m_round, m_opacity, m_illumination;
    QPixmap *m_pix;
};

class Q_DECL_EXPORT Focus
{
    Focus(const quint8 r, const QColor &c);
};

class Q_DECL_EXPORT Hover : public Shadow
{
public:
    static QPixmap *mask(const QColor &h, const quint8 r);
    static void render(const QRect &r, const QColor &c, QPainter *p, const quint8 round, const Sides s = All, const quint8 level = 0);
};

} //namespace

#endif //SHADOWS_H
