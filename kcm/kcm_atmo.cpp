#include <KPluginFactory>
#include <KCModule>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProcess>
#include <QFileInfo>

class KCMAtmo : public KCModule {
    Q_OBJECT
public:
    explicit KCMAtmo(QWidget *parent = nullptr, const QVariantList &args = QVariantList())
        : KCModule(parent, args)
    {
        auto *lay = new QVBoxLayout(this);
        auto *lbl = new QLabel(tr("Atmo (NSE) – Settings stub.\nOpen the full Atmo Framework Manager for complete configuration."), this);
        lbl->setWordWrap(true);
        lay->addWidget(lbl);
        auto *btn = new QPushButton(tr("Open Atmo Framework Manager"), this);
        lay->addWidget(btn);
        setLayout(lay);
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

K_PLUGIN_FACTORY_WITH_JSON(KCMAtmoFactory, "kcm_atmo.json", registerPlugin<KCMAtmo>();)

#include "kcm_atmo.moc"
