#ifndef SHADOWHANDLER_H
#define SHADOWHANDLER_H

#include <QWidget>

class QMenu;
class QToolButton;
class Q_DECL_EXPORT ShadowHandler : public QObject
{
public:
    static unsigned long *shadows(bool active);
    static unsigned long *menuShadow(bool up, QMenu *m, QToolButton *tb);
    static void installShadows(WId w, bool active = false);
    static void installShadows(QMenu *m);
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static void removeDelete();
    static ShadowHandler *instance();
protected:
    bool eventFilter(QObject *, QEvent *);
private:
    static ShadowHandler m_instance;
};

#endif //SHADOWHANDLER_H
