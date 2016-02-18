#ifndef MASKS_H
#define MASKS_H

#include "namespace.h"
#include <QStyleOptionTabV3>
#include <QtGlobal>

class QRect;
class QPainter;

namespace DSP
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
        static QPixmap *tabShadow(const TabStyle s, const TabPos pos, const int shape = QTabBar::RoundedNorth);
        static void drawTab(const TabStyle s, const TabPos pos, QRect r, QPainter *p, const QBrush &b, const QStyleOptionTabV3 *opt);
        static QPainterPath tabPath(const TabStyle s, QRect r, const int shape = QTabBar::RoundedNorth);
        static QRect maskAdjusted(const QRect &r, const int shape = QTabBar::RoundedNorth);
        static QSize maskSize(const int shape = QTabBar::RoundedNorth);
        static void drawMask(QPainter *p, const QRect &r, const TabStyle s, const TabPos pos, const QBrush &b, const int shape = QTabBar::RoundedNorth, const bool outline = false);
        static void drawShadow(QPainter *p, const QRect &r, const TabStyle s, const TabPos pos, const int shape = QTabBar::RoundedNorth);
        static void drawTiles(QPainter *p, const QRect &r, const QPixmap *tiles, const int shape = QTabBar::RoundedNorth);
        static QPoint eraseOffset(const QSize &sz, const TabPos pos, const int shape = QTabBar::RoundedNorth);
        static void eraseSides(const QRect &r, QPainter *p, const QPainterPath &path, const TabPos pos, const int shape = QTabBar::RoundedNorth, const bool outline = false);
        static QRect tabBarRect(const QRect &r, const TabPos pos, const int shape = QTabBar::RoundedNorth);
        static QRect tabRect(const QRect &r, const TabPos pos, const int shape);
        static void split(QPixmap *p, const QImage &img, const int shape = QTabBar::RoundedNorth);
        static QPainterPath chromePath(QRect r, const int shape = QTabBar::RoundedNorth);
        static bool isVertical(const int shape);
        static QPixmap colorized(const QPixmap &pix, const QBrush &b);
    };
};

}

#endif // MASKS_H
