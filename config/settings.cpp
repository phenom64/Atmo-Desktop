#include "settings.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QPalette>
#include <QDebug>

Q_DECL_EXPORT Settings Settings::conf;

Settings::Settings(QObject *parent) : QObject(parent), palette(0), m_settings(0)
{
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(writePalette()));
    conf.m_appName = QFileInfo(qApp->applicationFilePath()).fileName();
    QString settingsPath(QString("%1/.config/dsp").arg(QDir::homePath()));

    if (conf.m_appName == "eiskaltdcpp-qt")
        conf.app = Eiskalt;
    else if (conf.m_appName == "konversation")
        conf.app = Konversation;
    else if (conf.m_appName == "konsole")
        conf.app = Konsole;
    else if (conf.m_appName == "kwin")
        conf.app == KWin;
    else
        conf.app = None;

    const QFileInfo presetFile(QDir(settingsPath).absoluteFilePath(QString("%1.conf").arg(conf.m_appName)));
    QString append("/dsp.conf");
    if (presetFile.exists())
        append = QString("/%1.conf").arg(conf.m_appName);
    settingsPath.append(append);

    conf.m_settings = new QSettings(settingsPath, QSettings::NativeFormat);
    read();
}

Settings::~Settings()
{
    if (conf.m_settings)
    {
        delete conf.m_settings;
        conf.m_settings = 0;
    }
}

static Gradient stringToGrad(const QString &string)
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

static char *paletteString[3] = { "activePalette", "disabledPalette", "inactivePalette" };

void
Settings::writePalette()
{
    const QPalette pal(QApplication::palette());
    for (int g = 0; g < QPalette::NColorGroups; ++g)
    {
        unsigned int data[QPalette::NColorRoles];
        for (int i = 0; i < QPalette::NColorRoles; ++i)
        {
//            qDebug() << QApplication::palette().color((QPalette::ColorGroup)g, (QPalette::ColorRole)i) << QApplication::palette().color((QPalette::ColorRole)i);
            data[i] = pal.color((QPalette::ColorGroup)g, (QPalette::ColorRole)i).rgba();
        }
        QByteArray array(reinterpret_cast<char *>(data), QPalette::NColorRoles*sizeof(unsigned int));
        conf.m_settings->setValue(paletteString[g], array);
    }
}

void
Settings::readPalette()
{
    for (int g = 0; g < QPalette::NColorGroups; ++g)
    {
        QByteArray array(conf.m_settings->value(paletteString[g], QByteArray()).toByteArray());
        if (array.isEmpty())
            return;
        if (!conf.palette)
            conf.palette = new QPalette();
        unsigned int *data = reinterpret_cast<unsigned int *>(array.data());
        for (int i = 0; i < QPalette::NColorRoles; ++i)
        {
//            qDebug() << qRed(data[i]) << qGreen(data[i]) << qBlue(data[i]) << qAlpha(data[i]);
            conf.palette->setColor((QPalette::ColorGroup)g, (QPalette::ColorRole)i, QColor::fromRgba(data[i]));
        }
    }
}

