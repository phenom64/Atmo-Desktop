#include "kwinclient2.h"
#include "decobutton.h"
#include "decoadaptor.h"
#include "../stylelib/color.h"
#include "../stylelib/xhandler.h"
#include "../stylelib/shadowhandler.h"
#include "../stylelib/gfx.h"

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
#include <KWindowSystem>

#include <QEvent>
#include <QMouseEvent>

#include <QTimer>
#include <QSharedMemory>

#include <QDBusConnection>

#include <QPixmap>

#include <QSettings>
#include <QDir>
#include <QWindow>
#include <qmath.h>

#include <QDebug>
#if HASXCB
#include <QX11Info>
#endif

//K_PLUGIN_FACTORY_WITH_JSON(
//    DSPDecoFactory,
//    "dsp.json",
//    registerPlugin<DSP::Deco>();
//    registerPlugin<DSP::Button>(QStringLiteral("button"));
//    registerPlugin<DSP::ConfigModule>(QStringLiteral("kcmodule"));
//)


class DSPDecoFactory : public KPluginFactory
{
    Q_OBJECT
    Q_INTERFACES(KPluginFactory)
    Q_PLUGIN_METADATA(IID KPluginFactory_iid FILE "dsp.json")
public:
    explicit DSPDecoFactory()
    {
        registerPlugin<DSP::Deco>();
        registerPlugin<DSP::Button>(QStringLiteral("button"));
        registerPlugin<DSP::ConfigModule>(QStringLiteral("kcmodule"));
        DSP::Deco::Data::readWindowData();
//        DSP::SharedDataAdaptorManager::instance();
        DSP::Settings::read();
        DSP::XHandler::init();
        DSP::ShadowHandler::removeDelete();
        DSP::GFX::generateData();
    }
    ~DSPDecoFactory() {}

//protected:
//    QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
//    {
//        qDebug() << iface << parentWidget << parent << args << keyword;
//        return KPluginFactory::create(iface, parentWidget, parent, args, keyword);
//    }
};

namespace DSP
{

ConfigModule::ConfigModule(QWidget *parent, const QVariantList &args) : KCModule(parent, args)
{

}

///-------------------------------------------------------------------------------------------------

QMap<QString, Deco::Data> Deco::Data::s_data;

void
Deco::Data::addDataForWinClass(const QString &winClass, QSettings &s)
{
    Data d;
    d.fg = QColor::fromRgba(s.value("fgcolor", "0x00000000").toString().toUInt(0, 16));
    d.bg = QColor::fromRgba(s.value("bgcolor", "0x00000000").toString().toUInt(0, 16));
    d.grad = DSP::Settings::stringToGrad(s.value("gradient", "0:10, 1:-10").toString());
    d.noise = s.value("noise", 20).toUInt();
    d.separator = s.value("separator", true).toBool();
    d.btnStyle = s.value("btnstyle", -2).toInt();
    Data::s_data.insert(winClass, d);
}

void
Deco::Data::readWindowData()
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
    if (data.btnStyle != -2)
        d->m_buttonStyle = data.btnStyle;
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
    m_adaptor = new DecoAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.kde.dsp.kwindeco");
    QDBusConnection::sessionBus().registerObject("/DSPDecoAdaptor", this);
}

AdaptorManager::~AdaptorManager()
{
    QDBusConnection::sessionBus().unregisterService("org.kde.dsp.kwindeco");
    QDBusConnection::sessionBus().unregisterObject("/DSPDecoAdaptor");
    s_instance = 0;
    m_adaptor = 0;
}

void
AdaptorManager::windowChanged(uint win, bool active)
{
    if (m_adaptor)
        emit m_adaptor->windowActiveChanged(win, active);
}

void
AdaptorManager::dataChanged(uint win)
{
    if (m_adaptor)
        emit m_adaptor->dataChanged(win);
}

///-------------------------------------------------------------------------------------------------

static Deco *s_hovered(0);

Deco::Deco(QObject *parent, const QVariantList &args)
    : KDecoration2::Decoration(parent, args)
    , m_leftButtons(0)
    , m_rightButtons(0)
    , m_prevLum(0)
    , m_mem(0)
    , m_noise(0)
    , m_separator(true)
    , m_wd(0)
    , m_grip(0)
    , m_buttonStyle(0)
    , m_isHovered(false)
    , m_embedder(0)
{
}

