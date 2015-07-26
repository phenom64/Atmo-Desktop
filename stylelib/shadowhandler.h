#ifndef SHADOWHANDLER_H
#define SHADOWHANDLER_H

#include <QWidget>
#include "xhandler.h"

class QMenu;
class QToolButton;
class Q_DECL_EXPORT ShadowHandler : public QObject
{
public:
    static XHandler::XPixmap *shadows(bool active);
    static XHandler::XPixmap *menuShadow(bool up, QMenu *m, QToolButton *tb);
    static void installShadows(WId w, bool active = false);
    static void installShadows(QMenu *m);
    static void removeShadows(WId w);
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static void removeDelete();
    static ShadowHandler *instance();
protected:
    bool eventFilter(QObject *, QEvent *);
private:
    static ShadowHandler *m_instance;
};

#endif //SHADOWHANDLER_H
