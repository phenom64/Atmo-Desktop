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
#ifndef MACROS_H
#define MACROS_H

#define SUNKEN state & (QStyle::State_Sunken | QStyle::State_Selected | QStyle::State_On)
#define HOVER state & QStyle::State_MouseOver
#define ENABLED state & QStyle::State_Enabled

#define sAdjusted(_X1_, _Y1_, _X2_, _Y2_) adjusted(bool(sides&Left)*_X1_, bool(sides&Top)*_Y1_, bool(sides&Right)*_X2_, bool(sides&Bottom)*_Y2_)
#define sAdjust(_X1_, _Y1_, _X2_, _Y2_) adjust(bool(sides&Left)*_X1_, bool(sides&Top)*_Y1_, bool(sides&Right)*_X2_, bool(sides&Bottom)*_Y2_)
#define sShrink(_S_) sAdjust(_S_, _S_, -_S_, -_S_)
#define sShrinked(_S_) sAdjusted(_S_, _S_, -_S_, -_S_)
#define shrinked(_S_) adjusted(_S_, _S_, -_S_, -_S_)
#define shrink(_S_) adjust(_S_, _S_, -_S_, -_S_)
#define sGrow(_S_) sAdjust(-_S_, -_S_, _S_, _S_)
#define sGrowed(_S_) sAdjusted(-_S_, -_S_, _S_, _S_)
#define growed(_S_) adjusted(-_S_, -_S_, _S_, _S_)
#define grow(_S_) adjust(-_S_, -_S_, _S_, _S_)

#define ISURLBTN inherits("KDEPrivate::KUrlNavigatorButton")

#define BOLD { QFont f(painter->font());\
    f.setBold(true); \
    painter->setFont(f); }

#endif // MACROS_H
