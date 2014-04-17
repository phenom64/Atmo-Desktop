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
    static bool compositingActive();
    void update(WId window);

private:
    static Atom s_wmAtom;
};


#endif //FACTORY_H
