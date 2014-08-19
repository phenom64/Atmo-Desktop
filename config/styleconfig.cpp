#include "styleconfig.h"
#include "../settings.h"
#include <QSettings>

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
    ui.pbRoundness->setValue(s.value(PUSHBTNRND).toInt());
}
