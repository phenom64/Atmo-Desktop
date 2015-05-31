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
#include <QX11Info>
#include <xcb/xproto.h>

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

QMap<QString, Deco::Data> Deco::Data::s_data;

static void addDataForWinClass(const QString &winClass, QSettings &s)
{
    Deco::Data d;
    d.fg = QColor::fromRgba(s.value("fgcolor", "0x00000000").toString().toUInt(0, 16));
    d.bg = QColor::fromRgba(s.value("bgcolor", "0x00000000").toString().toUInt(0, 16));
    d.grad = Settings::stringToGrad(s.value("gradient", "0:10, 1:-10").toString());
    d.noise = s.value("noise", 20).toUInt();
    d.separator = s.value("separator", true).toBool();
    Deco::Data::s_data.insert(winClass, d);
}

static void readWindowData()
{
    Deco::Data::s_data.clear();
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

void
Deco::Data::decoData(const QString &winClass, Deco *d)
{
    Deco::Data data;
    if (s_data.contains(winClass))
        data = s_data.value(winClass);
    else
        data = s_data.value("default");
    d->m_bg = data.bg;
    d->m_fg = data.fg;
    d->m_gradient = data.grad;
    d->m_noise = data.noise;
    d->m_separator = data.separator;
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
//    Settings::read();
//    Render::makeNoise();
//    readWindowData();
    new DecoAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.kde.dsp.kwindeco");
    QDBusConnection::sessionBus().registerObject("/DSPDecoAdaptor", this);
}

AdaptorManager::~AdaptorManager()
{
    QDBusConnection::sessionBus().unregisterService("org.kde.dsp.kwindeco");
    QDBusConnection::sessionBus().unregisterObject("/DSPDecoAdaptor");
    s_instance = 0;
}

///-------------------------------------------------------------------------------------------------

#define TITLEHEIGHT client().data()->isModal()?20:25

Deco::Deco(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
    , m_leftButtons(0)
    , m_rightButtons(0)
    , m_prevLum(0)
    , m_mem(0)
    , m_noise(0)
    , m_separator(false)
    , m_wd(0)
{
}

Deco::~Deco()
{
    AdaptorManager::instance()->removeDeco(this);
}

void
Deco::init()
{
    setBorders(QMargins(0, TITLEHEIGHT, 0, 0));
    int buttonStyle = 0;
    if (const uint id = client().data()->windowId())
    {
        AdaptorManager::instance()->addDeco(this);
        buttonStyle = dConf.deco.buttons;
        if (m_wd = WindowData::memory(id, this))
        {
            initMemory();
            buttonStyle = m_wd->value<int>(WindowData::Buttons, buttonStyle);
        }
        else
            checkForDataFromWindowClass();
        ShadowHandler::installShadows(id);
    }
    else
        updateBgPixmap();
    //for whatever reason if I use these convenience constructs it segfaults.
//    m_leftButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Left, this, &Button::create);
//    m_rightButtons = new KDecoration2::DecorationButtonGroup(KDecoration2::DecorationButtonGroup::Position::Right, this, &Button::create);
    m_leftButtons = new KDecoration2::DecorationButtonGroup(this);
    m_leftButtons->setSpacing(4);
    QVector<KDecoration2::DecorationButtonType> lb = settings()->decorationButtonsLeft();
    int shadowOpacity = m_wd&&!m_wd->isEmpty()?m_wd->value<int>(WindowData::ShadowOpacity):dConf.shadows.opacity;
    if (!shadowOpacity)
        shadowOpacity = 127;
    for (int i = 0; i < lb.count(); ++i)
        if (Button *b = Button::create(lb.at(i), this, m_leftButtons))
        {
            b->setButtonStyle(buttonStyle);
            b->setShadowOpacity(shadowOpacity);
            m_leftButtons->addButton(b);
        }

    m_rightButtons = new KDecoration2::DecorationButtonGroup(this);
    m_rightButtons->setSpacing(4);
    QVector<KDecoration2::DecorationButtonType> rb = settings()->decorationButtonsRight();
    for (int i = 0; i < rb.count(); ++i)
        if (Button *b = Button::create(rb.at(i), this, m_rightButtons))
        {
            b->setButtonStyle(buttonStyle);
            b->setShadowOpacity(shadowOpacity);
            m_rightButtons->addButton(b);
        }
    connect(client().data(), &KDecoration2::DecoratedClient::widthChanged, this, &Deco::widthChanged);
    connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Deco::activeChanged);
    connect(client().data(), &KDecoration2::DecoratedClient::captionChanged, this, &Deco::captionChanged);
}

void
Deco::initMemory()
{
    connect(m_wd, &QObject::destroyed, this, &Deco::dataDestroyed);
    if (m_wd->isEmpty())
    {
        m_wd->setValue<bool>(WindowData::Separator, true);
        m_wd->setValue<bool>(WindowData::Uno, true);
    }
    m_wd->setValue<int>(WindowData::TitleHeight, TITLEHEIGHT);
}

void
Deco::updateData()
{
    if (!m_wd)
    if (m_wd = WindowData::memory(client().data()->windowId(), this))
        initMemory();
    if (m_wd)
    {
        const int buttonStyle = m_wd->value<int>(WindowData::Buttons);
        const int shadowOpacity = m_wd->value<int>(WindowData::ShadowOpacity);
        if (m_leftButtons)
        for (int i = 0; i < m_leftButtons->buttons().count(); ++i)
        {
            Button *b = static_cast<Button *>(m_leftButtons->buttons().at(i).data());
            b->setButtonStyle(buttonStyle);
            b->setShadowOpacity(shadowOpacity);
        }
        if (m_rightButtons)
        for (int i = 0; i < m_leftButtons->buttons().count(); ++i)
        {
            Button *b = static_cast<Button *>(m_rightButtons->buttons().at(i).data());
            b->setButtonStyle(buttonStyle);
            b->setShadowOpacity(shadowOpacity);
        }
    }
    update();
}

void
Deco::checkForDataFromWindowClass()
{
    KWindowInfo info(client().data()->windowId(), NET::WMWindowType|NET::WMVisibleName|NET::WMName, NET::WM2WindowClass);
    Data::decoData(info.windowClassClass(), this);
    updateBgPixmap();
}

void
Deco::widthChanged(const int width)
{
    if (m_leftButtons)
        m_leftButtons->setPos(QPoint(6, client().data()->isModal()?2:4));
    if (m_rightButtons)
        m_rightButtons->setPos(QPointF((width-m_rightButtons->geometry().width())-6, client().data()->isModal()?2:4));
    setTitleBar(QRect(0, 0, width, TITLEHEIGHT));
}

void
Deco::activeChanged(const bool active)
{
    update();
    ShadowHandler::installShadows(client().data()->windowId(), active);
}

void
Deco::paint(QPainter *painter, const QRect &repaintArea)
{
    if (!painter->isActive())
        return;
    painter->save();
    //bg
    bool needPaintBg(true);
    if (m_wd && m_wd->lock())
    {
        painter->setBrushOrigin(titleBar().topLeft());
        const QImage img = m_wd->image();
        if (!img.isNull())
        {
            painter->fillRect(titleBar(), img);
            needPaintBg = false;
        }
        m_wd->unlock();
    }
    if (needPaintBg)
    {
        if (!m_pix.isNull())
            painter->drawTiledPixmap(titleBar(), m_pix);
        else
            painter->fillRect(titleBar(), client().data()->palette().color(QPalette::Window));
    }

    const int bgLum(Color::luminosity(bgColor()));
    const bool isDark(Color::luminosity(fgColor()) > bgLum);

    if ((!dConf.deco.frameSize || client().data()->isMaximized()) && !client().data()->isModal())
        paintBevel(painter, bgLum);

    if ((m_wd && m_wd->value<bool>(WindowData::Separator)) || (!m_wd && m_separator))
    {
        painter->setPen(QColor(0, 0, 0, m_wd->value<int>(WindowData::ShadowOpacity, 32)));
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawLine(0, (TITLEHEIGHT)-1, titleBar().width(), (TITLEHEIGHT)-1);
        painter->setRenderHint(QPainter::Antialiasing, true);
    }

    if (m_wd && m_wd->value<bool>(WindowData::ContAware))
    {
        if (!m_mem)
            m_mem = new QSharedMemory(QString::number(client().data()->windowId()), this);
        if ((m_mem->isAttached() || m_mem->attach(QSharedMemory::ReadOnly)) && m_mem->lock())
        {
            const uchar *data(reinterpret_cast<const uchar *>(m_mem->constData()));
            painter->drawImage(QPoint(0, 0),
                               QImage(data, titleBar().width(), m_wd->value<int>(WindowData::UnoHeight), QImage::Format_ARGB32_Premultiplied),
                               titleBar());
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
    int maxW(titleBar().width());
    if (m_leftButtons && m_rightButtons)
        maxW = titleBar().width()-(qMax(m_leftButtons->geometry().width(), m_rightButtons->geometry().width())*2);
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
    if ((!m_wd && dConf.deco.icon) || (m_wd && m_wd->value<bool>(WindowData::WindowIcon)))
    {
        QRect ir(QPoint(), QSize(16, 16));
        ir.moveTop(titleBar().top()+(titleBar().height()/2-ir.height()/2));
        ir.moveRight(textRect.left()-4);
        if (ir.left() > m_leftButtons->geometry().width())
            client().data()->icon().paint(painter, ir, Qt::AlignCenter, client().data()->isActive()?QIcon::Active:QIcon::Disabled);
    }

    //buttons
    if (m_leftButtons)
        m_leftButtons->paint(painter, repaintArea);
    if (m_rightButtons)
        m_rightButtons->paint(painter, repaintArea);

    painter->restore();
}

void
Deco::paintBevel(QPainter *painter, const int bgLum)
{
    if (m_bevelCorner[0].isNull() || m_prevLum != bgLum)
    {
        m_prevLum = bgLum;
        for (int i = 0; i < 2; ++i)
        {
            m_bevelCorner[i] = QPixmap(5, 5);
            m_bevelCorner[i].fill(Qt::transparent);
        }
        m_bevelCorner[2] = QPixmap(1, 1);
        m_bevelCorner[2].fill(Qt::transparent);

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

const QColor
Deco::bgColor() const
{
    if (m_wd)
    {
        const QColor &c = m_wd->bg();
        if (c.alpha() == 0xff)
            return c;
    }
    if (m_bg.isValid() && m_bg.alpha() == 0xff)
        return m_bg;
    return client().data()->palette().color(QPalette::Window);
}

const QColor
Deco::fgColor() const
{
    if (m_wd)
    {
        const QColor &c = m_wd->fg();
        if (c.alpha() == 0xff)
            return c;
    }
    if (m_fg.isValid() && m_fg.alpha() == 0xff)
        return m_fg;
    return client().data()->palette().color(QPalette::WindowText);
}

void
Deco::updateBgPixmap()
{
    QRect r(0, 0, 1, TITLEHEIGHT);
    QLinearGradient lg(r.topLeft(), r.bottomLeft());
    if (!m_gradient.isEmpty())
        lg.setStops(Settings::gradientStops(m_gradient, bgColor()));
    else
        lg.setStops(QGradientStops() << QGradientStop(0, Color::mid(bgColor(), Qt::white, 4, 1)) << QGradientStop(1, Color::mid(bgColor(), Qt::black, 4, 1)));

    QPixmap p(Render::noise().size().width(), r.height());
    p.fill(Qt::transparent);
    QPainter pt(&p);
    pt.fillRect(p.rect(), lg);
    if (m_noise)
    {
        QPixmap noise(Render::noise().size());
        noise.fill(Qt::transparent);
        QPainter ptt(&noise);
        ptt.drawTiledPixmap(noise.rect(), Render::noise());
        ptt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        ptt.fillRect(noise.rect(), QColor(0, 0, 0, m_noise*2.55f));
        ptt.end();
        pt.setCompositionMode(QPainter::CompositionMode_Overlay);
        pt.drawTiledPixmap(p.rect(), noise);
    }
    pt.end();
    m_pix = p;
}

Grip::Grip(Deco *d)
    : QWidget()
    , m_deco(d)
{
    if (!QX11Info::isPlatformX11())
        return;
    KDecoration2::DecoratedClient *c = m_deco->client().data();
    xcb_window_t windowId = c->windowId();
    if (windowId)
    {
        xcb_window_t current = windowId;
        xcb_query_tree_cookie_t cookie = xcb_query_tree_unchecked(QX11Info::connection(), current);
        xcb_query_tree_reply_t *tree = xcb_query_tree_reply(QX11Info::connection(), cookie, 0);
        if (tree && tree->parent)
            current = tree->parent;
        // reparent
        xcb_reparent_window(QX11Info::connection(), winId(), current, 0, 0);
    }
    else
        hide();
}

void
Grip::updatePosition()
{
    if (!QX11Info::isPlatformX11())
        return;

    KDecoration2::DecoratedClient *c = m_deco->client().data();
    unsigned int values[2] = { c->width() - 16, c->height() - 16 };
    xcb_configure_window(QX11Info::connection(), winId(), XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
}

void
Grip::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::red);
    p.end();
}

void
Grip::mousePressEvent(QMouseEvent *e)
{

}

} //DSP

/*
 * Required for the K_PLUGIN_FACTORY_WITH_JSON vtable
 */
#include "kwinclient2.moc"
