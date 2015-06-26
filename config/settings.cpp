#include "settings.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QDesktopServices>
#include <QPalette>
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QUrl>

QSettings *Settings::s_settings(0);
QSettings *Settings::s_paletteSettings(0);
Settings::Conf Settings::conf;
bool Settings::s_isValid(false);

static const QString confPath = QString("%1/.config/dsp").arg(QDir::homePath());

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
    s_paletteSettings->beginGroup(groups[g]);
    s_paletteSettings->setValue(roles[r], color);
    s_paletteSettings->endGroup();
}

QColor
Settings::readPaletteColor(QPalette::ColorGroup g, QPalette::ColorRole r)
{
    s_paletteSettings->beginGroup(groups[g]);
    const QString color = s_paletteSettings->value(roles[r], QString()).toString();
    QColor c;
    if (color.size() == 8)
        c = QColor::fromRgba(color.toUInt(0, 16));
    s_paletteSettings->endGroup();
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
    if (s_paletteSettings)
        return s_paletteSettings;
    if (!s_settings)
        return 0;
    const QString paletteFileName(s_settings->value(READPALETTE).toString());
    if (paletteFileName.isEmpty())
        return 0;

    QString settingsPath(confPath);
    settingsPath.append(QString("/%1.conf").arg(paletteFileName));

    s_paletteSettings = new QSettings(settingsPath, QSettings::NativeFormat);
    return s_paletteSettings;
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

void
Settings::initiate()
{
    if (s_settings)
    {
        s_settings->deleteLater();
        s_settings = 0;
    }
    if (s_paletteSettings)
    {
        s_paletteSettings->deleteLater();
        s_paletteSettings = 0;
    }

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

    const QDir settingsDir(confPath);
    QString settingsFileName("dsp");
    s_settings = new QSettings(settingsDir.absoluteFilePath(QString("%1.conf").arg(settingsFileName)), QSettings::NativeFormat);
    const QString preset(getPreset(s_settings, conf.m_appName));
    const QFileInfo presetFile(settingsDir.absoluteFilePath(QString("%1.conf").arg(preset)));
    if (!preset.isEmpty() && presetFile.exists())
    {
        settingsFileName = preset;
        if (s_settings)
        {
            s_settings->deleteLater();
            s_settings = 0;
        }
    }
    if (!s_settings)
        s_settings = new QSettings(settingsDir.absoluteFilePath(QString("%1.conf").arg(settingsFileName)), QSettings::NativeFormat);
}

void
Settings::read()
{
    Settings::initiate();
#define READINT(_VAR_) s_settings->value(_VAR_).toInt()
#define READBOOL(_VAR_) s_settings->value(_VAR_).toBool()
#define READFLOAT(_VAR_) s_settings->value(_VAR_).toFloat()
#define READSTRING(_VAR_) s_settings->value(_VAR_).toString()
#define READSTRINGLIST(_VAR_) s_settings->value(_VAR_).toStringList()
    //globals
    conf.opacity                = 1.0f/*READFLOAT(READOPACITY)/100.0f*/;
    conf.blackList              = READSTRINGLIST(READBLACKLIST);
//    if (conf.blackList.contains(conf.m_appName) || conf.app == KWin)
//        conf.opacity = 1.0f;
    conf.removeTitleBars        = READBOOL(READREMOVETITLE);
    conf.titlePos               = conf.removeTitleBars?READINT(READTITLEPOS):-1;
    conf.hackDialogs            = READBOOL(READHACKDIALOGS);
    conf.compactMenu            = READBOOL(READCOMPACTMENU);
    conf.splitterExt            = READBOOL(READSPLITTEREXT);
    conf.balloonTips            = READBOOL(READBALLOONTIPS);
    conf.arrowSize              = READINT(READARROWSIZE);
    conf.animateStack           = READBOOL(READANIMATESTACK);
    //deco
    conf.deco.buttons           = READINT(READDECOBUTTONS);
    conf.deco.icon              = READBOOL(READDECOICON);
    conf.deco.shadowSize        = READINT(READDECOSHADOWSIZE);
    conf.deco.frameSize         = READINT(READDECOFRAME);
    //pushbuttons
    conf.pushbtn.rnd            = READINT(READPUSHBTNRND);
    conf.pushbtn.shadow         = READINT(READPUSHBTNSHADOW);
    conf.pushbtn.gradient       = stringToGrad(READSTRING(READPUSHBTNGRAD));
    conf.pushbtn.tint           = tintColor(READSTRING(READPUSHBTNTINT));
    //toolbuttons
    conf.toolbtn.rnd            = READINT(READTOOLBTNRND);
    conf.toolbtn.shadow         = READINT(READTOOLBTNSHADOW);
    conf.toolbtn.gradient       = stringToGrad(READSTRING(READTOOLBTNGRAD));
    conf.toolbtn.tint           = tintColor(READSTRING(READTOOLBTNTINT));
    conf.toolbtn.folCol         = READBOOL(READTOOLBTNFOLCOL);
    conf.toolbtn.invAct         = READBOOL(READTOOLBTNINVACT);
    conf.toolbtn.flat           = READBOOL(READTOOLBTNFLAT);
    //inputs
    conf.input.rnd              = READINT(READINPUTRND);
    conf.input.shadow           = READINT(READINPUTSHADOW);
    conf.input.gradient         = stringToGrad(READSTRING(READINPUTGRAD));
    conf.input.tint             = tintColor(READSTRING(READINPUTTINT));
    //tabs
    conf.tabs.safari            = READBOOL(READTABSAF);
    conf.tabs.rnd               = READINT(READTABRND);
    conf.tabs.shadow            = READINT(READTABSHADOW);
    conf.tabs.gradient          = stringToGrad(READSTRING(READTABGRAD));
    conf.tabs.safrnd            = qMin(READINT(READSAFTABRND), 8);
    conf.tabs.closeButtonSide   = READINT(READTABCLOSER);
    //uno
    conf.uno.enabled            = READBOOL(READUNOENABLED);
    conf.uno.gradient           = stringToGrad(READSTRING(READUNOGRAD));
    conf.uno.tint               = tintColor(READSTRING(READUNOTINT));
    conf.uno.noise              = READINT(READUNONOISE);
    conf.uno.noiseStyle         = READINT(READUNONOISESTYLE);
    conf.uno.hor                = READBOOL(READUNOHOR);
    conf.uno.contAware          = READSTRINGLIST(READUNOCONT).contains(QFileInfo(qApp->applicationFilePath()).fileName());
    conf.uno.opacity            = READFLOAT(READUNOOPACITY)/100.0f;
    conf.uno.blur               = READINT(READUNOCONTBLUR);
    //windows when not uno
    conf.windows.gradient       = stringToGrad(READSTRING(READWINGRAD));
    conf.windows.noise          = READINT(READWINNOISE);
    conf.windows.noiseStyle     = READINT(READWINNOISESTYLE);
    conf.windows.hor            = READBOOL(READWINHOR);
    //menues
    conf.menues.icons           = READBOOL(READMENUICONS);
    //sliders
    conf.sliders.size           = READINT(READSLIDERSIZE);
    conf.sliders.dot            = READBOOL(READSLIDERDOT);
    conf.sliders.grooveShadow   = READINT(READSLIDERGROOVESHAD);
    conf.sliders.grooveGrad     = stringToGrad(READSTRING(READSLIDERGROOVE));
    conf.sliders.sliderGrad     = stringToGrad(READSTRING(READSLIDERGRAD));
    conf.sliders.fillGroove     = READBOOL(READSLIDERFILLGROOVE);
    conf.sliders.grooveStyle    = READINT(READSLIDERINVERT);

    //scrollers
    conf.scrollers.size         = READINT(READSCROLLERSIZE);
    conf.scrollers.style        = READINT(READSCROLLERSTYLE);
    conf.scrollers.grooveGrad   = stringToGrad(READSTRING(READSCROLLERGROOVE));
    conf.scrollers.sliderGrad   = stringToGrad(READSTRING(READSCROLLERGRAD));
    conf.scrollers.grooveStyle  = READINT(READSCROLLERINVERTION);
    conf.scrollers.grooveShadow = READINT(READSCROLLERGSHADOW);
    //views
    conf.views.treelines        = READBOOL(READVIEWTREELINES);
    //progressbars
    conf.progressbars.shadow    = READINT(READPROGSHADOW);
    conf.progressbars.rnd       = READINT(READPROGRND);
    //shadows
    conf.shadows.opacity        = READFLOAT(READSHADOWOPACITY)/100.0f;
    conf.shadows.darkRaisedEdges = READBOOL(READSHADOWDARKRAISED);
#undef READINT
#undef READBOOL
#undef READFLOAT
#undef READSTRING
#undef READSTRINGLIST
    readPalette();
    s_isValid = true;
}

void
Settings::writeDefaults()
{
    if (!s_settings)
        return;

    qDebug() << "attempting to write default values...";
    for (int i = 0; i < Keycount; ++i)
        s_settings->setValue(s_key[i], s_default[i]);
    s_settings->sync();
}

void
Settings::edit()
{
    if (s_settings)
    {
        qDebug() << "trying to open file" << s_settings->fileName() << "in your default text editor.";
        QDesktopServices::openUrl(QUrl::fromLocalFile(s_settings->fileName()));
    }
}
