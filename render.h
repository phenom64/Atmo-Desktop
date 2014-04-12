#ifndef RENDER_H
#define RENDER_H

#include <QPixmap>
#include <QPainter>

#define MAXRND 32

class Render
{
public:
    enum Side { Left = 0x1, Top = 0x2, Right = 0x4, Bottom = 0x8, All = 0xf };
    enum Part { TopLeftPart = 0, TopMidPart, TopRightPart, LeftPart, CenterPart, RightPart, BottomLeftPart, BottomMidPart, BottomRightPart, PartCount };
    enum Shadow { Sunken = 0, Etched, Raised, ShadowCount };
    Render();
    ~Render(){}
    typedef uint Sides;
    static Render *instance();
    static inline void generateData() { instance()->_generateData(); }
    static inline void renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess = MAXRND, const Sides sides = All)
    { instance()->_renderMask(rect, painter, brush, roundNess, sides); }
    static inline void renderShadow(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess = MAXRND, const Sides sides = All)
    { instance()->_renderShadow(shadow, rect, painter, roundNess, sides); }
    static Sides checkedForWindowEdges(const QWidget *w, Sides from = All);

protected:
    void _generateData();
    void _renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess, const Sides sides);
    void _renderShadow(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess, const Sides sides);
    void initMaskParts();
    void initShadowParts();
    void splitShadowParts(const Shadow shadow, int roundNess, const int size, const QPixmap &source);
    bool isCornerPart(const Part part) const;
    bool needPart(const Part part, const Sides sides) const;
    QPixmap genPart(const Part part, const QPixmap &source, const int roundNess, const Sides sides) const;
    QRect partRect(const QRect &rect, const Part part, const int roundNess, const Sides sides) const;
    QRect rect(const QRect &rect, const Part part, const int roundNess) const;

private:
    static Render *m_instance;
    QPixmap m_mask[MAXRND+1][PartCount];
    QPixmap m_shadow[ShadowCount][MAXRND+1][PartCount];
};

#endif // RENDER_H
