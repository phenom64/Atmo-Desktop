#include "styleconfig.h"
#include "settings.h"
#include <QSettings>

extern "C"
{
    Q_DECL_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
    {
        return new StyleConfig(parent);
    }
}

StyleConfig::StyleConfig(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    readSettings();
}

void
StyleConfig::save()
{
    QSettings s("dsp", "dsp");
    s.setValue(PUSHBTNRND, ui.pbRoundness->value());
    s.setValue(TOOLBTNRND, ui.tbRoundness->value());
    s.setValue(INPUTRND, ui.inputRoundness->value());
    s.setValue(SCROLLERSIZE, ui.scrollerSize->value());
    s.setValue(SLIDERSIZE, ui.sliderSize->value());
    emit changed(true);
}

void
StyleConfig::defaults()
{
    ui.pbRoundness->setValue(DEFPUSHBTNRND);
    ui.tbRoundness->setValue(DEFTOOLBTNRND);
    ui.inputRoundness->setValue(DEFINPUTRND);
    ui.scrollerSize->setValue(DEFSCROLLERSIZE);
    ui.sliderSize->setValue(DEFSLIDERSIZE);
}

void
StyleConfig::readSettings()
{
    QSettings s("dsp", "dsp");
    ui.pbRoundness->setValue(s.value(READPUSHBTNRND).toInt());
    ui.tbRoundness->setValue(s.value(READTOOLBTNRND).toInt());
    ui.inputRoundness->setValue(s.value(READINPUTRND).toInt());
    ui.scrollerSize->setValue(s.value(READSCROLLERSIZE).toInt());
    ui.sliderSize->setValue(s.value(READSLIDERSIZE).toInt());
}
