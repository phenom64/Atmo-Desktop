#ifndef SHADOWS_H
#define SHADOWS_H

#include "namespace.h"

namespace DSP
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
    static quint8 shadowMargin(const ShadowStyle s);
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
