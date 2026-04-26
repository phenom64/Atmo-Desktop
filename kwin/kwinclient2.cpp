/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**************************************************************************
*   Based on styleproject, Copyright (C) 2013 by Robert Metsaranta        *
*   therealestrob@gmail.com                                              *
***************************************************************************/
#include "kwinclient2.h"
#include "decobutton.h"
#include "decoadaptor.h"
#include "decorationshadowcache.h"
#include "menubar.h"
#include "../atmolib/color.h"
#include "../atmolib/xhandler.h"
#include "../atmolib/shadowhandler.h"
#include "../atmolib/gfx.h"
#include "../atmolib/fx.h"
#include "../atmolib/debug.h"
#include "../atmolib/macros.h"
#include "../atmolib/ops.h"


#include <KConfigGroup>
#include <KColorUtils>
#include <KSharedConfig>

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
#include <QMenu>
#include <QWindow>
#include <QGuiApplication>
#include <QScreen>
#include <qmath.h>

#include <QDebug>
#if HASXCB || HASX11
#include "../atmolib/atmox11.h"
#if HASXCB
#include "xcbatoms.h"
#endif
#endif

#if HASDBUSMENU
#include <dbusmenuimporter.h>
#endif

//K_PLUGIN_FACTORY_WITH_JSON(
//    DSPDecoFactory,
//    "dsp.json",
//    registerPlugin<NSE::Deco>();
//    //    //)

static DSPDecoFactory *s_factory(0);
DSPDecoFactory::DSPDecoFactory()
{
    registerPlugin<NSE::Deco>();
    NSE::Deco::Data::readWindowData();
    //        NSE::SharedDataAdaptorManager::instance();
    NSE::Settings::read();
    NSE::XHandler::init();
    NSE::ShadowHandler::removeDelete();
    NSE::GFX::generateData();
    QTimer::singleShot(2000, this, SLOT(shapeCorners()));
    if (!s_factory)
        s_factory = this;
}
DSPDecoFactory::~DSPDecoFactory()
{
    if (s_factory == this)
        s_factory = 0;
}

void
DSPDecoFactory::shapeCorners()
{
    static const QString destination(QStringLiteral("org.kde.KWin"));
    static const QString path(QStringLiteral("/ShapeCorners"));
    static const QString interface(QStringLiteral("org.kde.kwin"));
    static const QString method(QStringLiteral("setRoundness"));
    QDBusMessage msg = QDBusMessage::createMethodCall(destination, path, interface, method);
    msg << dConf.deco.shadowRnd;
    QDBusConnection::sessionBus().send(msg);
}

