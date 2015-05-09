#include "kwinclient2.h"

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>

#include <KConfigGroup>
#include <KColorUtils>
#include <KSharedConfig>
#include <KPluginFactory>

#include <KSharedConfig>
#include <KCModule>

#include <QEvent>
#include <QMouseEvent>

K_PLUGIN_FACTORY_WITH_JSON(
    DSPDecoFactory,
    "dsp.json",
    registerPlugin<DSPDeco>();
    registerPlugin<DSPButton>(QStringLiteral("button"));
    registerPlugin<DSPConfigModule>(QStringLiteral("kcmodule"));
)

DSPConfigModule::DSPConfigModule(QWidget *parent, const QVariantList &args) : KCModule(parent, args)
{

}

///-------------------------------------------------------------------------------------------------

DSPDeco::DSPDeco(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
    , m_leftButtons(0)
    , m_rightButtons(0)
{

}

void
DSPDeco::init()
{
    m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &DSPButton::create);
    m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &DSPButton::create);
}

void
DSPDeco::paint(QPainter *painter, const QRect &repaintArea)
{

}

bool
DSPDeco::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (m_leftButtons)
        {
            QVector<QPointer<KDecoration2::DecorationButton> > btns = m_leftButtons->buttons();
            for (int i = 0; i < btns.count(); ++i)
            {
                DSPButton *b = static_cast<DSPButton *>(btns.at(i).data());
                b->processMouseEvent(me);
            }
        }
        if (m_rightButtons)
        {
            QVector<QPointer<KDecoration2::DecorationButton> > btns = m_rightButtons->buttons();
            for (int i = 0; i < btns.count(); ++i)
            {
                DSPButton *b = static_cast<DSPButton *>(btns.at(i).data());
                b->processMouseEvent(me);
            }
        }
        break;
    }
    default: break;
    }
    return KDecoration2::Decoration::event(event);
}

/*
 * Required for the K_PLUGIN_FACTORY_WITH_JSON vtable
 */
#include "kwinclient2.moc"
