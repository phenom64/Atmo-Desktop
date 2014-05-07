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
    enum Tab { BeforeSelected = 0, Selected = 1, AfterSelected = 2 };
    Render();
    ~Render(){}
    typedef uint Sides;
    static Render *instance();
    static QImage blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly = false);
    static inline void generateData() { instance()->_generateData(); }
    static inline void renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess = MAXRND, const Sides sides = All)
    { instance()->_renderMask(rect, painter, brush, roundNess, sides); }
    static inline void renderShadow(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess = MAXRND, const Sides sides = All, const float opacity = 1.0f)
    { instance()->_renderShadow(shadow, rect, painter, roundNess, opacity, sides); }
    static inline void renderTab(const QRect &r, QPainter *p, const Tab t, QPainterPath *path = 0, const float o = 1.0f)
    { instance()->_renderTab(r, p, t, path, o); }
    static Sides checkedForWindowEdges(const QWidget *w, Sides from = All);
    static void colorizePixmap(QPixmap &pix, const QBrush &b);
    static QPixmap colorized(QPixmap pix, const QBrush &b);

protected:
    void _generateData();
    void _renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess, const Sides sides);
    void _renderShadow(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess, const float opacity, const Sides sides);
    void _renderTab(const QRect &r, QPainter *p, const Tab t, QPainterPath *path, const float o);
    void initMaskParts();
    void initShadowParts();
    void initTabs();
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
    QPixmap m_tab[AfterSelected+1][PartCount]; //before selected, selected and after selected...
};

#endif // RENDER_H
