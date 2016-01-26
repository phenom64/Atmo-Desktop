#ifndef SHADOWHANDLER_H
#define SHADOWHANDLER_H

#include <QWidget>
#include "xhandler.h"

class QMenu;
class QToolButton;
namespace DSP
{
class Q_DECL_EXPORT ShadowHandler : public QObject
{
public:
    static XHandler::XPixmap *shadows(bool active);
    static void installShadows(WId w, bool active = false);
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
} //namespace

#endif //SHADOWHANDLER_H
