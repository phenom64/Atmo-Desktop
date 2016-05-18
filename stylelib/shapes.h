#ifndef SHAPES_H
#define SHAPES_H

#include <QPainterPath>
namespace DSP
{
class Q_DECL_EXPORT Shapes
{
public:
    static QPainterPath xMark(const QRectF &r, const float width = 2.0f, const Qt::Alignment &align = Qt::AlignCenter);
    static QPainterPath downArrow(const QRectF &r, const float width = 2.0f, const Qt::Alignment &align = Qt::AlignCenter);
    static QPainterPath upArrow(const QRectF &r, const float width = 2.0f, const Qt::Alignment &align = Qt::AlignCenter);
};
}

#endif // SHAPES_H
