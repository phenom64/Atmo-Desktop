#include "settings.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QDesktopServices>
#include <QPalette>
#include <QDebug>
#include <QUrl>
#include "../namespace.h"

using namespace DSP;

Settings::Conf Settings::conf;
Settings *Settings::s_instance(0);

static const char *s_key[] = {
    "opacity",
    "blacklist",
    "removetitlebars",
    "titlepos",
    "hackdialogs",
    "compactmenu",
    "splitterext",
    "maxarrowsize",
    "simplearrows",
    "balloontips",
    "palette",
    "animatestack",
    "animatescroll",
    "lockdocks",
    "differentinactive",

    "deco.buttons",
    "deco.icon",
    "deco.shadowsize",
    "deco.framesize",
    "deco.embedded",
    "deco.mincolor",
    "deco.maxcolor",
    "deco.closecolor",

    "pushbtn.rnd",
    "pushbtn.shadow",
    "pushbtn.gradient",
    "pushbtn.tinthue",

    "toolbtn.rnd",
    "toolbtn.shadow",
    "toolbtn.gradient",
    "toolbtn.activegradient",
    "toolbtn.tinthue",
    "toolbtn.followcolors",
    "toolbtn.invertactive",
    "toolbtn.flat",

    "input.rnd",
    "input.inunornd",
    "input.shadow",
    "input.gradient",
    "input.tinthue",

    "tabs.docstyle",
    "tabs.selectors",
    "tabs.safari",
    "tabs.rnd",
    "tabs.shadow",
    "tabs.gradient",
    "tabs.bargrad",
    "tabs.safrnd",          //safaritabs roundness capped at 8 atm, might change in the future if needed
    "tabs.closebuttonside",

    "uno",
    "uno.gradient",
    "uno.tinthue",
    "uno.noisefactor",
    "uno.noisestyle",
    "uno.horizontal",
    "uno.contentaware",
    "uno.contentopacity",
    "uno.contentblurradius",

    "menues.icons",
    "menues.gradient",
    "menues.itemgradient",
    "menues.itemshadow",

    "sliders.size",
    "sliders.dot",
    "sliders.slidergradient",
    "sliders.groovegradient",
    "sliders.grooveshadow",
    "sliders.fillgroove",
    "sliders.groovestyle",

    "scrollers.size",
    "scrollers.style",
    "scrollers.slidergradient",
    "scrollers.groovegradient",
    "scrollers.groovestyle",
    "scrollers.grooveshadow",

    "views.treelines",
    "views.itemgradient",
    "views.itemshadow",
    "views.itemrnd",
    "views.headergradient",

    "progressbars.shadow",
    "progressbars.rnd",
    "progressbars.textonlyonhover",
    "progressbars.textpos",
    "progressbars.gradient",
    "progressbars.stripesize",

    "windows.gradient",
    "windows.noisefactor",
    "windows.noisestyle",
    "windows.horizontal",

    "shadows.opacity",
    "shadows.illuminationopacity",
    "shadows.darkraisededges",
    "shadows.ontextopacity"
};

static const QVariant s_default[] = {
    100,
    "smplayer",
    false,
    1,
    false,
    false,
    false,
    9,
    true,
    false,
    QString(),
    false,
    false,
    false,
    false,

    0,
    true,
    32,
    0,
    false,
    "0xFFFFC05E",
    "0xFF88EB51",
    "0xFFF98862",

    8,
    3,
    "0.0:5, 1.0:-5",
    "-1:0",

    8,
    3,
    "0.0:5, 1.0:-5",
    "0.0:-5, 1.0:5",
    "-1:0",
    false,
    false,
    false,

    8,
    8,
    3,
    "0.0:-5, 1.0:5",
    "-1:0",

    0,
    true,
    true,
    4,
    3,
    "0.0:5, 1.0:-5",
    "0.0:-5, 1.0:5",
    4,
    0,

    true,
    "0.0:5, 1.0:-5",
    "-1:0",
    5,
    0,
    false,
    QStringList(),
    10,
    2,

    false,
    "0:5, 1:-5",
    "0.0:5, 1.0:-5",
    0,

    16,
    true,
    "0.0:5, 1.0:-5",
    "0.0:-5, 1.0:5",
    0,
    false,
    0,

    12,
    0,
    "0.0:5, 1.0:-5",
    "0.0:5, 0.5:-5, 1.0:5",
    0,
    0,

    true,
    "0:5, 1:-5",
    0,
    6,
    "0:5, 1:-5",

    3,
    4,
    false,
    0,
    "0:5, 1:-5",
    32,

    "0.0:-10, 0.5:10, 1.0:-10",
    0,
    0,
    true,

    33,
    33,
    false,
    50
};