Deco::~Deco()
{
    if (m_embedder)
        removeEmbedder();
    if (m_grip)
        m_grip->deleteLater();
    if (s_hovered && s_hovered == this)
        s_hovered = 0;
    AdaptorManager::instance()->removeDeco(this);
}

void
Deco::init()
{
    setBorders(QMargins(0, 0, 0, 0));
    setTitleHeight(client().data()->isModal()?20:25);

    if (const uint id = client().data()->windowId())
    {
        AdaptorManager::instance()->addDeco(this);
        m_buttonStyle = dConf.deco.buttons;
        initMemory(WindowData::memory(id, this));
        if (m_wd)
            m_buttonStyle = m_wd->value<int>(WindowData::Buttons, m_buttonStyle);
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
            b->setButtonStyle(m_buttonStyle);
            b->setShadowOpacity(shadowOpacity);
            m_leftButtons->addButton(b);
        }

    m_rightButtons = new KDecoration2::DecorationButtonGroup(this);
    m_rightButtons->setSpacing(4);
    QVector<KDecoration2::DecorationButtonType> rb = settings()->decorationButtonsRight();
    for (int i = 0; i < rb.count(); ++i)
        if (Button *b = Button::create(rb.at(i), this, m_rightButtons))
        {
            b->setButtonStyle(m_buttonStyle);
            b->setShadowOpacity(shadowOpacity);
            m_rightButtons->addButton(b);
        }
    connect(client().data(), &KDecoration2::DecoratedClient::widthChanged, this, &Deco::widthChanged);
    connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Deco::activeChanged);
    connect(client().data(), &KDecoration2::DecoratedClient::captionChanged, this, &Deco::captionChanged);

    if (client().data()->isResizeable() && !m_grip)
        m_grip = new Grip(this);
    if (m_wd && m_wd->value<bool>(WindowData::EmbeddedButtons) && !m_embedder)
        m_embedder = new EmbedHandler(this);
}

void
Deco::removeEmbedder()
{
    if (!m_embedder)
        return;
    delete m_embedder;
    m_embedder = 0;
}

void
Deco::initMemory(WindowData *data)
{
    if (!data)
        return;
    m_wd = data;
//    connect(m_wd, &WindowData::dataChanged, this, &Deco::updateData);
    connect(m_wd, &QObject::destroyed, this, &Deco::dataDestroyed);
    if (m_wd->isEmpty())
    {
        m_wd->setValue<bool>(WindowData::Separator, true);
        m_wd->setValue<bool>(WindowData::Uno, true);
    }
    m_wd->setValue<int>(WindowData::TitleHeight, titleHeight());
    activeChanged(true);
}

