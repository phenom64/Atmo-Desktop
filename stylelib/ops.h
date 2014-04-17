#ifndef OPS_H
#define OPS_H

#include <QColor>
#include <QWidget>

class QTabBar;
class Q_DECL_EXPORT Ops
{
public:
    static QColor mid(const QColor &c1, const QColor c2, int i1 = 1, int i2 = 1);
    static bool contrast(const QColor &c1, const QColor &c2);
    static void ensureContrast(QColor &c1, QColor &c2);
    static void setValue(const int value, QColor &c);
    static int luminosity(const QColor &c);
    static QWidget *window(QWidget *w);
    static bool isSafariTabBar(const QTabBar *tabBar);
    static void updateWindow(WId window);

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
