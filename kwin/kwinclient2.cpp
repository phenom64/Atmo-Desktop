#include "kwinclient2.h"
#include "decobutton.h"
#include "decoadaptor.h"
#include "../stylelib/color.h"
#include "../stylelib/xhandler.h"
#include "../stylelib/shadowhandler.h"
#include "../stylelib/gfx.h"
#include "../stylelib/fx.h"
#include "../stylelib/macros.h"
#include "../stylelib/ops.h"

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

class DSPDecoFactory;
static DSPDecoFactory *s_factory(0);
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
        if (!s_factory)
            s_factory = this;
    }
    ~DSPDecoFactory() { if (s_factory == this) s_factory = 0; }

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
    QString fgColor = s.value("fgcolor", "#000000").toString();
    d.fg = QColor(fgColor);
    if (fgColor.size() == QString("0xff000000").size() && fgColor.startsWith("0x")) //old format
        d.fg = QColor::fromRgba(fgColor.toUInt(0, 16));

    QString bgColor = s.value("bgcolor", "#ffffff").toString();
    d.bg = QColor(bgColor);
    if (bgColor.size() == QString("0xffffffff").size() && bgColor.startsWith("0x")) //old format
        d.bg = QColor::fromRgba(bgColor.toUInt(0, 16));

    d.grad = DSP::Settings::stringToGrad(s.value("gradient", "0:10, 1:-10").toString());
    d.noise = s.value("noise", 20).toUInt();
    d.separator = s.value("separator", true).toBool();
    d.btnStyle = s.value("btnstyle", -2).toInt();
    d.icon = s.value("icon", false).toBool();
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
    d->m_icon = data.icon;
    if (data.btnStyle != -2)
        d->m_buttonStyle = data.btnStyle;
    d->recalculate();
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
    , m_tries(0)
    , m_bevel(0)
    , m_illumination(0)
    , m_textBevOpacity(0)
    , m_bling(0)
    , m_shadowOpacity(127)
    , m_contAware(false)
    , m_blingEnabled(false)
    , m_icon(false)
    , m_buttonManager(0)
{
    if (s_factory)
        setParent(s_factory);
}

Deco::~Deco()
{
    if (m_embedder)
        removeEmbedder();
    if (m_grip)
        m_grip->deleteLater();
    if (m_bling)
    {
        delete [] m_bling;
        m_bling = 0;
    }
    if (s_hovered && s_hovered == this)
        s_hovered = 0;
    AdaptorManager::instance()->removeDeco(this);
}