void
Deco::updateData()
{
    if (!m_wd)
        initMemory(WindowData::memory(client().data()->windowId(), this, true));
    if (m_wd)
    {
        if (m_wd->value<bool>(WindowData::EmbeddedButtons, false) && !m_embedder)
            m_embedder = new EmbedHandler(this);
        else if (!m_wd->value<bool>(WindowData::EmbeddedButtons, false) && m_embedder)
            removeEmbedder();

        setTitleHeight(m_wd->value<int>(WindowData::TitleHeight, 0));
        const bool buttonShouldBeVisible(titleHeight()>6);
        const int buttonStyle = m_wd->value<int>(WindowData::Buttons);
        const int shadowOpacity = m_wd->value<int>(WindowData::ShadowOpacity);

        if (m_embedder)
        {
            m_embedder->setButtonStyle(buttonStyle);
            m_embedder->setButtonShadowOpacity(shadowOpacity);
            m_embedder->repaint();
        }

        if (m_leftButtons)
            for (int i = 0; i < m_leftButtons->buttons().count(); ++i)
            {
                QPointer<KDecoration2::DecorationButton> button = m_leftButtons->buttons().at(i);
                if (button)
                {
                    if (Button *b = qobject_cast<Button *>(button.data()))
                    {
                        b->setButtonStyle(buttonStyle);
                        b->setShadowOpacity(shadowOpacity);
                        b->setVisible(buttonShouldBeVisible);
                    }
                }
            }
        if (m_rightButtons)
            for (int i = 0; i < m_rightButtons->buttons().count(); ++i)
            {
                QPointer<KDecoration2::DecorationButton> button = m_rightButtons->buttons().at(i);
                if (button)
                {
                    if (Button *b = qobject_cast<Button *>(button.data()))
                    {
                        b->setButtonStyle(buttonStyle);
                        b->setShadowOpacity(shadowOpacity);
                        b->setVisible(buttonShouldBeVisible);
                    }
                }
            }
        m_wd->setDecoId(client().data()->decorationId());
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

const int
Deco::titleHeight() const
{
    return titleBar().height();
}

void
Deco::setTitleHeight(const int h)
{
    QRect tb(titleBar());
    tb.setHeight(h);
    setTitleBar(tb);
    QMargins b(borders());
    b.setTop(h);
    setBorders(b);
}

void
Deco::widthChanged(const int width)
{
    if (m_leftButtons)
        m_leftButtons->setPos(QPoint(6, client().data()->isModal()?2:4));
    if (m_rightButtons)
        m_rightButtons->setPos(QPointF((width-m_rightButtons->geometry().width())-6, client().data()->isModal()?2:4));
    setTitleBar(QRect(0, 0, width, titleHeight()));
}

void
Deco::activeChanged(const bool active)
{
    update();
//    AdaptorManager::instance()->windowChanged(client().data()->windowId(), active);
    if (m_wd)
    {
        m_wd->setValue<bool>(WindowData::IsActiveWindow, active);
        AdaptorManager::instance()->dataChanged(client().data()->windowId());
        qDebug() << active << client().data()->isActive() << client().data()->windowId();
    }
    if (m_grip)
        m_grip->setColor(Color::mid(fgColor(), bgColor(), 2, 1));;
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
        painter->drawLine(0, (titleHeight())-1, titleBar().width(), (titleHeight())-1);
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

#if 0
    //shape
    if (!client().data()->isModal())
        Render::shapeCorners(painter, Top|Right|Left);
#endif
    //text

    const bool paintText(titleBar().height() > 19);

    if (paintText)
    {

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
#if 0
    if (m_bevelCorner[0].isNull() || m_prevLum != bgLum)
    {
        static const int size(5);
        m_prevLum = bgLum;
        for (int i = 0; i < 2; ++i)
        {
            m_bevelCorner[i] = QPixmap(size, size);
            m_bevelCorner[i].fill(Qt::transparent);
        }
        m_bevelCorner[2] = QPixmap(1, 1);
        m_bevelCorner[2].fill(Qt::transparent);

        QPixmap tmp(size*2+1, size*2+1);
        tmp.fill(Qt::transparent);
        QPainter pt(&tmp);
        pt.setRenderHint(QPainter::Antialiasing);
        const QRectF bevel(0.5f, 0.5f, tmp.width()-0.5f, tmp.height());
        QLinearGradient lg(bevel.topLeft(), bevel.bottomLeft());
        lg.setColorAt(0.0f, QColor(255, 255, 255, qMin(255/*.0f*/, bgLum/**1.1f*/)));
        lg.setColorAt(0.5f, Qt::transparent);
        pt.setBrush(lg);
        pt.setPen(Qt::NoPen);
        pt.drawEllipse(tmp.rect());
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.setBrush(Qt::black);
        pt.drawEllipse(tmp.rect().adjusted(1, 1, -1, -1));
        pt.end();
        m_bevelCorner[0] = tmp.copy(QRect(0, 0, size, size));
        m_bevelCorner[1] = tmp.copy(QRect(size+1, 0, size, size));
        m_bevelCorner[2] = tmp.copy(size, 0, 1, 1);
    }
    painter->drawPixmap(QRect(titleBar().topLeft(), m_bevelCorner[0].size()), m_bevelCorner[0]);
    painter->drawPixmap(QRect(titleBar().topRight()-QPoint(m_bevelCorner[1].width()-1, 0), m_bevelCorner[1].size()), m_bevelCorner[1]);
    painter->drawTiledPixmap(titleBar().adjusted(m_bevelCorner[0].width(), 0, -m_bevelCorner[1].width(), -(titleBar().height()-1)), m_bevelCorner[2]);
#endif
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
    QRect r(0, 0, 1, titleHeight());
    QLinearGradient lg(r.topLeft(), r.bottomLeft());
    if (!m_gradient.isEmpty())
        lg.setStops(DSP::Settings::gradientStops(m_gradient, bgColor()));
    else
        lg.setStops(QGradientStops() << QGradientStop(0, Color::mid(bgColor(), Qt::white, 4, 1)) << QGradientStop(1, Color::mid(bgColor(), Qt::black, 4, 1)));

    QPixmap p(GFX::noise().size().width(), r.height());
    p.fill(Qt::transparent);
    QPainter pt(&p);
    pt.fillRect(p.rect(), lg);
    if (m_noise)
    {
        QPixmap noise(GFX::noise().size());
        noise.fill(Qt::transparent);
        QPainter ptt(&noise);
        ptt.drawTiledPixmap(noise.rect(), GFX::noise());
        ptt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        ptt.fillRect(noise.rect(), QColor(0, 0, 0, m_noise*2.55f));
        ptt.end();
        pt.setCompositionMode(QPainter::CompositionMode_Overlay);
        pt.drawTiledPixmap(p.rect(), noise);
    }
    pt.end();
    m_pix = p;
}

bool
Deco::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::HoverMove:
    {
        if (s_hovered != this)
        {
            if (s_hovered)
                s_hovered->hoverLeave();
            s_hovered = this;
        }
        const QPoint p(static_cast<QHoverEvent *>(event)->pos());
        if (!m_isHovered && titleBar().contains(p))
            hoverEnter();
        else if (m_isHovered && !titleBar().contains(p))
            hoverLeave();
        break;
    }
    case QEvent::Wheel:
        qDebug() << event;
        return true;
        break;
    default: break;
    }
    return KDecoration2::Decoration::event(event);
}

void
Deco::wheelEvent(QWheelEvent *event)
{
    qDebug() << "wheelEvent" << event;
}

void
Deco::hoverEnter()
{
    m_isHovered = true;
    if (m_leftButtons)
        for (int i = 0; i < m_leftButtons->buttons().count(); ++i)
        {
            QPointer<KDecoration2::DecorationButton> button = m_leftButtons->buttons().at(i);
            if (button)
                if (Button *b = qobject_cast<Button *>(button.data()))
                    b->hover();
        }
    if (m_rightButtons)
        for (int i = 0; i < m_rightButtons->buttons().count(); ++i)
        {
            QPointer<KDecoration2::DecorationButton> button = m_rightButtons->buttons().at(i);
            if (button)
                if (Button *b = qobject_cast<Button *>(button.data()))
                    b->hover();
        }
}

void
Deco::hoverLeave()
{
    m_isHovered = false;
    if (m_leftButtons)
        for (int i = 0; i < m_leftButtons->buttons().count(); ++i)
        {
            QPointer<KDecoration2::DecorationButton> button = m_leftButtons->buttons().at(i);
            if (button)
                if (Button *b = qobject_cast<Button *>(button.data()))
                    b->unhover();
        }
    if (m_rightButtons)
        for (int i = 0; i < m_rightButtons->buttons().count(); ++i)
        {
            QPointer<KDecoration2::DecorationButton> button = m_rightButtons->buttons().at(i);
            if (button)
                if (Button *b = qobject_cast<Button *>(button.data()))
                    b->unhover();
        }
}

//----------------------------------------------------------------------------------

Grip::Grip(Deco *d)
    : QWidget(0)
    , m_deco(d)
{
#if HASXCB
    if (!QX11Info::isPlatformX11() || !m_deco)
    {
        hide();
        return;
    }
#else
    hide();
    return;
#endif
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(Size, Size);
    setCursor(Qt::SizeFDiagCursor);

    static const QPolygon &p = QPolygon() << QPoint(Size, 0) << QPoint(Size, Size) << QPoint(0, Size); //topright, bottomright, bottomleft
    setMask(p);

    KDecoration2::DecoratedClient *c = m_deco->client().data();
    connect(c, &KDecoration2::DecoratedClient::heightChanged, this, &Grip::updatePosition);
    connect(c, &KDecoration2::DecoratedClient::widthChanged, this, &Grip::updatePosition);
    restack();
    updatePosition();
    show();
}

void
Grip::setColor(const QColor &c)
{
    QPalette p = m_deco->client().data()->palette();
    p.setColor(backgroundRole(), c);
    setPalette(p);
    setAutoFillBackground(true);
}

void
Grip::updatePosition()
{
    KDecoration2::DecoratedClient *c = m_deco->client().data();
    XHandler::move(winId(), QPoint(c->width()-(Size+Margin), c->height()-(Size+Margin)));
}

void
Grip::restack()
{
    if (XHandler::XWindow windowId = m_deco->client().data()->windowId())
        XHandler::restack(winId(), windowId);
    else
        hide();
}

void
Grip::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);
    XHandler::mwRes(e->pos(), e->globalPos(), winId(), true, m_deco->client().data()->windowId());
}

} //DSP

/*
 * Required for the K_PLUGIN_FACTORY_WITH_JSON vtable
 */
#include "kwinclient2.moc"
