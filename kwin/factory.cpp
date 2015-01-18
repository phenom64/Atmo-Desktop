#include <QDBusConnection>
#include <QDBusMessage>
#include <QPainter>

#include "kwinclient.h"
#include "factory.h"
#include "factorydbusadaptor.h"
#include "../stylelib/xhandler.h"
#include "../stylelib/shadowhandler.h"

KWIN_DECORATION(Factory)

KDecoration
*Factory::createDecoration(KDecorationBridge *bridge)
{
    return new KwinClient(bridge, this);
}

//Atom Factory::s_wmAtom;

Factory::Factory()
    : QObject()
    , KDecorationFactory()
{
//    QString string = QString("_NET_WM_CM_S%1").arg(DefaultScreen(QX11Info::display()));
//    s_wmAtom = XInternAtom(QX11Info::display(), string.toAscii().data(), False);
    ShadowHandler::removeDelete();
    new FactoryDbusAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/StyleProjectFactory", this);
}

Factory::~Factory()
{
    ShadowHandler::removeDelete();
}

bool
Factory::compositingActive()
{
//    return XGetSelectionOwner( QX11Info::display(), s_wmAtom ) != None;
    return QX11Info::isCompositingManagerRunning();
}

bool
Factory::supports(Ability ability) const
{
    switch (ability)
    {
    // announce
    case AbilityAnnounceButtons: ///< decoration supports AbilityButton* values (always use)
    case AbilityAnnounceColors: ///< decoration supports AbilityColor* values (always use), @deprecated @todo remove KDE5
        return false;
    // buttons
    case AbilityButtonMenu:   ///< decoration supports the window menu button
    case AbilityButtonOnAllDesktops: ///< decoration supports the on all desktops button
    case AbilityButtonSpacer: ///< decoration supports inserting spacers between buttons
    case AbilityButtonHelp:   ///< decoration supports what's this help button
    case AbilityButtonMinimize:  ///< decoration supports a minimize button
    case AbilityButtonMaximize: ///< decoration supports a maximize button
    case AbilityButtonClose: ///< decoration supports a close button
    case AbilityButtonAboveOthers: ///< decoration supports an above button
    case AbilityButtonBelowOthers: ///< decoration supports a below button
    case AbilityButtonShade: ///< decoration supports a shade button
        return true;
    case AbilityButtonResize: ///< decoration supports a resize button
        return false;
    case AbilityButtonApplicationMenu:   ///< decoration supports the application menu button
        return true;
    // colors
    case AbilityColorTitleBack: ///< decoration supports titlebar background color, @deprecated @todo remove KDE5
    case AbilityColorTitleFore:  ///< decoration supports titlebar foreground color, @deprecated @todo remove KDE5
    case AbilityColorTitleBlend: ///< decoration supports second titlebar background color, @deprecated @todo remove KDE5
    case AbilityColorFrame: ///< decoration supports frame color, @deprecated @todo remove KDE5
    case AbilityColorHandle: ///< decoration supports resize handle color, @deprecated @todo remove KDE5
    case AbilityColorButtonBack: ///< decoration supports button background color, @deprecated @todo remove KDE5
    case AbilityColorButtonFore: ///< decoration supports button foreground color, @deprecated @todo remove KDE5
    case ABILITYCOLOR_END: ///< @internal, @deprecated @todo remove KDE5
    // compositing
        return true;
    case AbilityProvidesShadow: ///< The decoration draws its own shadows.
    ///  @since 4.3
        return false;
    case AbilityUsesAlphaChannel: ///< The decoration isn't clipped to the mask when compositing is enabled.
    ///  The mask is still used to define the input region and the blurred
    ///  region, when the blur plugin is enabled.
    ///  @since 4.3
        return true;
    case AbilityExtendIntoClientArea: ///< The decoration respects transparentRect()
        return true;
    ///  @since 4.4
    case AbilityUsesBlurBehind: ///< The decoration wants the background to be blurred, when the blur plugin is enabled.
    /// @since 4.6
        return false;
    case AbilityAnnounceAlphaChannel: ///< The decoration can tell whether it currently uses an alpha channel or not. Requires AbilityUsesAlphaChannel.
    /// @since 4.10
        return false;
    // Tabbing
    case AbilityTabbing: ///< The decoration supports tabbing
    // TODO colors for individual button types
    case ABILITY_DUMMY:
        return false;

    //BEGIN ABI stability stuff
    // NOTICE for ABI stability
    // TODO remove with mandatory version tagging fo 4.9.x or 4.10
    /** @deprecated ABI compatibility only - don't use */
//    case AbilityClientGrouping:
    //END ABI stability stuff
    default: return false;
    }
}

void
Factory::update(WId window, unsigned int changed)
{
    QList<KwinClient *> decos(findChildren<KwinClient *>());
    for (int i = 0; i < decos.count(); ++i)
    {
        KwinClient *d = decos.at(i);
        if (d->windowId() == window)
        {
            if (changed == 64)
                d->updateContBg();
            else
                d->reset(changed);
        }
    }
}