static const char *s_description[] = {
    /*"opacity"*/                   "Opacity for UNO parts, *NOT* enabled atm due to problems w/ Qt5",
    /*"blacklist"*/                 "Blacklist of apps that should not get opacity if set, mostly media players should be here",
    /*"removetitlebars"*/           "Hack for removing titlebars from windows in order to get the look of Mac Os Yosemite",
    /*"titlepos"*/                  "Position of the title in the toolbar (if room for it at all), 0 = Left, 1 = Center, 2 = Right",
    /*"hackdialogs"*/               "Hack for making the dialogs act more like they do in Mac Os, appearing under the titlebar or toolbar of the parent window, also shapes away the titlebar of the dialog",
    /*"compactmenu"*/               "Hides the menubar and adds a button in the toolbar to popup the menu, if there is a toolbar to add it to",
    /*"splitterext"*/               "Splitters are small in this style (1px height/width), this adds an invisible area outside the splitter temporarily when you hover a splitter",
    /*"maxarrowsize"*/              "Maximum allowed size in pixels for any arrows thats drawn",
    /*"simpleArrows"*/              "Use, simple triangles as arrows.",
    /*"balloontips"*/               "Draw tooltips as comic a like balloons",
    /*"palette"*/                   "Palette to be used (filename, no suffix), this should only be used in presets, not directly in dsp.conf",
    /*"animatestack"*/              "Animate when the topmost widget in a stack changes, ie: when the active tab changes",
    /*"animatescroll"*/             "Smooth scrolling globally, known to cause trouble in certain cases, mainly dolphin",
    /*"lockdocks"*/                 "Locks the docks, removes the titlebar from them, cant float or close. Toggles w/ Ctrl+Alt+D",
    /*"differentinactive"*/         "Makes the UNO part of inactive windows shaded, a'la Mac Os, also, if the toolbuttons are set to Yosemite shadow style, this changes the toolbutton appearance for inactive windows slightly",

    /*"deco.buttons"*/              "Style of the Min|Max|Close buttons, Sunken = 0, Etched = 1, Raised = 2, Yosemite = 3, Carved = 4, Rect = 5",
    /*"deco.icon"*/                 "Wheter or not the deco client should paint an icon in the titlebar",
    /*"deco.shadowsize"*/           "Size of the windowshadow for active window, inactive windows will have a smaller",
    /*"deco.framesize"*/            "Size of borders for the decoration, not yet implemented in the kde5 deco",
    /*"deco.embedded"*/             "CSD-a-like bullshit that embeds crap in the title area, will cause trouble, do not use",
    /*"deco.mincolor"*/             "Color of minimize button 0xAARRGGBB",
    /*"deco.maxcolor"*/             "Color of maximize button 0xAARRGGBB",
    /*"deco.closecolor"*/           "Color of close button 0xAARRGGBB",

    /*"pushbtn.rnd"*/               "Roundness of normal pushbuttons",
    /*"pushbtn.shadow"*/            "Shadow of normal pushbuttons",
    /*"pushbtn.gradient"*/          "Gradient of normal pushbuttons",
    /*"pushbtn.tinthue"*/           "Hue to tint normal pushbuttons w/",

    /*"toolbtn.rnd"*/               "Roundness of toolbuttons",
    /*"toolbtn.shadow"*/            "Shadow of toolbuttons",
    /*"toolbtn.gradient"*/          "Gradient of toolbuttons",
    /*"toolbtn.activegradient"*/    "Gradient on active/selected toolbuttons",
    /*"toolbtn.tinthue"*/           "Hue to tint toolbuttons w/",
    /*"toolbtn.followcolors"*/      "If the icons on toolbuttons should be manipulated to be monochromatic and follow the palette (highly experimental and does not always produce nice results)",
    /*"toolbtn.invertactive"*/      "Whether the active/checked toolbuttons should have inverted foreground/background colors a'la Mac Os",
    /*"toolbtn.flat"*/              "Windows alike toolbuttons thats just icons and/or text",

    /*"input.rnd"*/                 "Roundness of input boxes, lineedits and spinboxes and such",
    /*"input.inunornd"*/            "Roundness of inputs inside UNO",
    /*"input.shadow"*/              "Shadow of input boxes, lineedits and spinboxes and such",
    /*"input.gradient"*/            "Gradient of input boxes, lineedits and spinboxes and such",
    /*"input.tinthue"*/             "Hue to tint input boxes, lineedits and spinboxes and such w/",

    /*"tabs.docstyle"*/             "Style / shape of document mode tabbar tabs, 0 = Chrome, 1 = Simple",
    /*"tabs.selectors"*/            "Use selectors instead of tabs on tabwidgets",
    /*"tabs.safari"*/               "Integrate the tabbars under toolbars like the Safari web browser from Mac Os does",
    /*"tabs.rnd"*/                  "Roundness of tabs",
    /*"tabs.shadow"*/               "Shadow of tabs",
    /*"tabs.gradient"*/             "Gradient of tabs",
    /*"tabs.bargrad"*/              "Gradient on document mode tabbars",
    /*"tabs.safrnd"*/               "Roundness of tabs in a safari-like tabbar. Max roundness allowed 8",          //safaritabs roundness capped at 8 atm, might change in the future if needed
    /*"tabs.closebuttonside"*/      "Side of the tab the close button should be on, 0 = Left, 1 = Right",

    /*"uno"*/                       "If the head of the window should be integrated into one area a'la Mac Os. Always enabled atm",
    /*"uno.gradient"*/              "Gradient of the UNO area",
    /*"uno.tinthue"*/               "Hue to tint the UNO area w/",
    /*"uno.noisefactor"*/           "How much noise the UNO area should have",
    /*"uno.noisestyle"*/            "Style of the noise on the UNO area, 0 = Generic, 1 = Brushed Metal",
    /*"uno.horizontal"*/            "Whether the UNO area gradient should be horizontal instead of Vertical, Always disabled atm",
    /*"uno.contentaware"*/          "Yosemite alike content aware toolbars, *very* experimental and expensive, veeery fast cpus/gfx cards should be fine",
    /*"uno.contentopacity"*/        "Opacity of the content painted in the toolbar",
    /*"uno.contentblurradius"*/     "Amount of blur applied to the content in the toolbar",

    /*"menues.icons"*/              "Show icons in menues",
    /*"menues.gradient"*/           "Gradient on menues",
    /*"menues.itemgradient"*/       "Gradient on menuitems",
    /*"menues.itemshadow"*/         "Shadow on menuitems",

    /*"sliders.size"*/              "Size of sliders",
    /*"sliders.dot"*/               "Paint a dot in the middle of the slider handles, like Mac Os pre Yosemite",
    /*"sliders.slidergradient"*/    "Gradient of sliderhandles",
    /*"sliders.groovegradient"*/    "Gradient of slidergrooves",
    /*"sliders.grooveshadow"*/      "Shadow of slidergrooves",
    /*"sliders.fillgroove"*/        "Fill up the groove like a progressbar to where the slider is w/ the highight color",
    /*"sliders.groovestyle"*/       "How to fill groove section of a slider, 0 = Window colored, 1 = Blend of WindowText and Window color, 2 = WindowText color",

    /*"scrollers.size"*/            "Size of scrollbars",
    /*"scrollers.style"*/           "Style of scrollbars, 0 = Yosemite alike, 1 = Pre Yosemite alike",
    /*"scrollers.slidergradient"*/  "Gradient of the slider in scrollbars",
    /*"scrollers.groovegradient"*/  "Gradient of the groove part of scrollbars, only read if style of scrollbars set to 1 (Pre Yosemite alike)",
    /*"scrollers.groovestyle"*/     "How to fill groove section of a scrollbar, 0 = Window colored, 1 = Blend of WindowText and Window color, 2 = WindowText color",
    /*"scrollers.grooveshadow"*/    "Shadow for the groove in scrollbars to be used, only read if style of scrollbars set to 1 (Pre Yosemite alike)",

    /*"views.treelines"*/           "Draw the branches in the treeviews",
    /*"views.itemgradient"*/        "Gradient on viewitems",
    /*"views.itemshadow"*/          "Shadow on viewitems",
    /*"views.itemrnd"*/             "Roundness of viewitems",
    /*"views.headergradient"*/      "Gradient on headers in views",

    /*"progressbars.shadow"*/       "Shadows for progressbars",
    /*"progressbars.rnd"*/          "Roundness for progressbars",
    /*"progressbars.txtHover"*/     "Text only on hover for progressbars",
    /*"progressbars.textPos"*/      "Text position for progressbars, 0 = Center (default), 1 = Left of progressbarcontents, 2 = Right of progressbarcontents",
    /*"progressbars.gradient"*/     "Gradient on progressbars",
    /*"progressbars.stripesize"*/   "Size of stripes on progressbars",

    /*"windows.gradient"*/          "Gradient for windows if UNO not enabled, not used atm",
    /*"windows.noisefactor"*/       "How much noise to use on the window background, not used atm",
    /*"windows.noisestyle"*/        "Style of the noise painted on the window background, 0 = Generic, 1 = Brushed Metal, not used atm",
    /*"windows.horizontal"*/        "Whether the gradient set on windows should be horizontal instead of vertical, not used atm",

    /*"shadows.opacity"*/           "Opacity of the shadows painted on widgets",
    /*"shadows.illumination"*/      "Opacity of the illuminated(light) parts of the shadows",
    /*"shadows.darkraisededges"*/   "Whether widgets w/ a raised shadow should be darker around the left/right edges",
    /*"shadows.ontextopacity"*/     "Opacity of the textshadow/textbevel, 0 to disable (faster)"
};

