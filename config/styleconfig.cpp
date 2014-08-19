#include "styleconfig.h"
#include "../settings.h"
#include <QSettings>
#include <QDebug>
#include <QDialog>

/** This function declares the kstyle config plugin, you may need to adjust it
for other plugins or won't need it at all, if you're not interested in a plugin */
extern "C"
{
    Q_DECL_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
    {
        /**Create our config dialog and reply it as plugin
        This is slightly different from the setup in a standalone dialog at the
        bottom of this file*/
        return new StyleConfig(parent);
    }
}

StyleConfig::StyleConfig(QWidget *parent) : QWidget(parent)
{
    QSettings s("dsp", "dsp");
    ui.setupUi(this);
    ui.pbRoundness->setValue(s.value(DEFPUSHBTNRND).toInt());
    ui.tbRoundness->setValue(s.value(DEFTOOLBTNRND).toInt());
    ui.inputRoundness->setValue(s.value(DEFINPUTRND).toInt());
    ui.scrollerSize->setValue(s.value(DEFSCROLLERSIZE).toInt());
    ui.sliderSize->setValue(s.value(DEFSLIDERSIZE).toInt());
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
}
