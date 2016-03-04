#ifndef GFX_H
#define GFX_H

//#include <QPixmap>
//#include <QPainter>
#include <QtGlobal>
#include <QtGui>
#include <QPalette>
#include <QStyleOptionTabV3>
#include "../namespace.h"

class QStyleOption;
class QPainter;
class QPixmap;
class QRect;
class QPoint;
class QBrush;
class QImage;
class QWidget;

namespace DSP
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
    static void drawClickable(ShadowStyle s,
                              QRect r,
                              QPainter *p,
                              QBrush mask,
                              int rnd,
                              int hover = 0,
                              Sides sides = All,
                              const QStyleOption *opt = 0,
                              const QWidget *w = 0,
                              QPoint offset = QPoint());
    static quint8 shadowMargin(const ShadowStyle s);
    static void makeNoise();
    static void drawCheckMark(QPainter *p, const QColor &c, const QRect &r, const bool tristate = false, const bool bevel = false);
    static void drawRadioMark(QPainter *p, const QColor &c, const QRect &r, const bool bevel = false);
    static void drawArrow(QPainter *p, const QColor &c, QRect r, const Direction d, int size, const Qt::Alignment align = Qt::AlignCenter, const bool bevel = false);
    static QRect subRect(const QRect &r, const int flags, const QRect &sr);
    static void drawWindowBg(QPainter *p, const QWidget *w, const QColor &bg, const QPoint &offset = QPoint());

protected:
    static void initShadows();
    static void initTabs();
    static QPixmap noisePix(const quint8 style);

private:
    static QPixmap *s_tab;
    static QPixmap *s_noise;
    static Shadow *s_shadow[MaxRnd+1][ShadowCount][2];
};

} //namespace

#endif // GFX_H