Settings::Key
Settings::key(const QString k)
{
    for (int i = 0; i < Keycount; ++i)
        if (s_key[i] == k.toLower())
            return (Key)i;
    return Invalid;
}

const char
*Settings::key(const Key k)
{
    return s_key[k];
}

const QVariant
Settings::defaultValue(const Key k)
{
    return s_default[k];
}

static const QString confPath()
{
    static const QString cp = QString("%1/.config/dsp").arg(QDir::homePath());
    return cp;
}

static const QString getPreset(QSettings *s, const QString &appName)
{
    if (qApp->arguments().contains("--dsppreset"))
        if (int i = qApp->arguments().indexOf("--dsppreset")+1)
            if (i < qApp->arguments().count())
                return qApp->arguments().at(i);

    QString preset;
    s->beginGroup("Presets");
    const QStringList presets(s->childKeys());
    const int count(presets.count());
    for (int i = 0; i < count; ++i)
    {
        const QStringList &apps(s->value(presets.at(i), QStringList()).toStringList());
        if (apps.contains(appName, Qt::CaseInsensitive))
        {
            preset = presets.at(i);
            break;
        }
    }
    s->endGroup();
    return preset;
}

static const QString appName()
{
    QString app;
    if (qApp)
    {
        if (!qApp->arguments().isEmpty())
            app = qApp->arguments().first();
        else if (!qApp->applicationName().isEmpty())
            app = qApp->applicationName();
        else
            app = QFileInfo(qApp->applicationFilePath()).fileName();
    }
    if (app.contains("/"))
        app = QFileInfo(app).fileName();
    return app;
}


