#ifndef SHADOWHANDLER_H
#define SHADOWHANDLER_H

#include <QWidget>
#include "xhandler.h"

class Q_DECL_EXPORT ShadowHandler
{
public:
    static quint32 *shadows(const int size = 32);
    static void installShadows(WId w);
    static void manage(QWidget *w);
    static void removeDelete();
};

#endif //SHADOWHANDLER_H
