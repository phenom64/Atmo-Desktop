#include "settings.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QDesktopServices>
#include <QPalette>
#include <QDebug>
#include <QUrl>

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
    "balloontips",
    "palette",
    "animatestack",
    "animatescroll",
    "lockdocks",

    "deco.buttons",
    "deco.icon",
    "deco.shadowsize",
    "deco.framesize",

    "pushbtn.rnd",
    "pushbtn.shadow",
    "pushbtn.gradient",
    "pushbtn.tinthue",

    "toolbtn.rnd",
    "toolbtn.shadow",
    "toolbtn.gradient",
    "toolbtn.tinthue",
    "toolbtn.followcolors",
    "toolbtn.invertactive",
    "toolbtn.flat",

    "input.rnd",
    "input.shadow",
    "input.gradient",
    "input.tinthue",

    "tabs.safari",
    "tabs.rnd",
    "tabs.shadow",
    "tabs.gradient",
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

    "progressbars.shadow",
    "progressbars.rnd",

    "windows.gradient",
    "windows.noisefactor",
    "windows.noisestyle",
    "windows.horizontal",

    "shadows.opacity",
    "shadows.darkraisededges"
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
    false,
    QString(),
    false,
    false,
    false,

    0,
    true,
    32,
    0,

    8,
    3,
    "0.0:5, 1.0:-5",
    "-1:0",

    8,
    3,
    "0.0:5, 1.0:-5",
    "-1:0",
    false,
    false,
    false,

    8,
    3,
    "0.0:-5, 1.0:5",
    "-1:0",

    true,
    4,
    3,
    "0.0:5, 1.0:-5",
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

    3,
    4,

    "0.0:-10, 0.5:10, 1.0:-10",
    0,
    0,
    true,

    33,
    false
};

static const QString confPath()
{
    static const QString cp = QString("%1/.config/dsp").arg(QDir::homePath());
    return cp;
}

static const QString getPreset(QSettings *s, const QString &appName)
{
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


Settings::Settings() : m_settings(0), m_paletteSettings(0)
{
    conf.m_appName = appName();
    if (conf.m_appName == "eiskaltdcpp-qt")
        conf.app = Eiskalt;
    else if (conf.m_appName == "konversation")
        conf.app = Konversation;
    else if (conf.m_appName == "konsole")
        conf.app = Konsole;
    else if (conf.m_appName == "kwin" || conf.m_appName == "kwin_x11" || conf.m_appName == "kwin_wayland")
        conf.app = KWin;
    else if (conf.m_appName == "be.shell")
        conf.app = BEShell;
    else if (conf.m_appName == "yakuake")
        conf.app = Yakuake;
    else if (conf.m_appName == "plasma-desktop")
        conf.app = Plasma;
    else
        conf.app = Unspecific;

    const QDir settingsDir(confPath());
    QString settingsFileName("dsp");
    m_settings = new QSettings(settingsDir.absoluteFilePath(QString("%1.conf").arg(settingsFileName)), QSettings::IniFormat);
    const QString preset(getPreset(m_settings, conf.m_appName));
    const QFileInfo presetFile(settingsDir.absoluteFilePath(QString("%1.conf").arg(preset)));
    if (!preset.isEmpty() && presetFile.exists())
    {
        settingsFileName = preset;
        if (m_settings)
        {
            m_settings->deleteLater();
            m_settings = 0;
        }
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

QSettings
*Settings::settings()
{
    return instance()->m_settings;
}

void
Settings::read()
{
    //globals
    conf.opacity                = 1.0f/*readFloat(Opacity)/100.0f*/;
    conf.blackList              = readStringList(Blacklist);
//    if (conf.blackList.contains(conf.m_appName) || conf.app == KWin)
//        conf.opacity = 1.0f;
    conf.removeTitleBars        = readBool(Removetitle);
    conf.titlePos               = conf.removeTitleBars?readInt(Titlepos):-1;
    conf.hackDialogs            = readBool(Hackdialogs);
    conf.compactMenu            = readBool(Compactmenu);
    conf.splitterExt            = readBool(Splitterext);
    conf.balloonTips            = readBool(Balloontips);
    conf.arrowSize              = readInt(Arrowsize);
    conf.animateStack           = readBool(Animatestack);
    conf.animateScroll          = readBool(Animatescroll);
    conf.lockDocks              = readBool(Lockdocks);
    //deco
    conf.deco.buttons           = readInt(Decobuttons);
    conf.deco.icon              = readBool(Decoicon);
    conf.deco.shadowSize        = readInt(Decoshadowsize);
    conf.deco.frameSize         = readInt(Decoframe);
    //pushbuttons
    conf.pushbtn.rnd            = readInt(Pushbtnrnd);
    conf.pushbtn.shadow         = readInt(Pushbtnshadow);
    conf.pushbtn.gradient       = stringToGrad(readString(Pushbtngrad));
    conf.pushbtn.tint           = tintColor(readString(Pushbtntint));
    //toolbuttons
    conf.toolbtn.rnd            = readInt(Toolbtnrnd);
    conf.toolbtn.shadow         = readInt(Toolbtnshadow);
    conf.toolbtn.gradient       = stringToGrad(readString(Toolbtngrad));
    conf.toolbtn.tint           = tintColor(readString(Toolbtntint));
    conf.toolbtn.folCol         = readBool(Toolbtnfolcol);
    conf.toolbtn.invAct         = readBool(Toolbtninvact);
    conf.toolbtn.flat           = readBool(Toolbtnflat);
    //inputs
    conf.input.rnd              = readInt(Inputrnd);
    conf.input.shadow           = readInt(Inputshadow);
    conf.input.gradient         = stringToGrad(readString(Inputgrad));
    conf.input.tint             = tintColor(readString(Inputtint));
    //tabs
    conf.tabs.safari            = readBool(Tabsaf);
    conf.tabs.rnd               = readInt(Tabrnd);
    conf.tabs.shadow            = readInt(Tabshadow);
    conf.tabs.gradient          = stringToGrad(readString(Tabgrad));
    conf.tabs.safrnd            = qMin(readInt(Saftabrnd), 8);
    conf.tabs.closeButtonSide   = readInt(Tabcloser);
    //uno
    conf.uno.enabled            = readBool(Unoenabled);
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
    //progressbars
    conf.progressbars.shadow    = readInt(Progshadow);
    conf.progressbars.rnd       = readInt(Progrnd);
    //shadows
    conf.shadows.opacity        = readFloat(Shadowopacity)/100.0f;
    conf.shadows.darkRaisedEdges = readBool(Shadowdarkraised);
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
