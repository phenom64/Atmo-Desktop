#ifndef FACTORY_H
#define FACTORY_H

#include <kdecorationfactory.h>
#include <QObject>
#include <X11/Xatom.h>

#include "../stylelib/xhandler.h"
#include "kwinclient.h"
#include "../config/settings.h"

typedef struct {
    QColor color[2];
    Gradient gradient;
    unsigned int noiseRatio;
} DecoData;

class KwinClient;
class Factory : public QObject, public KDecorationFactory
{
    Q_OBJECT
public:
    Factory();
    ~Factory();
    KDecoration *createDecoration(KDecorationBridge *bridge);
    bool supports(Ability ability) const;

    static bool xEventFilter(void *message);
    static KwinClient *deco(unsigned long w);
    static DecoData decoData(const QString &winClass);

private:
//    static Atom s_wmAtom;
    static Factory *s_instance;
};


#endif //FACTORY_H
