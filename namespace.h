#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <QtGlobal>
#include <QEvent>
#include <QStyle>

namespace DSP
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
