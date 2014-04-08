#ifndef RENDER_H
#define RENDER_H

#include <QPixmap>
#include <QPainter>

#define MAXRND 32

class Render
{
public:
    enum Sides { Left = 0x1, Top = 0x2, Right = 0x4, Bottom = 0x8, All = 0xf };
    enum Part { TopLeftPart = 0, TopMidPart, TopRightPart, LeftPart, CenterPart, RightPart, BottomLeftPart, BottomMidPart, BottomRightPart, PartCount };
    enum Shadow { Sunken = 0, Raised, Etched };
    Render();
    ~Render(){}
    static Render *instance();
    static inline void generateData() { instance()->_generateData(); }
    static inline void renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess = MAXRND-1, const Sides &parts = All)
    { instance()->_renderMask(rect, painter, brush, roundNess, parts); }

protected:
    void _generateData();
    void _renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, const int roundNess, const Sides &sides);
    bool isCornerPart(const Part &part) const;
    bool needPart(const Part &part, const Sides &sides) const;
    QPixmap genPart(const Part &part, const QPixmap &source, const Sides &sides, QRect &inSource, const int roundNess) const;
    QRect partRect(const QRect &rect, const Part &part, const int roundNess, const Sides &sides) const;
    QRect rect(const QRect &rect, const Part &part, const int roundNess) const;

private:
    static Render *m_instance;
    QPixmap m_mask[MAXRND][PartCount];
};

#endif // RENDER_H
