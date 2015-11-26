#ifndef FACTORY_H
#define FACTORY_H

#include <kdecorationfactory.h>
#include <QObject>
#include <X11/Xatom.h>

#include "../stylelib/xhandler.h"
#include "kwinclient.h"
#include "../config/settings.h"

namespace DSP
{

class KwinClient;
class Factory : public QObject, public KDecorationFactory
{
    Q_OBJECT
public:
    Factory();
    ~Factory();
    KDecoration *createDecoration(KDecorationBridge *bridge);
    bool supports(Ability ability) const;
#if 0
    static bool xEventFilter(void *message);
#endif
    static KwinClient *deco(unsigned long w);

private:
//    static Atom s_wmAtom;
    static Factory *s_instance;
};
} //dsp

#endif //FACTORY_H
