#include "kwinclient2.h"
#include "decobutton.h"
#include "decoadaptor.h"
#include "../stylelib/color.h"
#include "../stylelib/xhandler.h"
#include "../stylelib/shadowhandler.h"
#include "../stylelib/render.h"

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>
#include <KDecoration2/DecorationButton>

#include <KConfigGroup>
#include <KColorUtils>
#include <KSharedConfig>
#include <KPluginFactory>

#include <KSharedConfig>
#include <KCModule>

#include <KWindowInfo>

#include <QEvent>
#include <QMouseEvent>

#include <QTimer>
#include <QSharedMemory>

#include <QDBusConnection>

#include <QPixmap>

#include <QSettings>
#include <QDir>

#include <QDebug>

K_PLUGIN_FACTORY_WITH_JSON(
    DecoFactory,
    "dsp.json",
    registerPlugin<DSP::Deco>();
    registerPlugin<DSP::Button>(QStringLiteral("button"));
    registerPlugin<DSP::ConfigModule>(QStringLiteral("kcmodule"));
)

namespace DSP
{

ConfigModule::ConfigModule(QWidget *parent, const QVariantList &args) : KCModule(parent, args)
{

}

///-------------------------------------------------------------------------------------------------

static QMap<QString, DecoData> s_data;

static void addDataForWinClass(const QString &winClass, QSettings &s)
{
    DecoData d;
    d.fg = QColor::fromRgba(s.value("fgcolor", "0x00000000").toString().toUInt(0, 16));
    d.bg = QColor::fromRgba(s.value("bgcolor", "0x00000000").toString().toUInt(0, 16));
    d.gradient = Settings::stringToGrad(s.value("gradient", "0:10, 1:-10").toString());
    d.noiseRatio = s.value("noise", 20).toUInt();
    d.separator = s.value("separator", true).toBool();
    s_data.insert(winClass, d);
}

static void readWindowData()
{
    s_data.clear();
    static const QString confPath(QString("%1/.config/dsp").arg(QDir::homePath()));
    QSettings s(QString("%1/dspdeco.conf").arg(confPath), QSettings::IniFormat);
    bool hasDefault(false);
    foreach (const QString winClass, s.childGroups())
    {
        s.beginGroup(winClass);
        hasDefault |= (winClass == "default");
        addDataForWinClass(winClass, s);
        s.endGroup();
    }
    if (!hasDefault)
        addDataForWinClass("default", s);
}

DecoData decoData(const QString &winClass)
{
    if (s_data.contains(winClass))
        return s_data.value(winClass);
    return s_data.value("default");
}

///-------------------------------------------------------------------------------------------------

AdaptorManager *AdaptorManager::s_instance = 0;

AdaptorManager
*AdaptorManager::instance()
{
    if (!s_instance)
        s_instance = new AdaptorManager();
    return s_instance;
}

AdaptorManager::AdaptorManager()
{
    Settings::read();
    Render::makeNoise();
    readWindowData();
    new DecoAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.kde.dsp.kdecoration2");
    QDBusConnection::sessionBus().registerObject("/DSPDecoAdaptor", this);
}

AdaptorManager::~AdaptorManager()
{
    QDBusConnection::sessionBus().unregisterService("org.kde.dsp.kdecoration2");
    QDBusConnection::sessionBus().unregisterObject("/DSPDecoAdaptor");
    s_instance = 0;
}

///-------------------------------------------------------------------------------------------------

#define TITLEHEIGHT client().data()->isModal()?20:25

Deco::Deco(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
    , m_leftButtons(0)
    , m_rightButtons(0)
    , m_windowData(0)
    , m_prevLum(0)
    , m_mem(0)
{
}

Deco::~Deco()
{
    AdaptorManager::instance()->removeDeco(this);
     if (m_windowData)
     {
         delete m_windowData;
         m_windowData = 0;
     }
     if (m_mem && m_mem->isAttached())
         m_mem->detach();
}

void
Deco::init()
{
    AdaptorManager::instance()->addDeco(this);
    setBorders(QMargins(0, TITLEHEIGHT, 0, 0));

    //for whatever reason if I use these convenience constructs it segfaults.
//    m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &Button::create);
//    m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &Button::create);
    m_leftButtons = new KDecoration2::DecorationButtonGroup(this);
    m_leftButtons->setSpacing(4);
    QVector<KDecoration2::DecorationButtonType> lb = settings()->decorationButtonsLeft();
    for (int i = 0; i < lb.count(); ++i)
    {
        const KDecoration2::DecorationButtonType t = lb.at(i);
        m_leftButtons->addButton(Button::create(t, this, m_leftButtons));
    }

    m_rightButtons = new KDecoration2::DecorationButtonGroup(this);
    m_rightButtons->setSpacing(4);
    QVector<KDecoration2::DecorationButtonType> rb = settings()->decorationButtonsRight();
    for (int i = 0; i < rb.count(); ++i)
    {
        const KDecoration2::DecorationButtonType t = rb.at(i);
        m_rightButtons->addButton(Button::create(t, this, m_rightButtons));
    }
    connect(client().data(), &KDecoration2::DecoratedClient::widthChanged, this, &Deco::widthChanged);
    connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Deco::activeChanged);
    connect(client().data(), &KDecoration2::DecoratedClient::captionChanged, this, &Deco::captionChanged);

    m_windowData = new WindowData();
    m_windowData->setValue<bool>(WindowData::Separator, true);
    m_windowData->setValue<bool>(WindowData::Uno, true);
    m_windowData->setValue<int>(WindowData::UnoHeight, TITLEHEIGHT);

    if (uint id = client().data()->windowId())
    {
        unsigned char height(TITLEHEIGHT);
        XHandler::setXProperty<unsigned char>(id, XHandler::DecoTitleHeight, XHandler::Byte, &height); //never higher then 255...
        ShadowHandler::installShadows(id);
        updateDataFromX();
    }
}

void
Deco::widthChanged()
{
    m_leftButtons->setPos(QPoint(6, client().data()->isModal()?2:4));
    m_rightButtons->setPos(QPointF((size().width()-m_rightButtons->geometry().width())-6, client().data()->isModal()?2:4));
    setTitleBar(QRect(0, 0, client().data()->width(), TITLEHEIGHT));
}

void
Deco::paint(QPainter *painter, const QRect &repaintArea)
{
    //bg
    if (!m_pix.isNull())
        painter->drawTiledPixmap(titleBar(), m_pix);
    else
        painter->fillRect(titleBar(), client().data()->palette().color(QPalette::Window));

    const int bgLum(Color::luminosity(bgColor()));
    const bool isDark(Color::luminosity(fgColor()) > bgLum);

    if ((!dConf.deco.frameSize || client().data()->isMaximized()) && !client().data()->isModal())
        paintBevel(painter, bgLum);
    if (m_windowData && m_windowData->value<bool>(WindowData::Separator))
    {
        painter->setPen(QColor(0, 0, 0, 32));
        painter->drawLine(0, TITLEHEIGHT, titleBar().width(), TITLEHEIGHT);
    }
    if (m_windowData && m_windowData->value<bool>(WindowData::ContAware))
    {
        if (!m_mem)
            m_mem = new QSharedMemory(QString::number(client().data()->windowId()), this);
        if ((m_mem->isAttached() || m_mem->attach(QSharedMemory::ReadOnly)) && m_mem->lock())
        {
            const uchar *data(reinterpret_cast<const uchar *>(m_mem->constData()));
            painter->drawImage(QPoint(0, 0), QImage(data, titleBar().width(), m_windowData->value<int>(WindowData::UnoHeight), QImage::Format_ARGB32_Premultiplied), titleBar());
            m_mem->unlock();
        }
    }

    //shape
    if (!client().data()->isModal())
        Render::shapeCorners(painter, Render::Top|Render::Right|Render::Left);
    //text
    if (client().data()->isActive())
    {
        QFont f(painter->font());
        f.setBold(true);
        painter->setFont(f);
    }

    QString text(client().data()->caption());
    QRect textRect(painter->fontMetrics().boundingRect(titleBar(), Qt::AlignCenter|Qt::TextHideMnemonic, text));
    const int maxW(titleBar().width()-(qMax(m_leftButtons->geometry().width(), m_rightButtons->geometry().width())*2));
    if (painter->fontMetrics().width(text) > maxW)
    {
        text = painter->fontMetrics().elidedText(text, Qt::ElideRight, maxW);
        textRect = painter->fontMetrics().boundingRect(titleBar(), Qt::AlignCenter|Qt::TextHideMnemonic, text);
    }
    if (client().data()->isActive())
    {
        const int rgb(isDark?0:255);
        painter->setPen(QColor(rgb, rgb, rgb, 127));
        painter->drawText(textRect.translated(0, 1), Qt::AlignCenter|Qt::TextHideMnemonic, text);
    }

    static const QString character[] = { " :: ", QString(" %1 ").arg(QChar(0x2013)), " - " };
    bool needPaint(true);
    painter->setPen(fgColor());
    for (int i = 0; i < 3; ++i)
    if (text.contains(character[i]))
    {
        needPaint = false;
        QString leftText(text.mid(0, text.lastIndexOf(character[i])));
        QString rightText(text.mid(text.lastIndexOf(character[i])));

        QRect leftTextRect(textRect.adjusted(0, 0, -painter->fontMetrics().width(rightText), 0));
        QRect rightTextRect(textRect.adjusted(painter->fontMetrics().width(leftText), 0, 0, 0));

        painter->drawText(leftTextRect, Qt::AlignCenter|Qt::TextHideMnemonic, leftText);
        painter->setPen(Color::mid(fgColor(), bgColor(), 2, 1));
        painter->drawText(rightTextRect, Qt::AlignCenter|Qt::TextHideMnemonic, rightText);
        break;
    }

    if (needPaint)
    {
        painter->setPen(fgColor());
        painter->drawText(textRect, Qt::AlignCenter|Qt::TextHideMnemonic, text);
    }

    //icon
    if (dConf.deco.icon)
    {
        QRect ir(QPoint(), QSize(16, 16));
        ir.moveTop(titleBar().top()+(titleBar().height()/2-ir.height()/2));
        ir.moveRight(textRect.left()-4);
        if (ir.left() > m_leftButtons->geometry().width())
            client().data()->icon().paint(painter, ir, Qt::AlignCenter, client().data()->isActive()?QIcon::Active:QIcon::Disabled);
    }

    //buttons
    m_leftButtons->paint(painter, repaintArea);
    m_rightButtons->paint(painter, repaintArea);
}

void
Deco::paintBevel(QPainter *painter, const int bgLum)
{
    if (m_bevelCorner[0].isNull() || m_prevLum != bgLum)
    {
        m_prevLum = bgLum;
        if (!m_bevelCorner[0])
        {
            for (int i = 0; i < 2; ++i)
            {
                m_bevelCorner[i] = QPixmap(5, 5);
                m_bevelCorner[i].fill(Qt::transparent);
            }
            m_bevelCorner[2] = QPixmap(1, 1);
            m_bevelCorner[2].fill(Qt::transparent);
        }

        QPixmap tmp(9, 9);
        tmp.fill(Qt::transparent);
        QPainter pt(&tmp);
        pt.setRenderHint(QPainter::Antialiasing);
        const QRectF bevel(0.5f, 0.5f, tmp.width()-0.5f, tmp.height());
        QLinearGradient lg(bevel.topLeft(), bevel.bottomLeft());
        lg.setColorAt(0.0f, QColor(255, 255, 255, qMin(255.0f, bgLum*1.1f)));
        lg.setColorAt(0.5f, Qt::transparent);
        pt.setBrush(lg);
        pt.setPen(Qt::NoPen);
        pt.drawEllipse(tmp.rect());
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.setBrush(Qt::black);
        pt.drawEllipse(tmp.rect().adjusted(1, 1, -1, -1));
        pt.end();
        m_bevelCorner[0] = tmp.copy(QRect(0, 0, 4, 4));
        m_bevelCorner[1] = tmp.copy(QRect(6, 0, 4, 4));
        m_bevelCorner[2] = tmp.copy(5, 0, 1, 1);
    }
    painter->drawPixmap(QRect(titleBar().topLeft(), m_bevelCorner[0].size()), m_bevelCorner[0]);
    painter->drawPixmap(QRect(titleBar().topRight()-QPoint(m_bevelCorner[1].width()-1, 0), m_bevelCorner[1].size()), m_bevelCorner[1]);
    painter->drawTiledPixmap(titleBar().adjusted(m_bevelCorner[0].width(), 0, -m_bevelCorner[1].width(), -(titleBar().height()-1)), m_bevelCorner[2]);
}

void
Deco::updateDataFromX()
{
    const unsigned int id = client().data()->windowId();
    if (!id)
        return;
    if (SharedBgPixData *bgPixData = XHandler::getXProperty<SharedBgPixData>(id, XHandler::DecoBgPix))
    {
        setBgPix(bgPixData->bgPix, QSize(bgPixData->w, bgPixData->h));
        XHandler::freeData(bgPixData);
    }
    else
        checkForDataFromWindowClass();
    if (WindowData *wd = XHandler::getXProperty<WindowData>(id, XHandler::WindowData))
    {
        setWindowData(*wd);
        XHandler::freeData(wd);
    }

}

void
Deco::checkForDataFromWindowClass()
{
    KWindowInfo info(client().data()->windowId(), NET::WMWindowType|NET::WMVisibleName|NET::WMName, NET::WM2WindowClass);
    DecoData d = decoData(info.windowClassClass());
    if (d.isValid())
    {
        m_decoData = d;
        updateBgPixmap();
    }
}

void
Deco::setBgPix(const unsigned long pix, const QSize &sz)
{
    QImage img = XHandler::fromX11Pix(pix, sz);
    if (img.isNull())
        return;
    m_pix = QPixmap(img.size());
    m_pix.fill(Qt::transparent);
    QPainter p(&m_pix);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawImage(m_pix.rect(), img);
    p.end();
    update();
}

void
Deco::setWindowData(WindowData wd)
{
    if (!m_windowData)
        m_windowData = new WindowData();
    *m_windowData = wd;
    if (wd.value<int>(WindowData::Opacity) != 0xff)
    {
        bool needBlur(true);
        const uint id = client().data()->windowId();
        unsigned int *sd = XHandler::getXProperty<unsigned int>(id, XHandler::_KDE_NET_WM_BLUR_BEHIND_REGION);
        if (sd)
        {
            needBlur = false;
            XHandler::freeData(sd);
        }
        if (needBlur)
        {
            unsigned int d(0);
            XHandler::setXProperty<unsigned int>(id, XHandler::_KDE_NET_WM_BLUR_BEHIND_REGION, XHandler::Long, &d);
        }
    }
    update();
}

const QColor
Deco::bgColor() const
{
    if (m_windowData && m_windowData->isValid())
        return QColor::fromRgba(m_windowData->bg);
    if (m_decoData.isValid())
        return m_decoData.bg;
    return client().data()->palette().color(QPalette::Window);
}

const QColor
Deco::fgColor() const
{
    if (m_windowData && m_windowData->isValid())
        return QColor::fromRgba(m_windowData->fg);
    if (m_decoData.isValid())
        return m_decoData.fg;
    return client().data()->palette().color(QPalette::WindowText);
}

void
Deco::updateBgPixmap()
{
    if (m_windowData && m_windowData->value<bool>(WindowData::Uno))
    {
        QRect r(0, 0, titleBar().width(), m_windowData->value<int>(WindowData::UnoHeight));
        QLinearGradient lg(r.topLeft(), r.bottomLeft());
        if (m_decoData.isValid() && !m_decoData.gradient.isEmpty())
            lg.setStops(Settings::gradientStops(m_decoData.gradient, bgColor()));
        else
        {
            lg.setColorAt(0.0f, Color::mid(bgColor(), Qt::white, 4, 1));
            lg.setColorAt(1.0f, Color::mid(bgColor(), Qt::black, 4, 1));
        }
        QPixmap p(Render::noise().size().width(), m_windowData->value<int>(WindowData::UnoHeight));
        p.fill(Qt::transparent);
        QPainter pt(&p);
        pt.fillRect(p.rect(), lg);
        if (m_decoData.isValid() && m_decoData.noiseRatio)
        {
            QPixmap noise(Render::noise().size());
            noise.fill(Qt::transparent);
            QPainter ptt(&noise);
            ptt.drawTiledPixmap(noise.rect(), Render::noise());
            ptt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            ptt.fillRect(noise.rect(), QColor(0, 0, 0, m_decoData.noiseRatio*2.55f));
            ptt.end();
            pt.setCompositionMode(QPainter::CompositionMode_Overlay);
            pt.drawTiledPixmap(p.rect(), noise);
        }
        pt.end();
        m_pix = p;
    }
}

} //DSP

/*
 * Required for the K_PLUGIN_FACTORY_WITH_JSON vtable
 */
#include "kwinclient2.moc"
