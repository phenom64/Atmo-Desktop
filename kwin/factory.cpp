#include "factory.h"

KWIN_DECORATION(Factory)

KDecoration
*Factory::createDecoration(KDecorationBridge *bridge)
{
    return new KwinClient(bridge, this);
}

bool
Factory::supports(Ability ability) const
{
    return true;
}
