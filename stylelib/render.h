#ifndef RENDER_H
#define RENDER_H

//#include <QPixmap>
//#include <QPainter>
#include <QtGlobal>
#include <QtGui>

#define MAXRND 32

class QStyleOption;
class QPainter;
class QPixmap;
class QRect;
class QPoint;
class QBrush;
class QImage;
class QWidget;
class QPalette;
namespace DSP
{
enum Side { Left = 0x1, Top = 0x2, Right = 0x4, Bottom = 0x8, All = 0xf };
enum { West = 0, North, East, South, SidePartCount };
enum Pos { First = 0, Middle = 1, Last = 2, Alone = 3 };
enum Parts { TopLeftPart = 0, TopMidPart, TopRightPart, LeftPart, CenterPart, RightPart, BottomLeftPart, BottomMidPart, BottomRightPart, PartCount };
enum ShadowType { Sunken = 0,
                  Etched,   //maclike toolbar shadow pre-yosemite
                  Raised,   //pretty much a normal pushbutton like shadow
                  Yosemite, //yosemite simple shadow that reacts differently if widget inside toolbar...
                  Carved,   //rhino like
                  Rect,     //simple rounded rectangle, no honky-ponky
                  ElCapitan,
                  ShadowCount };
enum Tab { BeforeSelected = 0, Selected = 1, AfterSelected = 2 };
enum Effect { Noeffect =0, Inset, Outset };
typedef quint8 Sides, Shadow, Part, Direction;

class Q_DECL_EXPORT Render
{
public:
    static void deleteInstance();
    static QImage blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly = false);
    static void generateData(const QPalette &pal);
    static void renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess = MAXRND, const Sides sides = All, const QPoint &offSet = QPoint(), const int opacity = 255);
    static void renderShadow(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess = MAXRND, const Sides sides = All, const float opacity = 1.0f, const QBrush *brush = 0);
    static void renderTab(const QRect &r, QPainter *p, const Tab t, QPainterPath *path = 0, const float o = 1.0f);
    static Sides checkedForWindowEdges(const QWidget *w, Sides from = All);
    static Sides checkedForParentEdges(const QWidget *w, Sides from = All);
    static void colorizePixmap(QPixmap &pix, const QBrush &b);
    static QPixmap colorized(QPixmap pix, const QBrush &b);
    static inline QPixmap &noise() { return *s_noise; }
    static QPixmap mid(const QPixmap &p1, const QBrush &b, const int a1 = 1, const int a2 = 1);
    static QPixmap mid(const QPixmap &p1, const QPixmap &p2, const int a1 = 1, const int a2 = 1, const QSize &sz = QSize());
    static void drawClickable(Shadow s,
                              QRect r,
                              QPainter *p,
                              int rnd = MAXRND,
                              float opacity = 1.0f,
                              const QWidget *w = 0,
                              const QStyleOption *o = 0,
                              QBrush *mask = 0,
                              QBrush *shadow = 0,
                              const Sides sides = All,
                              const QPoint &offSet = QPoint());
    static Pos pos(const Sides s, const Qt::Orientation o);
    static int maskHeight(const Shadow s, const int height);
    static int maskWidth(const Shadow s, const int width);
    static int shadowMargin(const Shadow s);
    static QRect maskRect(const Shadow s, const QRect &r, const Sides sides = All);
    static QPixmap sunkenized(const QRect &r, const QPixmap &source, const bool isDark = false, const QColor &ref = QColor());
    static QPixmap monochromized(const QPixmap &source, const QColor &color, const Effect effect = Noeffect, bool isDark = false);
    static void expblur(QImage &img, int radius, Qt::Orientations o = Qt::Horizontal|Qt::Vertical );
    static void shapeCorners(QPainter *p, Sides s, int roundNess = 4, const QSize &forceSize = QSize());
    static void makeNoise();
    static void drawCheckMark(QPainter *p, const QColor &c, const QRect &r, const bool tristate = false);
    static void drawArrow(QPainter *p, const QPalette::ColorRole role, const QPalette &pal, const bool enabled, const QRect &r, const Direction d, int size, const Qt::Alignment align = Qt::AlignCenter);
    static void drawArrow(QPainter *p, const QColor &c, const QRect &r, const Direction d, int size, const Qt::Alignment align = Qt::AlignCenter, const bool bevel = false);

protected:
//    void generateData(const QPalette &pal);
//    void _renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess, const Sides sides, const QPoint &offSet);
    static void renderShadowPrivate(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess, const float opacity, const Sides sides);
//    void _renderShadow(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess, const Sides sides, const float opacity, const QBrush *brush);
//    void _renderTab(const QRect &r, QPainter *p, const Tab t, QPainterPath *path, const float o);
    static void initMaskParts();
    static void initShadowParts(const QPalette &pal);
    static void initTabs();
//    void _makeNoise();
    static void splitShadowParts(const Shadow shadow, int roundNess, int size, const QPixmap &source);
    static bool isCornerPart(const Part part);
    static bool needPart(const Part part, const Sides sides);
    static QPixmap genPart(const Part part, const QPixmap &source, const int roundNess, const Sides sides);
    static QRect partRect(const QRect &rect, const Part part, int roundNess, const Sides sides, bool isShadow = false);
    static QImage stretched(QImage img);
    static QImage stretched(QImage img, const QColor &c);

private:
    static QPixmap **s_mask;
    static QPixmap ***s_shadow;
    static QPixmap *s_tab;
    static QPixmap *s_noise;
};
} //namespace

#endif // RENDER_H