namespace NSE
{

///-------------------------------------------------------------------------------------------------

static qreal snapDecoMetric(qreal value, qreal scale)
{
    if (value <= 0.0)
        return 0.0;
    return qMax(KDecoration3::snapToPixelGrid(value, scale), KDecoration3::pixelSize(scale));
}

static qreal decoScale(const Deco *deco)
{
    return deco && deco->window() ? deco->window()->nextScale() : 1.0;
}

static qreal decorationDpr()
{
    if (QScreen *screen = QGuiApplication::primaryScreen())
        return qMax<qreal>(1.0, screen->devicePixelRatio());
    return 1.0;
}

QMap<QString, Deco::Data> Deco::Data::s_data;

void
Deco::Data::addDataForWinClass(const QString &winClass, QSettings &s)
{
    Data d;
    QString fgColor = s.value(QStringLiteral("fgcolor"), QStringLiteral("#000000")).toString();
    d.fg = QColor(fgColor);
    if (fgColor.size() == QStringLiteral("0xff000000").size() && fgColor.startsWith(QStringLiteral("0x"))) //old format
        d.fg = QColor::fromRgba(fgColor.toUInt(0, 16));

    QString bgColor = s.value(QStringLiteral("bgcolor"), QStringLiteral("#ffffff")).toString();
    d.bg = QColor(bgColor);
    if (bgColor.size() == QStringLiteral("0xffffffff").size() && bgColor.startsWith(QStringLiteral("0x"))) //old format
        d.bg = QColor::fromRgba(bgColor.toUInt(0, 16));

    d.grad = NSE::Settings::stringToGrad(s.value(QStringLiteral("gradient"), QStringLiteral("0:10, 1:-10")).toString());
    d.noise = s.value(QStringLiteral("noise"), 20).toUInt();
    d.separator = s.value(QStringLiteral("separator"), true).toBool();
    d.btnStyle = s.value(QStringLiteral("btnstyle"), -2).toInt();
    d.icon = s.value(QStringLiteral("icon"), false).toBool();
    d.illumination = s.value(QStringLiteral("illumination"), 63).toInt();
    d.noiseStyle = s.value(QStringLiteral("noiseStyle"), 0).toInt();
    d.menubar = s.value(QStringLiteral("menubar"), false).toBool();
    Data::s_data.insert(winClass, d);
}

void
Deco::Data::readWindowData()
{
    s_data.clear();
    static const QString confPath(QStringLiteral("%1/.config/NSE").arg(QDir::homePath()));
    QSettings s(QStringLiteral("%1/NSEdeco.conf").arg(confPath), QSettings::IniFormat);
    bool hasDefault(false);
    const QStringList groups = s.childGroups();
    for (const QString &winClass : groups)
    {
        s.beginGroup(winClass);
        hasDefault |= (winClass == QStringLiteral("default"));
        addDataForWinClass(winClass, s);
        s.endGroup();
    }
    if (!hasDefault)
        addDataForWinClass(QStringLiteral("default"), s);
}

void
Deco::Data::decoData(const QString &winClass, Deco *d)
{
    Deco::Data data;
    if (s_data.contains(winClass))
        data = s_data.value(winClass);
    else
        data = s_data.value(QStringLiteral("default"));
    d->m_bg = data.bg;
    d->m_fg = data.fg;
    d->m_gradient = data.grad;
    d->m_noise = data.noise;
    d->m_separator = data.separator;
    d->m_icon = data.icon;
    d->m_illumination = data.illumination;
    d->m_noiseStyle = data.noiseStyle;
    d->m_showMenuBar = data.menubar;
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
    QDBusConnection::sessionBus().registerService(QStringLiteral("com.syndromatic.atmo.kwindeco"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/NSEDecoAdaptor"), this);
}

AdaptorManager::~AdaptorManager()
{
    QDBusConnection::sessionBus().unregisterService(QStringLiteral("com.syndromatic.atmo.kwindeco"));
    QDBusConnection::sessionBus().unregisterObject(QStringLiteral("/NSEDecoAdaptor"));
    s_instance = 0;
    m_adaptor = 0;
}

void
AdaptorManager::windowChanged(uint win, bool active)
{
    if (m_adaptor)
        Q_EMIT m_adaptor->windowActiveChanged(win, active);
}

void
AdaptorManager::dataChanged(uint win)
{
    if (m_adaptor)
        Q_EMIT m_adaptor->dataChanged(win);
}

///-------------------------------------------------------------------------------------------------

static Deco *s_hovered(0);

Deco::Deco(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
    , m_leftButtons(0)
    , m_rightButtons(0)
    , m_prevLum(0)
    , m_mem(0)
    , m_noise(0)
    , m_separator(true)
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
    , m_opacity(0xff)
    , m_uno(true)
    , m_fg(Qt::black)
    , m_bg(Qt::white)
    , m_hor(false)
    , m_embedButtons(false)
    , m_followDecoShadow(false)
    , m_titleHeight(0)
    , m_noiseStyle(0)
    , m_showMenuBar(false)
    #if HASDBUSMENU
    , m_menuBar(0)
    #endif
    , m_hasSharedMem(false)
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

#if defined(ATMO_USE_KDECORATION3)
bool
Deco::init()
#else
void
Deco::init()
#endif
{
#if !defined(ATMO_USE_KDECORATION3)
    KDecoration3::Decoration::init();
#endif
    m_buttonManager = ButtonGroupBase::buttonGroup(decoKey());
    ATMO_LOG << "KDecoration init key=" << decoKey() << " winId=" << winId();
    setBorders(AtmoDecoMargins(0, 0, 0, 0));
    setTitleHeight(window()->isModal() ? Ops::dpiScaled(nullptr, 20) : Ops::dpiScaled(nullptr, 25));
    m_bevel = 1;
    m_illumination = 127;
    m_textBevOpacity = 127;

    int shadowOpacity = dConf.shadows.opacity;

    if (!shadowOpacity)
        shadowOpacity = 127;

    AdaptorManager::instance()->addDeco(this);
    m_buttonStyle = dConf.deco.buttons;
    checkForDataFromWindowClass();
    QTimer::singleShot(1000, this, &Deco::checkForDataFromWindowClass); //spotify libreoffice... sets the windowclass late
#if defined(ATMO_USE_KDECORATION3)
    applyNativeShadow();
#else
    if (const uint id = winId())
    {
        ShadowHandler::installShadows(id);
    }
    else
        updateBgPixmap();
#endif

    connect(window(), &AtmoDecoratedWindow::widthChanged, this, [this]() { updateLayout(); });
    connect(window(), &AtmoDecoratedWindow::activeChanged, this, &Deco::activeChanged);
    connect(window(), &AtmoDecoratedWindow::captionChanged, this, &Deco::captionChanged);
    connect(window(), &AtmoDecoratedWindow::maximizedChanged, this, &Deco::maximizedChanged);
    connect(window(), &AtmoDecoratedWindow::heightChanged, this, [this]() { updateLayout(); });
    connect(window(), &AtmoDecoratedWindow::paletteChanged, this, [this]() { updateBgPixmap(); applyNativeShadow(); update(); });
    connect(window(), &AtmoDecoratedWindow::nextScaleChanged, this, [this]() {
        setTitleHeight(m_titleHeight);
        updateLayout();
        applyNativeShadow();
    });

#if defined(ATMO_USE_KDECORATION3)
    const auto decoSettings = settings();
    connect(decoSettings.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, [this]() { reconfigure(); });
    connect(decoSettings.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, [this]() { reconfigure(); });
    connect(decoSettings.get(), &KDecoration3::DecorationSettings::fontChanged, this, [this]() { m_font = settings()->font(); update(); });
    m_font = decoSettings->font();
#else
    connect(settings().data(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, [this]() { reconfigure(); });
    connect(settings().data(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, [this]() { reconfigure(); });
    connect(settings().data(), &KDecoration3::DecorationSettings::fontChanged, this, [this]() { m_font = settings().data()->font(); update(); });
    m_font = settings().data()->font();
#endif

    if (window()->isResizeable() && winId() && !m_grip)
        m_grip = new Grip(this);

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
    reconfigure();
    updateData();
#if defined(ATMO_USE_KDECORATION3)
    return true;
#endif
}

void
Deco::reconfigure()
{
    if (m_leftButtons)
        delete m_leftButtons;
    m_leftButtons = new KDecoration3::DecorationButtonGroup(this);
    m_leftButtons->setSpacing(Ops::dpiScaled(nullptr, 4));
    QVector<KDecoration3::DecorationButtonType> lb = settings()->decorationButtonsLeft();

    for (int i = 0; i < lb.count(); ++i)
        if (Button *b = Button::create(lb.at(i), this, m_leftButtons))
            m_leftButtons->addButton(b);
    if (m_rightButtons)
        delete m_rightButtons;
    m_rightButtons = new KDecoration3::DecorationButtonGroup(this);
    m_rightButtons->setSpacing(Ops::dpiScaled(nullptr, 4));
    QVector<KDecoration3::DecorationButtonType> rb = settings()->decorationButtonsRight();
    for (int i = 0; i < rb.count(); ++i)
        if (Button *b = Button::create(rb.at(i), this, m_rightButtons))
            m_rightButtons->addButton(b);
    m_buttonManager->configure(m_shadowOpacity, m_illumination, m_buttonStyle, 0, Gradient());
    m_buttonManager->setColors(m_bg, m_fg, m_minColor, m_maxColor, m_closeColor);
    m_buttonManager->clearCache();
    updateLayout();
    applyNativeShadow();
}

void
Deco::recalculate()
{
    m_isDark = Color::lum(m_bg) < Color::lum(m_fg);
    if (m_bling)
        delete [] m_bling;
    m_bling = 0;
    reconfigure();
}

const int
Deco::border()
{
    bool max(false);
    if (!!window())
        max = window()->isMaximized();
    const int m = max || m_uno ? 0 : Ops::dpiScaled(nullptr, 6);
    return m;
}

quint32
Deco::winId() const
{
#if defined(ATMO_USE_KDECORATION3)
    return 0;
#else
    return window() ? window()->windowId() : 0;
#endif
}

quint32
Deco::decoKey() const
{
    if (const quint32 id = winId())
        return id;
#if defined(ATMO_USE_KDECORATION3)
    if (window())
        return qHash(window()->windowClass() + window()->caption());
#endif
    return static_cast<quint32>(reinterpret_cast<quintptr>(this) & 0xffffffffu);
}

QString
Deco::windowClassName() const
{
#if defined(ATMO_USE_KDECORATION3)
    return window() ? window()->windowClass() : QString();
#else
    if (!winId())
        return QString();
    KWindowInfo info(winId(), NET::WM2WindowClass);
    return info.windowClassClass();
#endif
}

uint
Deco::nativeShadowThemeHash() const
{
    uint hash = 0;
    hash = ::qHash(dConf.deco.shadowSize, hash);
    hash = ::qHash(dConf.deco.shadowRnd, hash);
    hash = ::qHash(dConf.deco.frameSize, hash);
    hash = ::qHash(dConf.shadows.opacity, hash);
    hash = ::qHash(dConf.shadows.illumination, hash);
    hash = ::qHash(dConf.shadows.darkRaisedEdges, hash);
    hash = ::qHash(m_bg.rgba(), hash);
    hash = ::qHash(m_fg.rgba(), hash);
    hash = ::qHash(m_shadowOpacity, hash);
    hash = ::qHash(m_uno, hash);
    hash = ::qHash(m_titleHeight, hash);
    return hash;
}

void
Deco::applyNativeShadow()
{
#if defined(ATMO_USE_KDECORATION3)
    if (!window())
        return;
    const int opacity = m_shadowOpacity ? m_shadowOpacity : dConf.shadows.opacity;
    if (dConf.deco.shadowSize <= 0 || opacity <= 0 || window()->isMaximized())
    {
        setShadow(std::shared_ptr<KDecoration3::DecorationShadow>());
        return;
    }

    const qreal scale = decoScale(this);
    const qreal dpr = qMax<qreal>(scale, decorationDpr());
    const int radius = dConf.deco.shadowRnd ? dConf.deco.shadowRnd : dConf.frameRnd;
    const QColor tint = window()->palette().color(QPalette::Shadow).isValid()
        ? Color::mid(window()->palette().color(QPalette::Shadow), m_bg, 1, 1)
        : m_bg;
    setShadow(DecorationShadowCache::shadow(scale,
                                            dpr,
                                            window()->isActive(),
                                            dConf.deco.shadowSize,
                                            opacity,
                                            radius,
                                            border(),
                                            tint,
                                            nativeShadowThemeHash()));
#endif
}

void
Deco::maximizedChanged(const bool max)
{
    Q_UNUSED(max)
    const qreal scale = decoScale(this);
    const qreal m = snapDecoMetric(border(), scale);
    setBorders(AtmoDecoMargins(m, snapDecoMetric(titleBar().height(), scale), m, m));
    applyNativeShadow();
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
    {
        const auto buttons = m_leftButtons->buttons();
        for (const auto &button : buttons)
            if (button)
                button->setVisible(visible);
    }
    if (m_rightButtons)
    {
        const auto buttons = m_rightButtons->buttons();
        for (const auto &button : buttons)
            if (button)
                button->setVisible(visible);
    }
}

WindowData
Deco::getShm()
{
    WindowData data = WindowData::memory(decoKey(), this);
    if (data && data.lock())
    {
        if (!data->imageHeight)
        {
            data->separator = true;
            data->uno = true;
        }
        if (!data->titleHeight)
            data->titleHeight = titleHeight();
        data.unlock();
    }
//    activeChanged(true);
    return data;
}

void
Deco::updateData()
{
    WindowData data = getShm();
    bool winDataChanged(false);
    if (data && data.lock())
    {
        m_illumination      = data->illumination;
        m_textBevOpacity    = data->textBevelOpacity;
        m_separator         = data->separator;
        m_shadowOpacity     = data->shadowOpacity;
        m_contAware         = data->contAware;
        m_icon              = data->winIcon;
        m_buttonStyle       = data->buttons;
        m_opacity           = data->opacity;
        m_uno               = data->uno;
        m_hor               = data->hor;
        m_embedButtons      = data->embedButtons;
        m_titleHeight       = data->titleHeight;
        m_bg                = QColor::fromRgba(data->bgColor);
        m_fg                = QColor::fromRgba(data->fgColor);
        m_minColor          = QColor::fromRgba(data->minColor);
        m_maxColor          = QColor::fromRgba(data->maxColor);
        m_closeColor        = QColor::fromRgba(data->closeColor);
        m_buttonGradient    = data.buttonGradient();
        m_windowGradient    = data.windowGradient();
        m_bgPix             = QPixmap::fromImage(data.image());
        m_hasSharedMem      = true;

        data->decoId = winId();
        if (!data->decoId)
            data->decoId = decoKey();
        data.unlock();

        m_tries = 0;
        winDataChanged = true;
    }
    else
    {
        if (++m_tries < 4)
            QTimer::singleShot(250, this, SLOT(updateData()));
    }

    if (m_embedButtons && !m_embedder)
        m_embedder = new EmbedHandler(this);
    else if (!m_embedButtons && m_embedder)
        removeEmbedder();

    setTitleHeight(m_titleHeight);
    setButtonsVisible(!m_embedder);

    if (!m_uno)
    {
        const qreal scale = decoScale(this);
        const qreal side = snapDecoMetric(border(), scale);
        setBorders(AtmoDecoMargins(side, snapDecoMetric(titleHeight(), scale), side, side));
        if (m_grip)
        {
            m_grip->deleteLater();
            m_grip = 0;
        }
        m_winGradient = Settings::gradientStops(m_windowGradient);
        if (m_bevel != 1)
        {
            m_bevel = 1;
            for (int i = 0; i < 3; ++i)
                m_bevelCorner[i] = QPixmap();
        }
    }

    m_buttonManager->setColors(m_bg, m_fg, m_minColor, m_maxColor, m_closeColor);
    m_buttonManager->configure(m_shadowOpacity, m_illumination, m_buttonStyle, m_followDecoShadow, m_buttonGradient);
    for (int i = 0; i < m_buttonManager->buttons().size(); ++i)
    {
        Button *button = static_cast<Button *>(m_buttonManager->buttons().at(i));
        button->updateGeometry();
    }
    m_buttonManager->clearCache();
    if (m_leftButtons)
    {
        if (m_buttonStyle == ButtonBase::Anton)
            m_leftButtons->setSpacing(-Ops::dpiScaled(nullptr, 2));
    }
    if (m_rightButtons)
    {
        if (m_buttonStyle == ButtonBase::Anton)
            m_rightButtons->setSpacing(-Ops::dpiScaled(nullptr, 2));
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
    recalculate();
    updateBgPixmap();
    applyNativeShadow();
    update();
    if (winDataChanged)
        Q_EMIT dataChanged();
}

void
Deco::checkForDataFromWindowClass()
{
    if (m_hasSharedMem)
        return;
    QString winClass = windowClassName();
    if (winClass.isEmpty())
        winClass = QStringLiteral("default");
    ATMO_LOG << "Applying decoration data for window class: " << winClass;
    Data::decoData(winClass, this);
    updateBgPixmap();
    applyNativeShadow();

#if HASDBUSMENU && HASXCB && !defined(ATMO_USE_KDECORATION3)
    if (AtmoX11::isAvailable() && m_showMenuBar && !m_menuBar)
    {
        Xcb::Property objectPath("_KDE_NET_WM_APPMENU_OBJECT_PATH", winId());
        Xcb::Property serviceName("_KDE_NET_WM_APPMENU_SERVICE_NAME", winId());

        if (objectPath && serviceName)
            m_menuBar = new MenuBar(this, serviceName.toString(), objectPath.toString());
    }
#endif
}

const int
Deco::titleHeight() const
{
    return m_titleHeight;
}

void
Deco::setTitleHeight(const int h)
{
    m_titleHeight = h;
    const qreal scale = decoScale(this);
    AtmoDecoRect tb(titleBar());
    tb.setHeight(snapDecoMetric(h, scale));
    setTitleBar(tb);
    AtmoDecoMargins b(borders());
    b.setTop(snapDecoMetric(h, scale));
    setBorders(b);
}

void
Deco::updateLayout()
{
    const qreal scale = decoScale(this);
    const qreal rectWidth = rect().width();
    const qreal borderLeft = borders().left();
    const qreal borderRight = borders().right();
    const qreal width = rectWidth - borderLeft - borderRight;
    const qreal leftBorder = borderLeft > 0.0 ? borderLeft : snapDecoMetric(6, scale);
    const qreal rightBorder = borderRight > 0.0 ? borderRight : snapDecoMetric(6, scale);
    qreal leftWidgetSize = leftBorder, rightWidgetSize = rightBorder;
    if (m_leftButtons)
    {
        const int add = (m_buttonStyle == ButtonBase::Anton) * 2;
        m_leftButtons->setPos(AtmoDecoPoint(leftBorder + add, window()->isModal() ? Ops::dpiScaled(nullptr, 2) : Ops::dpiScaled(nullptr, 4)));
        leftWidgetSize += m_leftButtons->geometry().width();
    }
#if HASDBUSMENU
    if (m_menuBar)
    {
        m_menuBar->setPos(QPointF(leftWidgetSize + Ops::dpiScaled(nullptr, 8), Ops::dpiScaled(nullptr, 2)));
        leftWidgetSize += m_menuBar->geometry().width();
    }
#endif
    if (m_rightButtons)
    {
        m_rightButtons->setPos(AtmoDecoPoint((rectWidth - (m_rightButtons->geometry().width() + rightBorder)), window()->isModal() ? Ops::dpiScaled(nullptr, 2) : Ops::dpiScaled(nullptr, 4)));
        rightWidgetSize += m_rightButtons->geometry().width() + Ops::dpiScaled(nullptr, 8);
    }
    setTitleBar(AtmoDecoRect(borderLeft, 0, width, snapDecoMetric(m_titleHeight, scale)));
    m_textRect = QRect(QPoint(qRound(leftWidgetSize + Ops::dpiScaled(nullptr, 16)), 0), QPoint(qRound(rectWidth - borderRight - rightWidgetSize), titleHeight()));
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

void
Deco::activeChanged(const bool active)
{
//    update();
//    AdaptorManager::instance()->windowChanged(window()->windowId(), active);
    if (m_grip)
        m_grip->setColor(Color::mid(m_fg, m_bg, 1, 2));;
#if defined(ATMO_USE_KDECORATION3)
    Q_UNUSED(active)
    applyNativeShadow();
#else
    if (const quint32 id = winId())
        ShadowHandler::installShadows(id, active);
#endif
    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

void
Deco::paint(QPainter *painter, const AtmoDecoRect &repaintArea)
{
    if (!painter->isActive())
        return;
    painter->save();
    //bg
    painter->setFont(m_font);
    bool needPaintBg(true);
    painter->setBrushOrigin(titleBar().topLeft());
    if (!m_bgPix.isNull())
    {
        QRect r = atmoToRect(rect());
        if (m_uno)
            r = atmoToRect(titleBar());
        painter->drawTiledPixmap(r, m_bgPix);
        needPaintBg = false;
    }
    if (!m_uno && !m_winGradient.isEmpty())
    {
        const bool hor = m_hor;
        QLinearGradient lg(rect().topLeft(), hor ? rect().topRight() : rect().bottomLeft());
        lg.setStops(m_winGradient);
        painter->setCompositionMode(QPainter::CompositionMode_Overlay);
        painter->fillRect(rect(), lg);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        needPaintBg = false;
    }
    if (needPaintBg)
    {
        if (!m_pix.isNull())
            painter->drawTiledPixmap(titleBar(), m_pix);
        else
            painter->fillRect(titleBar(), window()->palette().color(QPalette::Window));
    }

    if (m_opacity != 0xff && !m_uno)
    {
        painter->setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter->fillRect(rect(), QColor(0,0,0,m_opacity));
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    }

    if (m_separator)
        painter->fillRect(0, titleHeight() - 1, qRound(titleBar().width()), 1, QColor(0, 0, 0, m_shadowOpacity));
    if (m_contAware)
    {
        if (!m_mem && winId())
            m_mem = new QSharedMemory(QString::number(winId()), this);
        if (m_mem && (m_mem->isAttached() || m_mem->attach(QSharedMemory::ReadOnly)) && m_mem->lock())
        {
            const uchar *idata(reinterpret_cast<const uchar *>(m_mem->constData()));
            painter->drawImage(QPoint(0, 0),
                               QImage(idata, qRound(titleBar().width()), titleHeight(), QImage::Format_ARGB32_Premultiplied),
                               titleBar());
            m_mem->unlock();
        }
    }

    if ((!dConf.deco.frameSize || window()->isMaximized()) && !window()->isModal() && !window()->isMaximized())
        paintBevel(painter, m_illumination/*bgLum*/);
    if (m_blingEnabled)
        paintBling(painter, titleTextArea());

    //text
    const bool paintText(titleBar().height() > Ops::dpiScaled(nullptr, 19));

    QFont f(painter->font());
    if (paintText)
    {
        if (window()->isActive())
        {
            f.setBold(true);
            painter->setFont(f);
        }
        int flags = Qt::AlignCenter|Qt::TextHideMnemonic;

        QString text(window()->caption());
        QRect textRect(painter->fontMetrics().boundingRect(atmoToRect(titleBar()), flags, text));
        const int iconPad = m_icon * Ops::dpiScaled(nullptr, 20);
        if (m_textRect.x()+iconPad > textRect.x())
        {
            textRect.moveLeft(m_textRect.left()+iconPad);
//            flags &= ~Qt::AlignHCenter;
//            flags |= Qt::AlignLeft;
        }

        if (m_icon)
        {

            QRect ir(QPoint(), QSize(Ops::dpiScaled(nullptr, 16), Ops::dpiScaled(nullptr, 16)));
            ir.moveTop(qRound(titleBar().top() + (titleBar().height() / 2.0 - ir.height() / 2.0)));
            ir.moveLeft(textRect.left() - Ops::dpiScaled(nullptr, 20));
            window()->icon().paint(painter, ir, Qt::AlignCenter, window()->isActive()?QIcon::Active:QIcon::Disabled);
        }
        if (textRect.right() > m_textRect.right())
            textRect.setRight(m_textRect.right());
        text = painter->fontMetrics().elidedText(text, Qt::ElideRight, m_textRect.width()-iconPad, flags);
        if (window()->isActive() && m_textBevOpacity)
        {
            const int rgb(m_isDark?0:255);
            painter->setPen(QColor(rgb, rgb, rgb, m_textBevOpacity));
            painter->drawText(textRect.translated(0, Ops::dpiScaled(nullptr, 1)), flags, text);
        }
        painter->setPen(m_fg);
        painter->drawText(textRect, flags, text);
        //icon


    }
    f.setBold(false);
    painter->setFont(f);
    //buttons
    if (m_leftButtons)
        m_leftButtons->paint(painter, repaintArea);
    if (m_rightButtons)
        m_rightButtons->paint(painter, repaintArea);
#if HASDBUSMENU
    if (m_menuBar)
        m_menuBar->paint(painter, repaintArea);
#endif

    painter->restore();
}

void
Deco::paintBevel(QPainter *painter, const int bgLum)
{
//#if 0
    const int size = Ops::dpiScaled(nullptr, 5);
    if (m_bevelCorner[0].isNull() || m_prevLum != bgLum)
    {
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

//        QImage tmp2 = tmp;
//        FX::expblur(tmp, 1);
//        pt.begin(&tmp);
//        pt.drawImage(0,0,tmp2);
//        pt.end();
        m_bevelCorner[0] = QPixmap::fromImage(tmp.copy(QRect(0, 0, size, size)));
        m_bevelCorner[1] = QPixmap::fromImage(tmp.copy(QRect(size+1, 0, size, size)));
        m_bevelCorner[2] = QPixmap::fromImage(tmp.copy(size, 0, 1, 4));
    }
    const QRect decoRect = atmoToRect(rect());
    painter->drawPixmap(QRect(decoRect.topLeft(), m_bevelCorner[0].size()), m_bevelCorner[0]);
    painter->drawPixmap(QRect(decoRect.topRight() - QPoint(m_bevelCorner[1].width() - 1, 0), m_bevelCorner[1].size()), m_bevelCorner[1]);
    painter->drawTiledPixmap(decoRect.adjusted(m_bevelCorner[0].width(), 0, -m_bevelCorner[1].width(), -(decoRect.height() - 4)), m_bevelCorner[2]);

    static QPixmap cornerErase[2];
    if (cornerErase[0].isNull())
    {
        QImage c(size*2+1, size*2+1, QImage::Format_ARGB32_Premultiplied);
        c.fill(Qt::transparent);
        QPainter p(&c);
        p.setBrush(Qt::black);
        p.setPen(Qt::NoPen);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(c.rect(), Qt::black);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.drawEllipse(c.rect());
        p.end();
        cornerErase[0] = QPixmap::fromImage(c.copy(QRect(0, 0, size, size)));
        cornerErase[1] = QPixmap::fromImage(c.copy(QRect(size+1, 0, size, size)));
    }
    const QPainter::CompositionMode mode = painter->compositionMode();
    painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
    painter->drawPixmap(QRect(decoRect.topLeft(), cornerErase[0].size()), cornerErase[0]);
    painter->drawPixmap(QRect(decoRect.topRight() - QPoint(cornerErase[1].width() - 1, 0), cornerErase[1].size()), cornerErase[1]);
    painter->setCompositionMode(mode);

//#endif
}

void
Deco::paintBling(QPainter *painter, const QRect &r)
{
    const int rad = Ops::dpiScaled(nullptr, 16);
    if (!m_bling)
    {
        m_bling = new QPixmap[3]();

        const int mid = Ops::dpiScaled(nullptr, 16);
        QRect rect(0, 0, rad*2+mid, r.height());
        QImage tmp(rect.size() + QSize(Ops::dpiScaled(nullptr, 8), Ops::dpiScaled(nullptr, 8)), QImage::Format_ARGB32_Premultiplied);
        tmp.fill(Qt::transparent);
        QPainter p(&tmp);
        const int blurRad = Ops::dpiScaled(nullptr, 4);
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

            tmp = tmp.copy(0, 0, tmp.width() - Ops::dpiScaled(nullptr, 8), tmp.height() - Ops::dpiScaled(nullptr, 8));
            p.begin(&tmp);
            p.setRenderHint(QPainter::Antialiasing);
            const QColor c = m_bg;
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

            tmp = tmp.copy(0, 0, tmp.width() - Ops::dpiScaled(nullptr, 8), tmp.height() - Ops::dpiScaled(nullptr, 8));
            p.begin(&tmp);
            p.setRenderHint(QPainter::Antialiasing);
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
//            p.fillPath(bling, Qt::black);

            p.drawImage(0,0,tmp2);

    //        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);

            const QColor c = m_bg;
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
    const QRect tb = atmoToRect(titleBar());
    const int left = m_leftButtons ? qRound(m_leftButtons->geometry().right()) + 1 : tb.left();
    const int right = (m_rightButtons ? qRound(m_rightButtons->geometry().left()) : tb.right()) - left;
    return QRect(left, 0, right, tb.height());
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
        break;
    }
    default: break;
    }
    return path;
}

void
Deco::updateBgPixmap()
{
    QRect r(0, 0, 1, titleHeight());
    QLinearGradient lg(r.topLeft(), r.bottomLeft());
    if (!m_gradient.isEmpty())
        lg.setStops(NSE::Settings::gradientStops(m_gradient, m_bg));
    else
        lg.setStops(QGradientStops() << QGradientStop(0, Color::mid(m_bg, Qt::white, 4, 1)) << QGradientStop(1, Color::mid(m_bg, Qt::black, 4, 1)));

    const QPixmap noisePix = GFX::noisePix(m_noiseStyle);
    QPixmap p(noisePix.size().width(), r.height());
    p.fill(Qt::transparent);
    QPainter pt(&p);
    pt.fillRect(p.rect(), lg);
    if (m_noise)
    {
        QPixmap noise(noisePix.size());
        noise.fill(Qt::transparent);
        QPainter ptt(&noise);
        ptt.drawTiledPixmap(noise.rect(), noisePix);
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const AtmoDecoPoint p(static_cast<QHoverEvent *>(event)->position());
#else
        const AtmoDecoPoint p(static_cast<QHoverEvent *>(event)->pos());
#endif
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
    return KDecoration3::Decoration::event(event);
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
            QPointer<KDecoration3::DecorationButton> button = m_leftButtons->buttons().at(i);
            if (button)
                if (Button *b = qobject_cast<Button *>(button.data()))
                    b->hover();
        }
    if (m_rightButtons)
        for (int i = 0; i < m_rightButtons->buttons().count(); ++i)
        {
            QPointer<KDecoration3::DecorationButton> button = m_rightButtons->buttons().at(i);
            if (button)
                if (Button *b = qobject_cast<Button *>(button.data()))
                    b->hover();
        }
}

void
Deco::hoverLeave()
{
    m_isHovered = false;
//#if HASDBUSMENU
//    if (m_menuBar && !m_menuBar->hasShownMenues())
//    {
//        m_menuBar->stopMousePolling();
//        QVector<QPointer<KDecoration3::DecorationButton> > kids = m_menuBar->buttons();
//        for (int i = 0; i < kids.count(); ++i)
//            static_cast<MenuBarItem *>(kids.at(i).data())->hoverLeave();
//    }
//#endif
    if (m_leftButtons)
        for (int i = 0; i < m_leftButtons->buttons().count(); ++i)
        {
            QPointer<KDecoration3::DecorationButton> button = m_leftButtons->buttons().at(i);
            if (button)
                if (Button *b = qobject_cast<Button *>(button.data()))
                    b->unhover();
        }
    if (m_rightButtons)
        for (int i = 0; i < m_rightButtons->buttons().count(); ++i)
        {
            QPointer<KDecoration3::DecorationButton> button = m_rightButtons->buttons().at(i);
            if (button)
                if (Button *b = qobject_cast<Button *>(button.data()))
                    b->unhover();
        }
}

//----------------------------------------------------------------------------------

QPolygon
Grip::shape()
{
    const int gripSize = Ops::dpiScaled(nullptr, Size);
    return QPolygon() << QPoint(gripSize, 0) << QPoint(gripSize, gripSize) << QPoint(0, gripSize); //topright, bottomright, bottomleft
}

Grip::Grip(Deco *d)
    : QWidget(0)
    , m_deco(d)
{
#if HASXCB || HASX11
    if (!AtmoX11::isAvailable() || !m_deco || !m_deco->winId())
    {
        hide();
        return;
    }
#else
    hide();
    return;
#endif
    setFocusPolicy(Qt::NoFocus);
    const int gripSize = Ops::dpiScaled(nullptr, Size);
    setFixedSize(gripSize, gripSize);
    setCursor(Qt::SizeFDiagCursor);
    setMask(shape());
    AtmoDecoratedWindow *c = m_deco->window();
    connect(c, &AtmoDecoratedWindow::heightChanged, this, &Grip::updatePosition);
    connect(c, &AtmoDecoratedWindow::widthChanged, this, &Grip::updatePosition);
    restack();
    updatePosition();
    show();
}

void
Grip::setColor(const QColor &c)
{
    QPalette p = m_deco->window()->palette();
    p.setColor(backgroundRole(), c);
    setPalette(p);
    regenPix();
}

void
Grip::updatePosition()
{
    AtmoDecoratedWindow *c = m_deco->window();
    if (!c || !m_deco->winId())
        return;
    const int gripSize = Ops::dpiScaled(nullptr, Size);
    const int gripMargin = Ops::dpiScaled(nullptr, Margin);
    XHandler::move(winId(), QPoint(c->width() - (gripSize + gripMargin), c->height() - (gripSize + gripMargin)));
}

void
Grip::restack()
{
    if (XHandler::XWindow windowId = m_deco->winId())
        XHandler::restack(winId(), windowId);
    else
        hide();
}

void
Grip::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);
    if (m_deco->winId())
    {
        XHandler::mwRes(e->position().toPoint(), e->globalPosition().toPoint(), winId(), true, m_deco->winId());
    }
}

void
Grip::regenPix()
{
    QImage img(size(), QImage::Format_ARGB32);
    QPainter p(&img);
    p.setPen(QPen(QColor(0, 0, 0, 255), qMax<qreal>(1.0, Ops::scaleForWidget(this) * 2.0)));
    p.setBrush(palette().color(backgroundRole()));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPolygon(shape());
    p.end();
    FX::expblur(img, qMax(1, Ops::dpiScaled(this, 2)));
    m_pix = QPixmap::fromImage(img);
}

void
Grip::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(rect(), m_pix);
}

} //NSE
