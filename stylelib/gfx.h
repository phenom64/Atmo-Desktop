#ifndef GFX_H
#define GFX_H

//#include <QPixmap>
//#include <QPainter>
#include <QtGlobal>
#include <QtGui>
#include "../namespace.h"

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
class Shadow;
class Q_DECL_EXPORT GFX
{
public:
    static void generateData(const QPalette &pal);
    static void drawMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess = MaxRnd, const Sides sides = All);
    static void drawShadow(const ShadowStyle shadow, const QRect &rect, QPainter *painter, const bool isEnabled = true, int roundNess = MaxRnd, const Sides sides = All);
    static void drawTab(const QRect &r, QPainter *p, const TabPos t, QPainterPath *path = 0, const float o = 1.0f);
    static inline QPixmap &noise(const bool bg = false) { return s_noise[bg]; }
    static void drawClickable(ShadowStyle s,
                              QRect r,
                              QPainter *p,
                              const QBrush &mask,
                              int rnd,
                              const Sides sides = All,
                              const QStyleOption *opt = 0,
                              const QWidget *w = 0);
    static Pos pos(const Sides s, const Qt::Orientation o);
    static int maskHeight(const ShadowStyle s, const int height);
    static int maskWidth(const ShadowStyle s, const int width);
    static int shadowMargin(const ShadowStyle s);
    static QRect maskRect(const ShadowStyle s, const QRect &r, const Sides sides = All);
    static void makeNoise();
    static void drawCheckMark(QPainter *p, const QColor &c, const QRect &r, const bool tristate = false);
    static void drawArrow(QPainter *p, const QPalette::ColorRole role, const QPalette &pal, const bool enabled, const QRect &r, const Direction d, int size, const Qt::Alignment align = Qt::AlignCenter);
    static void drawArrow(QPainter *p, const QColor &c, const QRect &r, const Direction d, int size, const Qt::Alignment align = Qt::AlignCenter, const bool bevel = false);

protected:
    static void initShadows(const QPalette &pal);
    static void initTabs();

private:
    static QPixmap *s_tab;
    static QPixmap *s_noise;
    static Shadow *s_shadow[MaxRnd+1][ShadowCount][2];
};

} //namespace

#endif // GFX_H