Settings::Settings() : m_settings(0), m_paletteSettings(0), m_overrideSettings(0)
{
    conf.appName = appName();
    if (conf.appName == "eiskaltdcpp-qt")
        conf.app = Eiskalt;
    else if (conf.appName == "konversation")
        conf.app = Konversation;
    else if (conf.appName == "konsole")
        conf.app = Konsole;
    else if (conf.appName == "kwin" || conf.appName == "kwin_x11" || conf.appName == "kwin_wayland")
        conf.app = KWin;
    else if (conf.appName == "be.shell")
        conf.app = BEShell;
    else if (conf.appName == "yakuake")
        conf.app = Yakuake;
    else if (conf.appName == "plasma-desktop")
        conf.app = Plasma;
    else if (conf.appName == "dfm")
        conf.app = DFM;
    else if (conf.appName == "systemsettings")
        conf.app = SystemSettings;
    else
        conf.app = Unspecific;

    const QDir settingsDir(confPath());
    QString settingsFileName("dsp");
    m_settings = new QSettings(settingsDir.absoluteFilePath(QString("%1.conf").arg(settingsFileName)), QSettings::IniFormat);
    const QString preset(getPreset(m_settings, conf.appName));
    const QFileInfo presetFile(settingsDir.absoluteFilePath(QString("%1.conf").arg(preset)));
    if (!preset.isEmpty())
    {
        if (presetFile.exists())
        {
            settingsFileName = preset;
            if (m_settings)
            {
                m_settings->deleteLater();
                m_settings = 0;
            }
        }
        else
            qDebug() << "DSP: unable to load preset or preset doesnt exist:" << preset;
    }
    if (!m_settings)
        m_settings = new QSettings(settingsDir.absoluteFilePath(QString("%1.conf").arg(settingsFileName)), QSettings::IniFormat);
//    QObject::connect(qApp, SIGNAL(aboutToQuit()), m_settings, SLOT(deleteLater()));

    const QString paletteFileName = m_settings->value(s_key[Palette], s_default[Palette]).toString();
    if (!paletteFileName.isEmpty())
    {
        QString settingsPath(confPath());
        settingsPath.append(QString("/%1.conf").arg(paletteFileName));
        m_paletteSettings = new QSettings(settingsPath, QSettings::IniFormat);
//        QObject::connect(qApp, SIGNAL(aboutToQuit()), m_paletteSettings, SLOT(deleteLater()));
    }
}