void
Deco::init()
{
    KDecoration2::Decoration::init();
    m_buttonManager = ButtonGroupBase::buttonGroup(client().data()->windowId());
    setBorders(QMargins(0, 0, 0, 0));
    setTitleHeight(client().data()->isModal()?20:25);
    m_bevel = 1;
    m_illumination = 127;
    m_textBevOpacity = 127;

    if (const uint id = client().data()->windowId())
    {
        AdaptorManager::instance()->addDeco(this);
        m_buttonStyle = dConf.deco.buttons;
        if (m_wd = getShm())
            m_buttonStyle = m_wd->value<int>(WindowData::Buttons, m_buttonStyle);
        else
            checkForDataFromWindowClass();
        ShadowHandler::installShadows(id);
    }
    else
        updateBgPixmap();

    m_leftButtons = new KDecoration2::DecorationButtonGroup(this);
    m_leftButtons->setSpacing(4);
    QVector<KDecoration2::DecorationButtonType> lb = settings()->decorationButtonsLeft();
    int shadowOpacity = m_wd&&!m_wd->isEmpty()?m_wd->value<int>(WindowData::ShadowOpacity):dConf.shadows.opacity;
    if (!shadowOpacity)
        shadowOpacity = 127;
    for (int i = 0; i < lb.count(); ++i)
        if (Button *b = Button::create(lb.at(i), this, m_leftButtons))
            m_leftButtons->addButton(b);
    m_rightButtons = new KDecoration2::DecorationButtonGroup(this);
    m_rightButtons->setSpacing(4);
    QVector<KDecoration2::DecorationButtonType> rb = settings()->decorationButtonsRight();
    for (int i = 0; i < rb.count(); ++i)
        if (Button *b = Button::create(rb.at(i), this, m_rightButtons))
            m_rightButtons->addButton(b);

    m_buttonManager->configure(shadowOpacity, m_illumination, m_buttonStyle, 0, Gradient());
    m_buttonManager->setColors(m_bg, m_fg);
    m_buttonManager->clearCache();
    connect(client().data(), &KDecoration2::DecoratedClient::widthChanged, this, &Deco::widthChanged);
    connect(client().data(), &KDecoration2::DecoratedClient::activeChanged, this, &Deco::activeChanged);
    connect(client().data(), &KDecoration2::DecoratedClient::captionChanged, this, &Deco::captionChanged);
    connect(client().data(), &KDecoration2::DecoratedClient::maximizedChanged, this, &Deco::maximizedChanged);

    if (client().data()->isResizeable() && !m_grip)
        m_grip = new Grip(this);
    if (m_wd && m_wd->value<bool>(WindowData::EmbeddedButtons) && !m_embedder)
        m_embedder = new EmbedHandler(this);

    if (m_blingEnabled)
    {
        Ops::swap<QColor>(m_bg, m_fg);
        if (m_bling)
        {
            delete [] m_bling;
            m_bling = 0;
        }
    }
    recalculate();
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

void
Deco::recalculate()
{
    m_isDark = Color::lum(m_bg) < Color::lum(m_fg);
    if (m_bling)
        delete [] m_bling;
    m_bling = 0;
    update();
}

const int
Deco::border()
{
    return 6;
}

void
Deco::maximizedChanged(const bool max)
{
    const int m = max||!m_wd||m_wd->value<bool>(WindowData::Uno, false) ? 0 : border();
    setBorders(QMargins(m, titleBar().height(), m, m));
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
Deco::setButtonsVisible(const bool visible)
{
    if (m_leftButtons)
        for (int i = 0; i < m_leftButtons->buttons().count(); ++i)
            m_leftButtons->buttons().at(i).data()->setVisible(visible);
    if (m_rightButtons)
        for (int i = 0; i < m_rightButtons->buttons().count(); ++i)
            m_rightButtons->buttons().at(i).data()->setVisible(visible);
}

WindowData
*Deco::getShm()
{
    WindowData *data = WindowData::memory(client().data()->windowId(), this);
    if (!data)
        return 0;
//    connect(m_wd, &WindowData::dataChanged, this, &Deco::updateData);
    connect(data, &QObject::destroyed, this, &Deco::dataDestroyed);
    if (data->isEmpty())
    {
        data->setValue<bool>(WindowData::Separator, true);
        data->setValue<bool>(WindowData::Uno, true);
    }
    data->setValue<int>(WindowData::TitleHeight, titleHeight());
    activeChanged(true);
    return data;
}

void
Deco::updateData()
{
    if (!m_wd)
        m_wd = getShm();
    if (m_wd)
    {
        m_illumination      = m_wd->value<uint>(WindowData::Illumination, 127);
        m_textBevOpacity    = m_wd->value<uint>(WindowData::TextBevOpacity, 127);
        m_separator         = m_wd->value<bool>(WindowData::Separator, true);
        m_shadowOpacity     = m_wd->value<uint>(WindowData::ShadowOpacity, 127);
        m_contAware         = m_wd->value<bool>(WindowData::ContAware, false);
        m_icon              = m_wd->value<bool>(WindowData::WindowIcon, false);
        m_buttonStyle       = m_wd->value<int>(WindowData::Buttons);
        m_shadowOpacity     = m_wd->value<int>(WindowData::ShadowOpacity);

        if (m_wd->value<bool>(WindowData::EmbeddedButtons, false) && !m_embedder)
            m_embedder = new EmbedHandler(this);
        else if (!m_wd->value<bool>(WindowData::EmbeddedButtons, false) && m_embedder)
            removeEmbedder();

        setTitleHeight(m_wd->value<int>(WindowData::TitleHeight, 0));
        const bool uno = m_wd->value<bool>(WindowData::Uno, false);
        if (!uno)
        {
            setBorders(QMargins(border(),titleHeight(),border(),border()));
            if (m_grip)
            {
                m_grip->deleteLater();
                m_grip = 0;
            }
            m_winGradient = Settings::gradientStops(m_wd->windowGradient());
            if (m_bevel != 1)
            {
                m_bevel = 1;
                for (int i = 0; i < 3; ++i)
                    m_bevelCorner[i] = QPixmap();
            }
        }

        const QColor &bg = m_wd->bg();
        if (bg.alpha() == 0xff)
            m_bg = bg;
        else
            m_bg = client().data()->palette().color(QPalette::Window);

        const QColor &fg = m_wd->fg();
        if (fg.alpha() == 0xff)
            m_fg = fg;
        else
            m_fg = client().data()->palette().color(QPalette::WindowText);

        m_buttonManager->setColors(m_bg, m_fg, m_wd->minColor(), m_wd->maxColor(), m_wd->closeColor());
        m_buttonManager->configure(m_shadowOpacity, m_illumination, m_buttonStyle, m_wd->value<int>(WindowData::FollowDecoShadow), m_wd->buttonGradient());
        for (int i = 0; i < m_buttonManager->buttons().size(); ++i)
        {
            Button *button = static_cast<Button *>(m_buttonManager->buttons().at(i));
            button->updateGeometry();
        }
        m_buttonManager->clearCache();

//        const bool buttonShouldBeVisible();
        if (m_embedder)
            m_embedder->repaint();
        if (m_leftButtons)
        {
            if (m_buttonStyle == ButtonBase::Anton)
                m_leftButtons->setSpacing(-2);
        }
        if (m_rightButtons)
        {
            if (m_buttonStyle == ButtonBase::Anton)
                m_rightButtons->setSpacing(-2);
        }

        if (m_blingEnabled)
        {
            Ops::swap<QColor>(m_bg, m_fg);
            if (m_bling)
            {
                delete [] m_bling;
                m_bling = 0;
            }
        }
        setButtonsVisible(titleBar().height() > 6);
        m_wd->setDecoId(client().data()->decorationId());
        m_tries = 0;
        recalculate();
    }
    else
    {
        if (++m_tries < 4)
            QTimer::singleShot(250, this, SLOT(updateData()));
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
    {
        const int add = (m_buttonStyle == ButtonBase::Anton) * 2;
        m_leftButtons->setPos(QPoint((borders().left() ? borders().left() : 6)+add, client().data()->isModal()?2:4));
    }
    if (m_rightButtons)
        m_rightButtons->setPos(QPoint((rect().width()-(m_rightButtons->geometry().width()+(borders().right() ? borders().right() : 6))), client().data()->isModal()?2:4));
    setTitleBar(QRect(borders().left(), 0, width, titleHeight()));
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

void
Deco::activeChanged(const bool active)
{
//    update();
//    AdaptorManager::instance()->windowChanged(client().data()->windowId(), active);
    if (m_wd)
    {
        m_wd->setValue<bool>(WindowData::IsActiveWindow, active);
        AdaptorManager::instance()->dataChanged(client().data()->windowId());
    }
    if (m_grip)
        m_grip->setColor(Color::mid(fgColor(), bgColor(), 1, 2));;
    ShadowHandler::installShadows(client().data()->windowId(), active);
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

void
Deco::paint(QPainter *painter, const QRect &repaintArea)
{
    if (!painter->isActive())
        return;
    painter->save();
    //bg
    bool needPaintBg(true);
    const bool uno = m_wd && m_wd->value<bool>(WindowData::Uno, false);
    if (m_wd && m_wd->lock())
    {
        painter->setBrushOrigin(titleBar().topLeft());
        const QImage img = m_wd->image();
        if (!img.isNull())
        {
            QRect r = rect();
            if (uno)
                r = titleBar();
            painter->fillRect(r, img);
            needPaintBg = false;
        }
        m_wd->unlock();
        if (!uno && !m_winGradient.isEmpty())
        {
            const bool hor = m_wd->value<bool>(WindowData::Horizontal, false);
            QLinearGradient lg(rect().topLeft(), hor ? rect().topRight() : rect().bottomLeft());
            lg.setStops(m_winGradient);
            painter->setCompositionMode(QPainter::CompositionMode_Overlay);
            painter->fillRect(rect(), lg);
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            needPaintBg = false;
        }
    }
    if (needPaintBg)
    {
        if (!m_pix.isNull())
            painter->drawTiledPixmap(titleBar(), m_pix);
        else
            painter->fillRect(titleBar(), client().data()->palette().color(QPalette::Window));
    }
    if (m_separator)
        painter->fillRect(0, titleHeight()-1, titleBar().width(), 1, QColor(0, 0, 0, m_shadowOpacity));
    if (m_contAware)
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
    const int bgLum(Color::lum(bgColor()));
    const bool isDark(Color::lum(fgColor()) > bgLum);
    if ((!dConf.deco.frameSize || client().data()->isMaximized()) && !client().data()->isModal() && !client().data()->isMaximized())
        paintBevel(painter, m_illumination/*bgLum*/);
    if (m_blingEnabled)
        paintBling(painter, titleTextArea());

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
            text = painter->fontMetrics().elidedText(text, Qt::ElideRight, maxW, Qt::TextShowMnemonic);
            textRect = painter->fontMetrics().boundingRect(titleBar(), Qt::AlignCenter|Qt::TextHideMnemonic, text);
        }
        if (client().data()->isActive() && m_textBevOpacity)
        {
            const int rgb(isDark?0:255);
            painter->setPen(QColor(rgb, rgb, rgb, m_textBevOpacity));
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
        if (m_icon)
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
//#if 0
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

        QImage tmp(size*2+1, size*2+1, QImage::Format_ARGB32_Premultiplied);
        tmp.fill(Qt::transparent);
        QPainter pt(&tmp);
        pt.setRenderHint(QPainter::Antialiasing);
//        const QRectF bevel(0.5f, 0.5f, tmp.width()-0.5f, tmp.height());
//        QLinearGradient lg(bevel.topLeft(), bevel.bottomLeft());
//        lg.setColorAt(0.0f, QColor(255, 255, 255, qMin(255/*.0f*/, bgLum/**1.1f*/)));
//        lg.setColorAt(0.5f, Qt::transparent);
        pt.setBrush(QColor(255, 255, 255, qMin(255/*.0f*/, bgLum/**1.1f*/)));
        pt.setPen(Qt::NoPen);
        pt.drawEllipse(tmp.rect());
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.setBrush(Qt::black);
        pt.drawEllipse(tmp.rect().translated(0, m_bevel));
        pt.end();

        QImage tmp2 = tmp;
        FX::expblur(tmp, 2);
        pt.begin(&tmp);
        pt.drawImage(0,0,tmp2);
        pt.end();
        m_bevelCorner[0] = QPixmap::fromImage(tmp.copy(QRect(0, 0, size, size)));
        m_bevelCorner[1] = QPixmap::fromImage(tmp.copy(QRect(size+1, 0, size, size)));
        m_bevelCorner[2] = QPixmap::fromImage(tmp.copy(size, 0, 1, 4));
    }
    painter->drawPixmap(QRect(rect().topLeft(), m_bevelCorner[0].size()), m_bevelCorner[0]);
    painter->drawPixmap(QRect(rect().topRight()-QPoint(m_bevelCorner[1].width()-1, 0), m_bevelCorner[1].size()), m_bevelCorner[1]);
    painter->drawTiledPixmap(rect().adjusted(m_bevelCorner[0].width(), 0, -m_bevelCorner[1].width(), -(rect().height()-4)), m_bevelCorner[2]);
//#endif
}

void
Deco::paintBling(QPainter *painter, const QRect &r)
{
    static int rad(16);
    if (!m_bling)
    {
        m_bling = new QPixmap[3]();

        const int mid(16);
        QRect rect(0, 0, rad*2+mid, r.height());
        QImage tmp(rect.size()+QSize(8,8), QImage::Format_ARGB32_Premultiplied);
        tmp.fill(Qt::transparent);
        QPainter p(&tmp);
        int blurRad(4);
        QRect pathRect = rect.adjusted(blurRad, 0, -blurRad, -blurRad);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath bling = blingPath(1, pathRect, rad);

        int mode(m_isDark);
        switch (mode)
        {
        case 0:
        {
            p.fillPath(bling, Qt::black);
            p.end();

            FX::expblur(tmp, blurRad);

            tmp = tmp.copy(0,0,tmp.width()-8, tmp.height()-8);
            p.begin(&tmp);
            p.setRenderHint(QPainter::Antialiasing);
            const QColor c = bgColor();
            QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
            lg.setColorAt(0, Color::mid(c, Qt::white, 2, 2));
            lg.setColorAt(1, Color::mid(c, Qt::black, 2, 1));
            p.fillPath(bling, lg);
            break;
        }
        case 1:
        {
            p.fillRect(pathRect.adjusted(0,0,0,blurRad), Qt::black);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.fillPath(bling, Qt::black);
            p.end();

            QImage tmp2(tmp.size(), QImage::Format_ARGB32_Premultiplied);
            tmp2.fill(Qt::transparent);
            p.begin(&tmp2);
            p.setRenderHint(QPainter::Antialiasing);
            p.fillPath(bling, Qt::black);
            p.end();

            FX::expblur(tmp, 2);

            tmp = tmp.copy(0,0,tmp.width()-8, tmp.height()-8);
            p.begin(&tmp);
            p.setRenderHint(QPainter::Antialiasing);
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
//            p.fillPath(bling, Qt::black);

            p.drawImage(0,0,tmp2);

    //        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);

            const QColor c = bgColor();
            QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
            lg.setColorAt(0, Color::mid(c, Qt::white, 2, 2));
            lg.setColorAt(1, Color::mid(c, Qt::black, 2, 1));

            p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
            p.fillPath(bling, lg);
            break;
        }
        default: break;
        }
        p.end();
        m_bling[0] = QPixmap::fromImage(tmp.copy(0, 0, rad+blurRad, r.height()));
        m_bling[1] = QPixmap::fromImage(tmp.copy(rad+blurRad, 0, 1, r.height()));
        m_bling[2] = QPixmap::fromImage(tmp.copy(rad+mid-blurRad, 0, rad+blurRad, r.height()));
    }
    painter->drawPixmap(r.x(), r.y(), m_bling[0].width(), m_bling[0].height(), m_bling[0]);
    painter->drawTiledPixmap(r.x()+m_bling[0].width(), r.y(), r.width()-m_bling[2].width()*2, r.height(), m_bling[1]);
    painter->drawPixmap(r.x()+r.width()-m_bling[2].width(), r.y(), m_bling[2].width(), m_bling[2].height(), m_bling[2]);
}

const QRect
Deco::titleTextArea() const
{
    int left = m_leftButtons ? m_leftButtons->geometry().right()+1 : titleBar().left();
    int right = (m_rightButtons ? m_rightButtons->geometry().left() : titleBar().right()) - left;
    return QRect(left, 0, right, titleBar().height());
}

const QPainterPath
Deco::blingPath(const quint8 style, const QRectF &r, const int radius) const
{
    QPainterPath path;
    qreal x,y,w,h;
    r.getRect(&x, &y, &w, &h);
    switch (style)
    {
    case 0:
    {
        path.moveTo(x,y);
        path.lineTo(x+radius, y+h);
        path.lineTo(x+w-radius, y+h);
        path.lineTo(x+w, y);
        path.closeSubpath();
        break;
    }
    case 1:
    {
        const qreal rad(radius/2.0f);
        path.moveTo(x,y);
        path.quadTo(x+rad, y, x+rad, y+(h/2.0f));
        path.quadTo(x+rad, y+h, x+radius, y+h);
        path.lineTo(x+w-radius, y+h);
        path.quadTo(x+w-rad, y+h, x+w-rad, y+(h/2.0f));
        path.quadTo(x+w-rad, y, x+w, y);
        path.closeSubpath();
    }
    default: break;
    }
    return path;
}

const QColor
Deco::bgColor() const
{
    return m_bg;
}

const QColor
Deco::fgColor() const
{
    return m_fg;
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
//        qDebug() << event;
//        return true;
        break;
    default: break;
    }
    return KDecoration2::Decoration::event(event);
}

void
Deco::wheelEvent(QWheelEvent *event)
{
//    qDebug() << "wheelEvent" << event;
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

QPolygon
Grip::shape()
{
    static const QPolygon &p = QPolygon() << QPoint(Size, 0) << QPoint(Size, Size) << QPoint(0, Size); //topright, bottomright, bottomleft
    return p;
}

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
    setMask(shape());
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
    regenPix();
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

void
Grip::regenPix()
{
    QImage img(size(), QImage::Format_ARGB32);
    QPainter p(&img);
    p.setPen(QPen(QColor(0, 0, 0, 255), 2.0f));
    p.setBrush(palette().color(backgroundRole()));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPolygon(shape());
    p.end();
    FX::expblur(img, 2);
    m_pix = QPixmap::fromImage(img);
}

void
Grip::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(rect(), m_pix);
}

} //DSP

/*
 * Required for the K_PLUGIN_FACTORY_WITH_JSON vtable
 */
#include "kwinclient2.moc"
