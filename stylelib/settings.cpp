#include "settings.h"
#include "color.h"
#include <QSettings>
#include <QFileInfo>
#include <QApplication>

Q_DECL_EXPORT Settings Settings::conf;

static QList<QPair<float, int> > stringToGrad(const QString &string)
{
    const QStringList pairs(string.split(",", QString::SkipEmptyParts));
    QList<QPair<float, int> > gradient;
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
        gradient << QPair<float, int>(stop, val);
    }
    return gradient;
}

QGradientStop
Settings::pairToStop(const QPair<float, int> pair, const QColor &c)
{
    QGradientStop stop;
    stop.first = pair.first;
    const QColor mix(pair.second>0?Qt::white:Qt::black);
    const int val(qAbs(pair.second));
    stop.second = Color::mid(c, mix, 100-val, val);
    return stop;
}

QGradientStops
Settings::gradientStops(const QList<QPair<float, int> > pairs, const QColor &c)
{
    QGradientStops stops;
    for (int i = 0; i < pairs.count(); ++i)
        stops << pairToStop(pairs.at(i), c);
    return stops;
}

void
Settings::read()
{
    QSettings s("dsp", "dsp");
    //globals
    conf.opacity = s.value(READOPACITY).toFloat()/100.0f;
    conf.blackList = s.value(READBLACKLIST).toStringList();
    if (conf.blackList.contains(QFileInfo(qApp->applicationFilePath()).fileName()))
        conf.opacity = 1.0f;
    conf.removeTitleBars = s.value(READREMOVETITLE).toBool();
    conf.hackDialogs = s.value(READHACKDIALOGS).toBool();
    conf.titleButtons = s.value(READTITLEBUTTONS).toInt();
    //pushbuttons
    conf.pushbtn.rnd = s.value(READPUSHBTNRND).toInt();
    conf.pushbtn.shadow = s.value(READPUSHBTNSHADOW).toInt();
    conf.pushbtn.gradient = stringToGrad(s.value(READPUSHBTNGRAD).toString());
    //toolbuttons
    conf.toolbtn.rnd = s.value(READTOOLBTNRND).toInt();
    conf.toolbtn.shadow = s.value(READTOOLBTNSHADOW).toInt();
    conf.toolbtn.gradient = stringToGrad(s.value(READTOOLBTNGRAD).toString());
    //inputs
    conf.input.rnd = s.value(READINPUTRND).toInt();
    conf.input.shadow = s.value(READINPUTSHADOW).toInt();
    conf.input.gradient = stringToGrad(s.value(READINPUTGRAD).toString());
    //sliders
    conf.sliders.size = s.value(READSLIDERSIZE).toInt();
    //scrollers
    conf.scrollers.size = s.value(READSCROLLERSIZE).toInt();
    //shadows
    conf.shadows.opacity = s.value(READSHADOWOPACITY).toFloat()/100.0f;
    //tabs
    conf.tabs.rnd = s.value(READTABRND).toInt();
    conf.tabs.closeButtonSide = s.value(READTABCLOSER).toInt();
}
