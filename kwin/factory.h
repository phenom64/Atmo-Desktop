#ifndef FACTORY_H
#define FACTORY_H

#include <kdecorationfactory.h>
#include <QObject>
#include <X11/Xatom.h>

#include "kwinclient.h"

class Factory : public QObject, public KDecorationFactory
{
    Q_OBJECT
public:
    Factory();
    ~Factory();
    KDecoration *createDecoration(KDecorationBridge *bridge);
    bool supports(Ability ability) const;
    void update(WId window, unsigned int changed);

    static bool xEventFilter(void *message);

private:
//    static Atom s_wmAtom;
    static Factory *s_instance;
};


#endif //FACTORY_H
