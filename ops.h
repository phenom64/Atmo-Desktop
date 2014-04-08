#ifndef OPS_H
#define OPS_H

#include <QColor>

class Ops
{
public:
    Ops();
    static QColor mid(const QColor &c1, const QColor c2, int i1 = 1, int i2 = 1);
};

#endif // OPS_H
