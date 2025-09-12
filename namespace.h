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
#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <QtGlobal>
#include <QEvent>
#include <QStyle>

namespace NSE
{
enum {
    CCSize = QStyle::CC_MdiControls+1,
    CESize = QStyle::CE_ShapedFrame+1,
    PESize = QStyle::PE_PanelMenu+1,
    EVSize = QEvent::PlatformPanel+1,
    MaxRnd = 11,
    Steps = 16,
    InactiveTabOffset = 2,
    TabBarBottomSize = 4,
    TabPadding = 6,
    TabDocModePadding = 12
};
enum Side { Left = 0x1, Top = 0x2, Right = 0x4, Bottom = 0x8, All = 0xf };
enum Position { West = 0, North, East, South, PosCount };
enum Pos { First = 0, Middle = 1, Last = 2, Alone = 3 };
enum Parts { TopLeftPart = 0, TopMidPart, TopRightPart, LeftPart, CenterPart, RightPart, BottomLeftPart, BottomMidPart, BottomRightPart, PartCount };
enum ShadowType { Sunken = 0,
                  Etched,   //maclike toolbar shadow pre-yosemite
                  Raised,   //pretty much a normal pushbutton like shadow
                  Yosemite, //yosemite simple shadow that reacts differently if widget inside toolbar...
                  Carved,   //rhino like
                  Rect,     //simple rounded rectangle, no honky-ponky
                  ElCapitan,
                  SemiCarved,
                  LagaDesk, //http://lagadesk.deviantart.com/art/MoNoChroMinal-307076680?q=gallery%3ALaGaDesk%2F11859681&qo=82
                  ShadowCount };

enum TabPart { LeftTabPart = 0, MidTabPart, RightTabPart, TabPartCount };
enum TabPos { BeforeSelected = 0, Selected, AfterSelected, TabPositionCount };
enum Effect { Noeffect =0, Inset, Outset };
enum Control { Input, ProgressBar, PushBtn, Scroller, Slider, Tab, ToolBtn, ControlCount };

enum TabStyle { Chrome = 0, Simple, TabStyleCount };

typedef quint8 Sides, Part, Direction;
typedef qint8 ShadowStyle;
}

#endif //NAMESPACE_H
