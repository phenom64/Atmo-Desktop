#include <KPluginFactory>
#include <KCModule>
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
    explicit KCMAtmo(QObject *parent, const KPluginMetaData &metaData)
        : KCModule(parent, metaData)
    {
        QWidget *root = widget();
        auto *lay = new QVBoxLayout(root);
        auto *lbl = new QLabel(tr("Atmo (NSE) - Settings stub.\nOpen the full Atmo Framework Manager for complete configuration."), root);
        lbl->setWordWrap(true);
        lay->addWidget(lbl);
        auto *btn = new QPushButton(tr("Open Atmo Framework Manager"), root);
        lay->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this]
        {
            const QString bin(QStringLiteral("/usr/bin/atmo_manager"));
            if (QFileInfo::exists(bin))
                QProcess::startDetached(bin);
            else
                QProcess::startDetached(QStringLiteral("atmo_manager")); // fallback if custom path is used
        });
    }
};

K_PLUGIN_CLASS_WITH_JSON(KCMAtmo, "kcm_atmo.json")

#include "kcm_atmo.moc"
