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
    Shadow(const ShadowStyle t, const quint8 r, const quint8 o);
    void render(const QRect &r, QPainter *p, const Sides s = All) const;
    inline ShadowStyle type() const {return m_type;}
    inline quint8 round() const { return m_round; }
    inline quint8 opacity() const { return m_opacity; }

protected:
    void genShadow(const ShadowStyle t);
    void split(const QPixmap &pix, const quint8 size, const quint8 cornerSize);

private:
    ShadowStyle m_type;
    quint8 m_round, m_opacity;
    QPixmap m_pix[PartCount];
};

}

#endif //SHADOWS_H