Settings::~Settings()
{
    s_instance = 0;
    m_settings = 0;
    m_paletteSettings = 0;
    restoreFileName();
}

Settings
*Settings::instance()
{
    if (!s_instance)
        s_instance = new Settings();
    return s_instance;
}

Gradient
Settings::stringToGrad(const QString &string)
{
    const QStringList pairs(string.split(",", QString::SkipEmptyParts));
    Gradient gradient;
    if (pairs.isEmpty())
        return gradient;
    for (int i = 0; i < pairs.count(); ++i)
    {
        QStringList pair(pairs.at(i).split(":"));
        if (pair.count() != 2)
            break;
        bool ok;
        float stop(pair.first().toFloat(&ok));
        if (!ok)
            break;
        int val(pair.last().toInt(&ok));
        if (!ok)
            break;
        gradient << GradientStop(stop, val);
    }
    return gradient;
}

QGradientStop
Settings::pairToStop(const GradientStop pair, const QColor &c)
{
    QGradientStop stop;
    stop.first = pair.first;
    const int val(qAbs(pair.second));
    stop.second = pair.second==0?c:(pair.second>0?c.lighter(100+val):c.darker(100+val));
    return stop;
}

QGradientStops
Settings::gradientStops(const Gradient pairs, const QColor &c)
{
    QGradientStops stops;
    for (int i = 0; i < pairs.count(); ++i)
        stops << pairToStop(pairs.at(i), c);
    return stops;
}

