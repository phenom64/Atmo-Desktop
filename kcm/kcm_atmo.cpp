#include <KPluginFactory>
#include <KCMUtils/KCModule>
#include <KPluginMetaData>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProcess>
#include <QFileInfo>
#include <QWidget>

class KCMAtmo : public KCModule {
    Q_OBJECT
public:
    explicit KCMAtmo(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
        : KCModule(parent, metaData, args)
    {
        auto *lay = new QVBoxLayout(this);
        auto *lbl = new QLabel(tr("Atmo (NSE) - Settings stub.\nOpen the full Atmo Framework Manager for complete configuration."), this);
        lbl->setWordWrap(true);
        lay->addWidget(lbl);
        auto *btn = new QPushButton(tr("Open Atmo Framework Manager"), this);
        lay->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this]
        {
            const QString bin("/usr/bin/atmo_manager");
            if (QFileInfo::exists(bin))
                QProcess::startDetached(bin);
            else
                QProcess::startDetached("atmo_manager"); // fallback if custom path is used
        });
    }
};

K_PLUGIN_CLASS_WITH_JSON(KCMAtmo, "kcm_atmo.json")

#include "kcm_atmo.moc"
