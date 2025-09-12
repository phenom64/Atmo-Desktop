#include <KPluginFactory>
#include <KCModule>
#include <QLabel>
#include <QVBoxLayout>

class KCMAtmo : public KCModule {
    Q_OBJECT
public:
    explicit KCMAtmo(QWidget *parent = nullptr, const QVariantList &args = QVariantList())
        : KCModule(parent, args)
    {
        auto *lay = new QVBoxLayout(this);
        auto *lbl = new QLabel(tr("Atmo (NSE) – Settings stub\nThis module will expose common options in a future release."), this);
        lbl->setWordWrap(true);
        lay->addWidget(lbl);
        setLayout(lay);
    }
};

K_PLUGIN_FACTORY_WITH_JSON(KCMAtmoFactory, "kcm_atmo.json", registerPlugin<KCMAtmo>();)

#include "kcm_atmo.moc"