static Tint tintColor(const QString &tint)
{
    const QStringList pair(tint.split(":"));
    Tint c;
    if (pair.count() != 2)
        return c;
    c.second = pair.last().toInt();
    if (c.second > -1 && c.second < 360)
        c.first.setHsv(pair.first().toInt(), 255, 255);
    return c;
}

static const char *groups[QPalette::NColorGroups] =
{
    "activePalette",
    "disabledPalette",
    "inactivePalette"
};
static const char *roles[QPalette::NColorRoles] =
{
    "WindowText", "Button", "Light", "Midlight", "Dark", "Mid",
    "Text", "BrightText", "ButtonText", "Base", "Window", "Shadow",
    "Highlight", "HighlightedText",
    "Link", "LinkVisited", // ### Qt 5: remove
    "AlternateBase",
    "NoRole", // ### Qt 5: value should be 0 or -1
    "ToolTipBase", "ToolTipText"
    //                         NColorRoles = ToolTipText + 1,
    //                         Foreground = WindowText, Background = Window // ### Qt 5: remove
};

void
Settings::writePaletteColor(QPalette::ColorGroup g, QPalette::ColorRole r, QColor c)
{
    const QString &color = QString::number(c.rgba(), 16);
    paletteSettings()->beginGroup(groups[g]);
    paletteSettings()->setValue(roles[r], color);
    paletteSettings()->endGroup();
}

QColor
Settings::readPaletteColor(QPalette::ColorGroup g, QPalette::ColorRole r)
{
    paletteSettings()->beginGroup(groups[g]);
    const QString color = paletteSettings()->value(roles[r], QString()).toString();
    QColor c;
    if (color.size() == 8)
        c = QColor::fromRgba(color.toUInt(0, 16));
    paletteSettings()->endGroup();
    return c;
}

void
Settings::writePalette()
{
    QSettings *s = paletteSettings();
    if (!s || QFileInfo(s->fileName()).exists())
        return;
    const QPalette pal(QApplication::palette());
    for (int g = 0; g < QPalette::NColorGroups; ++g)
        for (int r = 0; r < QPalette::NColorRoles; ++r)
            writePaletteColor((QPalette::ColorGroup)g, (QPalette::ColorRole)r, pal.color((QPalette::ColorGroup)g, (QPalette::ColorRole)r));
}

void
Settings::readPalette()
{
    if (!paletteSettings())
        return;
    for (int g = 0; g < QPalette::NColorGroups; ++g)
    {
        for (int r = 0; r < QPalette::NColorRoles; ++r)
        {
            const QColor c = readPaletteColor((QPalette::ColorGroup)g, (QPalette::ColorRole)r);
            if (c.isValid())
            {
                if (!conf.palette)
                    conf.palette = new QPalette();
            }
            else
            {
                if (conf.palette)
                {
                    delete conf.palette;
                    conf.palette = 0;
                }
                return;
            }
            conf.palette->setColor((QPalette::ColorGroup)g, (QPalette::ColorRole)r, c);
        }
    }
}

QSettings
*Settings::paletteSettings()
{
    return instance()->m_paletteSettings;
}

void
Settings::setFileName(const QString &file)
{
    if (instance()->m_overrideSettings)
        restoreFileName();
    instance()->m_overrideSettings = new QSettings(QString("%1/%2.conf").arg(confPath(), file), QSettings::IniFormat);
}

void
Settings::restoreFileName()
{
    if (instance()->m_overrideSettings)
    {
        instance()->m_overrideSettings->sync();
        instance()->m_overrideSettings->deleteLater();
        instance()->m_overrideSettings = 0;
    }
}

void
Settings::writeVal(const Key k, const QVariant v)
{
    if (!settings())
        return;
    settings()->setValue(s_key[k], v);
}

QVariant
Settings::readVal(const Key k)
{
    if (!settings())
        return QVariant();
    return settings()->value(s_key[k], s_default[k]);
}

