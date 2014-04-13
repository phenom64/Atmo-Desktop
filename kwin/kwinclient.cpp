#include <kwindowinfo.h>

#include "kwinclient.h"

KwinClient::KwinClient(KDecorationBridge *bridge, KDecorationFactory *factory)
    : KDecoration(bridge, factory)
{

}

void
KwinClient::init()
{
    createMainWidget();
    KWindowInfo info(windowId(), NET::WMWindowType, NET::WM2WindowClass);
}
