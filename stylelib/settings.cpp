#include "settings.h"
#include "color.h"
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QApplication>

Q_DECL_EXPORT Settings Settings::conf;

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
#if 0
    const QColor mix(pair.second>0?Qt::white:Qt::black);
    stop.second = Color::mid(c, mix, 100-val, val);
#else
    stop.second = pair.second==0?c:(pair.second>0?c.lighter(100+val):c.darker(100+val));
#endif
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

void
Settings::read()
{
    const QString appName(QFileInfo(qApp->applicationFilePath()).fileName());
    QString settingsPath(QString("%1/.config/dsp").arg(QDir::homePath()));

    const QFileInfo presetFile(QDir(settingsPath).absoluteFilePath(QString("%1.conf").arg(appName)));
    QString append("/dsp.conf");
    if (presetFile.exists())
        append = QString("/%1.conf").arg(appName);
    settingsPath.append(append);

#define READINT(_VAR_) s.value(_VAR_).toInt();

    QSettings s(settingsPath, QSettings::NativeFormat);
    //globals
    conf.opacity = s.value(READOPACITY).toFloat()/100.0f;
    conf.blackList = s.value(READBLACKLIST).toStringList();
    if (conf.blackList.contains(appName))
        conf.opacity = 1.0f;
    conf.removeTitleBars = s.value(READREMOVETITLE).toBool();
    conf.titlePos = conf.removeTitleBars?s.value(READTITLEPOS).toInt():-1;
    conf.hackDialogs = s.value(READHACKDIALOGS).toBool();
    conf.compactMenu = s.value(READCOMPACTMENU).toBool();
    //deco
    conf.deco.buttons = s.value(READDECOBUTTONS).toInt();
    conf.deco.icon = s.value(READDECOICON).toBool();
    conf.deco.shadowSize = READINT(READDECOSHADOWSIZE);
    //pushbuttons
    conf.pushbtn.rnd = s.value(READPUSHBTNRND).toInt();
    conf.pushbtn.shadow = s.value(READPUSHBTNSHADOW).toInt();
    conf.pushbtn.gradient = stringToGrad(s.value(READPUSHBTNGRAD).toString());
    conf.pushbtn.tint = tintColor(s.value(READPUSHBTNTINT).toString());
    //toolbuttons
    conf.toolbtn.rnd = s.value(READTOOLBTNRND).toInt();
    conf.toolbtn.shadow = s.value(READTOOLBTNSHADOW).toInt();
    conf.toolbtn.gradient = stringToGrad(s.value(READTOOLBTNGRAD).toString());
    conf.toolbtn.tint = tintColor(s.value(READTOOLBTNTINT).toString());
    conf.toolbtn.folCol = s.value(READTOOLBTNFOLCOL).toBool();
    conf.toolbtn.invAct = s.value(READTOOLBTNINVACT).toBool();
    //inputs
    conf.input.rnd = s.value(READINPUTRND).toInt();
    conf.input.shadow = s.value(READINPUTSHADOW).toInt();
    conf.input.gradient = stringToGrad(s.value(READINPUTGRAD).toString());
    conf.input.tint = tintColor(s.value(READINPUTTINT).toString());
    //tabs
    conf.tabs.rnd = s.value(READTABRND).toInt();
    conf.tabs.shadow = s.value(READTABSHADOW).toInt();
    conf.tabs.gradient = stringToGrad(s.value(READTABGRAD).toString());
    conf.tabs.safrnd = qMin(s.value(READSAFTABRND).toInt(), 8);
    conf.tabs.closeButtonSide = s.value(READTABCLOSER).toInt();
    //uno
    conf.uno.gradient = stringToGrad(s.value(READUNOGRAD).toString());
    conf.uno.tint = tintColor(s.value(READUNOTINT).toString());
    conf.uno.noise = s.value(READUNONOISE).toInt();
    conf.uno.hor = s.value(READUNOHOR).toBool();
    conf.uno.contAware = s.value(READUNOCONT).toStringList().contains(QFileInfo(qApp->applicationFilePath()).fileName());
    conf.uno.opacity = s.value(READUNOOPACITY).toFloat()/100.0f;
    conf.uno.blur = s.value(READUNOCONTBLUR).toInt();
    //menues
    conf.menues.icons = s.value(READMENUICONS).toBool();
    //sliders
    conf.sliders.size = s.value(READSLIDERSIZE).toInt();
    conf.sliders.dot = s.value(READSLIDERDOT).toBool();
    conf.sliders.grooveShadow = s.value(READSLIDERGROOVESHAD).toInt();
    conf.sliders.grooveGrad = stringToGrad(s.value(READSLIDERGROOVE).toString());
    conf.sliders.sliderGrad = stringToGrad(s.value(READSLIDERGRAD).toString());
    conf.sliders.fillGroove = s.value(READSLIDERFILLGROOVE).toBool();
    //scrollers
    conf.scrollers.size = s.value(READSCROLLERSIZE).toInt();
    //shadows
    conf.shadows.opacity = s.value(READSHADOWOPACITY).toFloat()/100.0f;

#undef READINT
}