const char *
Settings::description(const Key k)
{
    return s_description[k];
}

QSettings
*Settings::settings()
{
    return instance()->m_overrideSettings?instance()->m_overrideSettings:instance()->m_settings;
}

void
Settings::read()
{
    conf.baseSize               = 24;
    //globals
    conf.opacity                = 1.0f/*readFloat(Opacity)/100.0f*/;
    conf.blackList              = readStringList(Blacklist);
//    if (conf.blackList.contains(conf.m_appName) || conf.app == KWin)
//        conf.opacity = 1.0f;
    conf.removeTitleBars        = false/*readBool(Removetitle)*/;
    conf.titlePos               = readInt(Titlepos);
    conf.hackDialogs            = readBool(Hackdialogs);
    conf.compactMenu            = readBool(Compactmenu);
    conf.splitterExt            = readBool(Splitterext);
    conf.balloonTips            = readBool(Balloontips);
    conf.arrowSize              = readInt(Arrowsize);
    conf.simpleArrows           = readBool(Simplearrows);
    conf.animateStack           = readBool(Animatestack);
    conf.animateScroll          = readBool(Animatescroll);
    conf.lockDocks              = readBool(Lockdocks);
    conf.differentInactive      = readBool(Differentinactive);
    //deco
    conf.deco.buttons           = readInt(Decobuttons);
    conf.deco.icon              = readBool(Decoicon);
    conf.deco.shadowSize        = readInt(Decoshadowsize);
    conf.deco.frameSize         = readInt(Decoframe);
    conf.deco.embed             = readBool(Decoembedded);
    conf.deco.min               = readString(Decomincolor).toUInt(0, 16);
    conf.deco.max               = readString(Decomaxcolor).toUInt(0, 16);
    conf.deco.close             = readString(Decoclosecolor).toUInt(0, 16);
    //pushbuttons
    conf.pushbtn.rnd            = qMin<quint8>(MaxRnd, readInt(Pushbtnrnd));
    conf.pushbtn.shadow         = readInt(Pushbtnshadow);
    conf.pushbtn.gradient       = stringToGrad(readString(Pushbtngrad));
    conf.pushbtn.tint           = tintColor(readString(Pushbtntint));
    //toolbuttons
    conf.toolbtn.rnd            = qMin<quint8>(MaxRnd, readInt(Toolbtnrnd));
    conf.toolbtn.shadow         = readInt(Toolbtnshadow);
    conf.toolbtn.gradient       = stringToGrad(readString(Toolbtngrad));
    conf.toolbtn.activeGradient = stringToGrad(readString(Toolbtnactivegrad));
    conf.toolbtn.tint           = tintColor(readString(Toolbtntint));
    conf.toolbtn.folCol         = readBool(Toolbtnfolcol);
    conf.toolbtn.invAct         = readBool(Toolbtninvact);
    conf.toolbtn.flat           = readBool(Toolbtnflat);
    //inputs
    conf.input.rnd              = qMin<quint8>(MaxRnd, readInt(Inputrnd));
    conf.input.unoRnd           = qMin<quint8>(MaxRnd, readInt(Inputunornd));
    conf.input.shadow           = readInt(Inputshadow);
    conf.input.gradient         = stringToGrad(readString(Inputgrad));
    conf.input.tint             = tintColor(readString(Inputtint));
    //tabs
    conf.tabs.docStyle          = readInt(Tabdocstyle);
    conf.tabs.safari            = readBool(Tabsaf);
    conf.tabs.regular           = !readBool(Tabselectors);
    conf.tabs.rnd               = qMin<quint8>(MaxRnd, readInt(Tabrnd));
    conf.tabs.shadow            = readInt(Tabshadow);
    conf.tabs.gradient          = stringToGrad(readString(Tabgrad));
    conf.tabs.barGrad           = stringToGrad(readString(Tabbargrad));
    conf.tabs.safrnd            = qMin(readInt(Saftabrnd), 8);
    conf.tabs.closeButtonSide   = readInt(Tabcloser);
    //uno
    conf.uno.enabled            = /*readBool(Unoenabled)*/true;
    conf.uno.gradient           = stringToGrad(readString(Unograd));
    conf.uno.tint               = tintColor(readString(Unotint));
    conf.uno.noise              = readInt(Unonoise);
    conf.uno.noiseStyle         = readInt(Unonoisestyle);
    conf.uno.hor                = readBool(Unohor);
    conf.uno.contAware          = readStringList(Unocont).contains(QFileInfo(qApp->applicationFilePath()).fileName());
    conf.uno.opacity            = readFloat(Unoopacity)/100.0f;
    conf.uno.blur               = readInt(Unocontblur);
    //windows when not uno
    conf.windows.gradient       = stringToGrad(readString(Wingrad));
    conf.windows.noise          = readInt(Winnoise);
    conf.windows.noiseStyle     = readInt(Winnoisestyle);
    conf.windows.hor            = readBool(Winhor);
    //menues
    conf.menues.icons           = readBool(Menuicons);
    conf.menues.gradient        = stringToGrad(readString(Menugrad));
    conf.menues.itemGradient    = stringToGrad(readString(Menuitemgrad));
    conf.menues.itemShadow      = readInt(Menuitemshadow);
    //sliders
    conf.sliders.size           = readInt(Slidersize);
    conf.sliders.dot            = readBool(Sliderdot);
    conf.sliders.grooveShadow   = readInt(Slidergrooveshadow);
    conf.sliders.grooveGrad     = stringToGrad(readString(Slidergroove));
    conf.sliders.sliderGrad     = stringToGrad(readString(Slidergrad));
    conf.sliders.fillGroove     = readBool(Sliderfillgroove);
    conf.sliders.grooveStyle    = readInt(Sliderinvert);
    //scrollers
    conf.scrollers.size         = readInt(Scrollersize);
    conf.scrollers.style        = readInt(Scrollerstyle);
    conf.scrollers.grooveGrad   = stringToGrad(readString(Scrollergroove));
    conf.scrollers.sliderGrad   = stringToGrad(readString(Scrollergrad));
    conf.scrollers.grooveStyle  = readInt(Scrollerinvert);
    conf.scrollers.grooveShadow = readInt(Scrollergshadow);
    //views
    conf.views.treelines        = readBool(Viewtreelines);
    conf.views.itemGradient     = stringToGrad(readString(Viewitemgrad));
    conf.views.itemShadow       = readInt(Viewitemshadow);
    conf.views.itemRnd          = readInt(Viewitemrnd);
    conf.views.headerGradient   = stringToGrad(readString(Viewheadergrad));
    //progressbars
    conf.progressbars.shadow    = readInt(Progshadow);
    conf.progressbars.rnd       = qMin<quint8>(MaxRnd, readInt(Progrnd));
    conf.progressbars.txtHover  = readBool(Progtxthover);
    conf.progressbars.textPos   = readInt(Progtxtpos);
    conf.progressbars.gradient  = stringToGrad(readString(Proggrad));
    conf.progressbars.stripeSize= readInt(Progstripe);
    //shadows
    conf.shadows.opacity        = readInt(Shadowopacity)*2.55f;
    conf.shadows.illumination   = readInt(Shadowillumination)*2.55f;
    conf.shadows.darkRaisedEdges= readBool(Shadowdarkraised);
    conf.shadows.onTextOpacity  = readInt(Shadowontextopacity)*2.55f;
    readPalette();
}

void
Settings::writeDefaults()
{
    if (!settings())
        return;

    qDebug() << "attempting to write default values...";
    for (int i = 0; i < Keycount; ++i)
        settings()->setValue(s_key[i], s_default[i]);
    settings()->sync();
}

void
Settings::edit()
{
    if (settings())
    {
        qDebug() << "trying to open file" << settings()->fileName() << "in your default text editor.";
        QDesktopServices::openUrl(QUrl::fromLocalFile(settings()->fileName()));
    }
}
