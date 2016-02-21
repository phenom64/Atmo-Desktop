#ifndef FX_H
#define FX_H

#include <QtGlobal>
#include <QSize>
#include <qnamespace.h>
#include "namespace.h"
class QImage;
class QRect;
class QPixmap;
class QBrush;
class QColor;

namespace DSP
{

class Q_DECL_EXPORT FX
{
public:
    FX();
    static QImage blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly = false);
    static void expblur(QImage &img, int radius, Qt::Orientations o = Qt::Horizontal|Qt::Vertical );
    static QPixmap mid(const QPixmap &p1, const QBrush &b, const int a1 = 1, const int a2 = 1, const QSize &sz = QSize());
    static QPixmap mid(const QPixmap &p1, const QPixmap &p2, const int a1 = 1, const int a2 = 1, const QSize &sz = QSize());
    static void colortoalpha(float *a1, float *a2, float *a3, float *a4, float c1, float c2, float c3);
    static QPixmap sunkenized(const QRect &r, const QPixmap &source, const bool isDark = false, const QColor &ref = QColor());
    static QPixmap monochromized(const QPixmap &source, const QColor &color, const Effect effect = Noeffect, bool isDark = false);
    static int stretch(const int v, const float n = 2.0f);
    static QImage stretched(QImage img);
    static QImage stretched(QImage img, const QColor &c);
    static void colorizePixmap(QPixmap &pix, const QBrush &b);
    static QPixmap colorized(QPixmap pix, const QBrush &b);
};

}

#endif // FX_H
