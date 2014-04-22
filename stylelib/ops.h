#ifndef OPS_H
#define OPS_H

#include <QColor>
#include <QWidget>

/**
 * This class is a mess atm, I just add stuff here
 * that I need outta the way... most of this stuff
 * need to be properly categorised in some classes
 * that actually makes sense, for now, deal w/ it.
 */

class QTabBar;
class Q_DECL_EXPORT Ops
{
public:
    enum Dir{ Left, Up, Right, Down };
    typedef unsigned int Direction;

    static QWidget *window(QWidget *w);
    static bool isSafariTabBar(const QTabBar *tabBar);
    static void updateWindow(WId window);
    static void drawCheckMark(QPainter *p, const QColor &c, const QRect &r);
    static void drawArrow(QPainter *p, const QColor &c, const QRect &r, const Direction &d);
    static QPalette::ColorRole opposingRole(const QPalette::ColorRole &role);
    static void fixWindowTitleBar(QWidget *win);

    template<typename T> static inline bool isOrInsideA(QWidget *widget)
    {
        if (!widget)
            return false;
        QWidget *w = widget;
        while (w->parentWidget())
        {
            if (qobject_cast<T>(w))
                return true;
            w = w->parentWidget();
        }
        return false;
    }
};

#endif // OPS_H