void
Settings::read()
{
#define READINT(_VAR_) conf.m_settings->value(_VAR_).toInt();
    //globals
    conf.opacity = conf.m_settings->value(READOPACITY).toFloat()/100.0f;
    conf.blackList = conf.m_settings->value(READBLACKLIST).toStringList();
    if (conf.blackList.contains(conf.m_appName) || conf.app == KWin)
        conf.opacity = 1.0f;
    conf.removeTitleBars = conf.m_settings->value(READREMOVETITLE).toBool();
    conf.titlePos = conf.removeTitleBars?conf.m_settings->value(READTITLEPOS).toInt():-1;
    conf.hackDialogs = conf.m_settings->value(READHACKDIALOGS).toBool();
    conf.compactMenu = conf.m_settings->value(READCOMPACTMENU).toBool();
    conf.splitterExt = conf.m_settings->value(READSPLITTEREXT).toBool();
    conf.arrowSize = READINT(READARROWSIZE);

    readPalette();

    //deco
    conf.deco.buttons = conf.m_settings->value(READDECOBUTTONS).toInt();
    conf.deco.icon = conf.m_settings->value(READDECOICON).toBool();
    conf.deco.shadowSize = READINT(READDECOSHADOWSIZE);
    conf.deco.frameSize = READINT(READDECOFRAME);
    //pushbuttons
    conf.pushbtn.rnd = conf.m_settings->value(READPUSHBTNRND).toInt();
    conf.pushbtn.shadow = conf.m_settings->value(READPUSHBTNSHADOW).toInt();
    conf.pushbtn.gradient = stringToGrad(conf.m_settings->value(READPUSHBTNGRAD).toString());
    conf.pushbtn.tint = tintColor(conf.m_settings->value(READPUSHBTNTINT).toString());
    //toolbuttons
    conf.toolbtn.rnd = conf.m_settings->value(READTOOLBTNRND).toInt();
    conf.toolbtn.shadow = conf.m_settings->value(READTOOLBTNSHADOW).toInt();
    conf.toolbtn.gradient = stringToGrad(conf.m_settings->value(READTOOLBTNGRAD).toString());
    conf.toolbtn.tint = tintColor(conf.m_settings->value(READTOOLBTNTINT).toString());
    conf.toolbtn.folCol = conf.m_settings->value(READTOOLBTNFOLCOL).toBool();
    conf.toolbtn.invAct = conf.m_settings->value(READTOOLBTNINVACT).toBool();
    conf.toolbtn.flat = conf.m_settings->value(READTOOLBTNFLAT).toBool();
    //inputs
    conf.input.rnd = conf.m_settings->value(READINPUTRND).toInt();
    conf.input.shadow = conf.m_settings->value(READINPUTSHADOW).toInt();
    conf.input.gradient = stringToGrad(conf.m_settings->value(READINPUTGRAD).toString());
    conf.input.tint = tintColor(conf.m_settings->value(READINPUTTINT).toString());
    //tabs
    conf.tabs.rnd = conf.m_settings->value(READTABRND).toInt();
    conf.tabs.shadow = conf.m_settings->value(READTABSHADOW).toInt();
    conf.tabs.gradient = stringToGrad(conf.m_settings->value(READTABGRAD).toString());
    conf.tabs.safrnd = qMin(conf.m_settings->value(READSAFTABRND).toInt(), 8);
    conf.tabs.closeButtonSide = conf.m_settings->value(READTABCLOSER).toInt();
    //uno
    conf.uno.enabled = conf.m_settings->value(READUNOENABLED).toBool();
    conf.uno.gradient = stringToGrad(conf.m_settings->value(READUNOGRAD).toString());
    conf.uno.tint = tintColor(conf.m_settings->value(READUNOTINT).toString());
    conf.uno.noise = conf.m_settings->value(READUNONOISE).toInt();
    conf.uno.noiseStyle = conf.m_settings->value(READUNONOISESTYLE).toInt();
    conf.uno.hor = conf.m_settings->value(READUNOHOR).toBool();
    conf.uno.contAware = conf.m_settings->value(READUNOCONT).toStringList().contains(QFileInfo(qApp->applicationFilePath()).fileName());
    conf.uno.opacity = conf.m_settings->value(READUNOOPACITY).toFloat()/100.0f;
    conf.uno.blur = conf.m_settings->value(READUNOCONTBLUR).toInt();
    //windows when not uno
    conf.windows.gradient = stringToGrad(conf.m_settings->value(READWINGRAD).toString());
    conf.windows.noise = READINT(READWINNOISE);
    conf.windows.noiseStyle = READINT(READWINNOISESTYLE);
    conf.windows.hor = conf.m_settings->value(READWINHOR).toBool();
    //menues
    conf.menues.icons = conf.m_settings->value(READMENUICONS).toBool();
    //sliders
    conf.sliders.size = conf.m_settings->value(READSLIDERSIZE).toInt();
    conf.sliders.dot = conf.m_settings->value(READSLIDERDOT).toBool();
    conf.sliders.grooveShadow = conf.m_settings->value(READSLIDERGROOVESHAD).toInt();
    conf.sliders.grooveGrad = stringToGrad(conf.m_settings->value(READSLIDERGROOVE).toString());
    conf.sliders.sliderGrad = stringToGrad(conf.m_settings->value(READSLIDERGRAD).toString());
    conf.sliders.fillGroove = conf.m_settings->value(READSLIDERFILLGROOVE).toBool();
    //scrollers
    conf.scrollers.size = conf.m_settings->value(READSCROLLERSIZE).toInt();
    conf.scrollers.style = READINT(READSCROLLERSTYLE);
    conf.scrollers.grooveGrad = stringToGrad(conf.m_settings->value(READSCROLLERGROOVE).toString());
    conf.scrollers.sliderGrad = stringToGrad(conf.m_settings->value(READSCROLLERGRAD).toString());
    //views
    conf.views.treelines = conf.m_settings->value(READVIEWTREELINES).toBool();
    //progressbars
    conf.progressbars.shadow = READINT(READPROGSHADOW);
    conf.progressbars.rnd = READINT(READPROGRND);
    //shadows
    conf.shadows.opacity = conf.m_settings->value(READSHADOWOPACITY).toFloat()/100.0f;

#undef READINT
}
