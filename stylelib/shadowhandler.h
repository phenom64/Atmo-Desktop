#ifndef SHADOWHANDLER_H
#define SHADOWHANDLER_H

#include <QWidget>

class Q_DECL_EXPORT ShadowHandler : public QObject
{
public:
    static unsigned long *shadows(const int size = 32);
    static void installShadows(WId w);
    static void manage(QWidget *w);
    static void removeDelete();
    static ShadowHandler *instance();
protected:
    bool eventFilter(QObject *, QEvent *);
private:
    static ShadowHandler *m_instance;
};

#endif //SHADOWHANDLER_H
