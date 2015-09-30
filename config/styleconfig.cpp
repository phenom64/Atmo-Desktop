#include "styleconfig.h"
#include "settings.h"
#include <QSettings>

#if QT_VERSION < 0x050000
extern "C"
{
    Q_DECL_EXPORT QWidget* allocate_kstyle_config(QWidget* parent)
    {
        return new DSP::StyleConfig(parent);
    }
}
#endif

using namespace DSP;

StyleConfig::StyleConfig(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    readSettings();
}

void
StyleConfig::save()
{
    emit changed(true);
}

void
StyleConfig::defaults()
{
}

void
StyleConfig::readSettings()
{
}

//#include "styleconfig.moc"
