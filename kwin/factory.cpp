#include <QDBusConnection>
#include <QDBusMessage>
#include <QPainter>
#include <QAbstractEventDispatcher>
#include <QDir>
#include <QSettings>
#include <QMap>
#include <QDebug>

#include "kwinclient.h"
#include "factory.h"
#include "../stylelib/xhandler.h"
#include "../stylelib/shadowhandler.h"

KWIN_DECORATION(Factory)

#if 0
static QMap<QString, DecoData> s_data;

static void addDataForWinClass(const QString &winClass, QSettings &s)
{
    DecoData d;
    d.color[0] = QColor::fromRgba(s.value("fgcolor", "0x00000000").toString().toUInt(0, 16));
    d.color[1] = QColor::fromRgba(s.value("bgcolor", "0x00000000").toString().toUInt(0, 16));
    d.gradient = Settings::stringToGrad(s.value("gradient", "0:10, 1:-10").toString());
    d.noiseRatio = s.value("noise", 20).toUInt();
    d.separator = s.value("separator", true).toBool();
    s_data.insert(winClass, d);
}

static void readWindowData()
{
    s_data.clear();
    static const QString confPath(QString("%1/.config/dsp").arg(QDir::homePath()));
    QSettings s(QString("%1/dspdeco.conf").arg(confPath), QSettings::IniFormat);
    bool hasDefault(false);
    foreach (const QString winClass, s.childGroups())
    {
        s.beginGroup(winClass);
        hasDefault |= (winClass == "default");
        addDataForWinClass(winClass, s);
        s.endGroup();
    }
    if (!hasDefault)
        addDataForWinClass("default", s);
}

DecoData
Factory::decoData(const QString &winClass)
{
    if (s_data.contains(winClass))
        return s_data.value(winClass);
    return s_data.value("default");
}
#endif

KDecoration
*Factory::createDecoration(KDecorationBridge *bridge)
{
    return new KwinClient(bridge, this);
}

Factory *Factory::s_instance = 0;

//Atom Factory::s_wmAtom;
static QAbstractEventDispatcher::EventFilter s_x11eventFilter = 0;

KwinClient
*Factory::deco(unsigned long w)
{
    if (!s_instance)
        return 0;
    QList<KwinClient *> clients = s_instance->findChildren<KwinClient *>();
    for (int i = 0; i < clients.count(); ++i)
    {
        KwinClient *client = clients.at(i);
        if (client->windowId() == w)
            return client;
    }
    return 0;
}
#if 0
bool
Factory::xEventFilter(void *message)
{
    XEvent *xe = static_cast<XEvent *>(message);
    if (!xe)
        return false;
    switch (xe->type)
    {
    case PropertyNotify:
    {
        XPropertyEvent *xpe = static_cast<XPropertyEvent *>(message);
        if (xpe->state != PropertyNewValue)
            break;

        if (xpe->atom == XHandler::xAtom(XHandler::DecoBgPix))
        {
            if (KwinClient *client = deco(xpe->window))
            if (SharedBgPixData *bgPixData = XHandler::getXProperty<SharedBgPixData>(xpe->window, XHandler::DecoBgPix))
            {
                client->setBgPix(bgPixData->bgPix, QSize(bgPixData->w, bgPixData->h));
//                XHandler::deleteXProperty(xpe->window, XHandler::DecoBgPix);
//                XHandler::freePix(bgPixData->bgPix);
                XHandler::freeData(bgPixData);
            }
            return true;
        }
        else if (xpe->atom == XHandler::xAtom(XHandler::WindowData))
        {
            if (KwinClient *client = deco(xpe->window))
            if (WindowData *wd = XHandler::getXProperty<WindowData>(xpe->window, XHandler::WindowData))
            {
                client->setWindowData(*wd);
                XHandler::freeData(wd);
            }
            return true;
        }
        break;
    }
    case ClientMessage:
    {
        if (xe->xclient.message_type == XHandler::xAtom(XHandler::Repaint))
        {
            if (KwinClient *client = deco(xe->xclient.window))
                client->update();
            return true;
        }
        break;
    }
    default: break;
    }
    return s_x11eventFilter&&(*s_x11eventFilter)(message);
}
#endif

Factory::Factory()
    : QObject()
    , KDecorationFactory()
{
//    if (!s_x11eventFilter && !s_instance)
//        s_x11eventFilter = QAbstractEventDispatcher::instance()->setEventFilter(&Factory::xEventFilter);
//    s_instance = this;
//    readWindowData();

//    QString string = QString("_NET_WM_CM_S%1").arg(DefaultScreen(QX11Info::display()));
//    s_wmAtom = XInternAtom(QX11Info::display(), string.toAscii().data(), False);
    DSP::ShadowHandler::removeDelete();
}

Factory::~Factory()
{
    DSP::ShadowHandler::removeDelete();
//    if (this == s_instance)
//    {
//        s_instance = 0;
//        QAbstractEventDispatcher::instance()->setEventFilter(s_x11eventFilter);
//        s_x11eventFilter = 0;
//    }
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
