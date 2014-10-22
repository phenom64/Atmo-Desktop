#ifndef RENDER_H
#define RENDER_H

#include <QPixmap>
#include <QPainter>

#define MAXRND 32

class Q_DECL_EXPORT Render
{
public:
    enum Side { Left = 0x1, Top = 0x2, Right = 0x4, Bottom = 0x8, All = 0xf };
    enum SidePart { West = 0, North, East, South, SidePartCount };
    enum Pos { First = 0, Middle = 1, Last = 2, Alone = 3 };
    enum Part { TopLeftPart = 0, TopMidPart, TopRightPart, LeftPart, CenterPart, RightPart, BottomLeftPart, BottomMidPart, BottomRightPart, PartCount };
    enum ShadowType { Sunken = 0, Etched, Raised, Simple, Carved, Strenghter, ShadowCount };
    enum Tab { BeforeSelected = 0, Selected = 1, AfterSelected = 2 };
    enum Effect { Noeffect =0, Inset, Outset };
    Render();
    ~Render(){}
    typedef uint Sides, Shadow;
    static Render *instance();
    static void deleteInstance();
    static QImage blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly = false);
    static inline void generateData() { instance()->_generateData(); }
    static inline void renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess = MAXRND, const Sides sides = All, const QPoint &offSet = QPoint())
    { instance()->_renderMask(rect, painter, brush, roundNess, sides, offSet); }
    static inline void renderShadow(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess = MAXRND, const Sides sides = All, const float opacity = 1.0f, const QBrush *brush = 0)
    { instance()->_renderShadow(shadow, rect, painter, roundNess, sides, opacity, brush); }
    static inline void renderTab(const QRect &r, QPainter *p, const Tab t, QPainterPath *path = 0, const float o = 1.0f)
    { instance()->_renderTab(r, p, t, path, o); }
    static Sides checkedForWindowEdges(const QWidget *w, Sides from = All);
    static void colorizePixmap(QPixmap &pix, const QBrush &b);
    static QPixmap colorized(QPixmap pix, const QBrush &b);
    static QPixmap noise() { return instance()->m_noise; }
    static QPixmap mid(const QPixmap &p1, const QBrush &b, const int a1 = 1, const int a2 = 1);
    static QPixmap mid(const QPixmap &p1, const QPixmap &p2, const int a1 = 1, const int a2 = 1, const QSize &sz = QSize());
    static void drawClickable(const Shadow s, QRect r, QPainter *p, const Sides sides = All, int rnd = MAXRND, const float opacity = 1.0f, const QWidget *w = 0, QBrush *mask = 0, QBrush *shadow = 0, const bool needStrong = false, const QPoint &offSet = QPoint());
    static Pos pos(const Sides s, const Qt::Orientation o);
    static int maskHeight(const Shadow s, const int height);
    static int maskWidth(const Shadow s, const int width);
    static QRect maskRect(const Shadow s, const QRect &r, const Sides sides);
    static QPixmap sunkenized(const QRect &r, const QPixmap &source, const bool isDark = false, const QColor &ref = QColor());
    static QPixmap monochromized(const QPixmap &source, const QColor &color, const Effect effect = Noeffect, const bool isDark = false);

protected:
    void _generateData();
    void _renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess, const Sides sides, const QPoint &offSet);
    void _renderShadowPrivate(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess, const float opacity, const Sides sides);
    void _renderShadow(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess, const Sides sides, const float opacity, const QBrush *brush);
    void _renderTab(const QRect &r, QPainter *p, const Tab t, QPainterPath *path, const float o);
    void initMaskParts();
    void initShadowParts();
    void initTabs();
    void makeNoise();
    void splitShadowParts(const Shadow shadow, int roundNess, const int size, const QPixmap &source);
    bool isCornerPart(const Part part) const;
    bool needPart(const Part part, const Sides sides) const;
    QPixmap genPart(const Part part, const QPixmap &source, const int roundNess, const Sides sides) const;
    QRect partRect(const QRect &rect, const Part part, const int roundNess, const Sides sides) const;
    QRect rect(const QRect &rect, const Part part, const int roundNess) const;

private:
    static Render m_instance;
    QPixmap m_mask[MAXRND+1][PartCount];
    QPixmap m_shadow[ShadowCount][MAXRND+1][PartCount];
    QPixmap m_tab[PartCount];
    QPixmap m_noise;
};

#endif // RENDER_H
