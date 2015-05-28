#include <kwindowinfo.h>
#include <QPainter>
#include <QHBoxLayout>
#include <QTimer>
#include <QPixmap>
#include <QCoreApplication>
#include <QBuffer>
#include <KWindowSystem>
#include <QSettings>
#include <QDir>

#include "kwinclient.h"
#include "../stylelib/ops.h"
#include "../stylelib/shadowhandler.h"
#include "../stylelib/render.h"
#include "../stylelib/color.h"
#include "../config/settings.h"
#include "../stylelib/xhandler.h"

#if !defined(QT_NO_DBUS)
#include "decoadaptor.h"
#include <QDBusConnection>
#include <QDBusMessage>
#endif

#define MARGIN (isModal()?3:6)
#define SPACING 4

///-------------------------------------------------------------------

DButton::DButton(const Type &t, KwinClient *client)
    : ButtonBase(t)
    , QSpacerItem(16, 16)
    , m_client(client)
{
}

void
DButton::hoverChanged()
{
    m_client->widget()->repaint(buttonRect());
}

const bool
DButton::isDark() const
{
    QColor fgc(m_client->options()->color(KDecoration::ColorFont, m_client->isActive()));
    if (m_client->m_custcol[Fg].isValid())
        fgc = m_client->m_custcol[Fg];
    QColor bgc(m_client->options()->color(KDecoration::ColorTitleBar, m_client->isActive()));
    if (m_client->m_custcol[Bg].isValid())
        bgc = m_client->m_custcol[Bg];
    return Color::luminosity(fgc) > Color::luminosity(bgc);
}

const QColor
DButton::color(const ColorRole &c) const
{
    if (c == Highlight)
        return m_client->widget()->palette().color(QPalette::Highlight);
    QColor fgc(m_client->options()->color(KDecoration::ColorFont, m_client->isActive()));
    if (m_client->m_custcol[Fg].isValid())
        fgc = m_client->m_custcol[Fg];
    if (c == Fg)
        return fgc;
    QColor bgc(m_client->options()->color(KDecoration::ColorTitleBar, m_client->isActive()));
    if (m_client->m_custcol[Bg].isValid())
        bgc = m_client->m_custcol[Bg];
    if (c == Bg)
        return bgc;
    const bool isd(isDark());
    return Color::mid(fgc, bgc, 1, (!isd*4)+(!isActive()*(isd?2:8)));
}

void
DButton::onClick(const Qt::MouseButton &button)
{
    switch (type())
    {
    case Close: m_client->closeWindow(); break;
    case Minimize: m_client->minimize(); break;
    case Maximize: m_client->maximize(button); break;
    case OnAllDesktops: m_client->toggleOnAllDesktops(); break;
    case Menu: m_client->showWindowMenu(m_client->widget()->mapToGlobal(buttonRect().bottomLeft())); break;
    case KeepAbove: m_client->setKeepAbove(!m_client->keepAbove()); break;
    case KeepBelow: m_client->setKeepBelow(!m_client->keepBelow()); break;
    case ApplicationMenu: m_client->showApplicationMenu(m_client->widget()->mapToGlobal(buttonRect().bottomLeft())); break;
    case Shade: m_client->setShade(!m_client->isShade()); break;
    case ContextHelp: m_client->showContextHelp(); break;
    default: break;
    }
    m_client->widget()->repaint();
}

const bool
DButton::isMaximized() const
{
    return m_client->maximizeMode()==KDecoration::MaximizeFull;
}

const bool
DButton::onAllDesktops() const
{
    return m_client->isOnAllDesktops();
}

const bool
DButton::keepAbove() const
{
    return m_client->keepAbove();
}

const bool
DButton::isActive() const
{
    return m_client->isActive();
}

const bool
DButton::keepBelow() const
{
    return m_client->keepBelow();
}

const bool
DButton::shade() const
{
    return m_client->isShade();
}

///-------------------------------------------------------------------------------------------------

QMap<QString, KwinClient::Data> KwinClient::Data::s_data;

static void addDataForWinClass(const QString &winClass, QSettings &s)
{
    KwinClient::Data d;
    d.fg = QColor::fromRgba(s.value("fgcolor", "0x00000000").toString().toUInt(0, 16));
    d.bg = QColor::fromRgba(s.value("bgcolor", "0x00000000").toString().toUInt(0, 16));
    d.grad = Settings::stringToGrad(s.value("gradient", "0:10, 1:-10").toString());
    d.noise = s.value("noise", 20).toUInt();
    d.separator = s.value("separator", true).toBool();
    KwinClient::Data::s_data.insert(winClass, d);
}

static void readWindowData()
{
    KwinClient::Data::s_data.clear();
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
KwinClient::Data::decoData(const QString &winClass, KwinClient *d)
{
    KwinClient::Data data;
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

DSP::AdaptorManager *DSP::AdaptorManager::s_instance = 0;

DSP::AdaptorManager
*DSP::AdaptorManager::instance()
{
    if (!s_instance)
        s_instance = new DSP::AdaptorManager();
    return s_instance;
}

DSP::AdaptorManager::AdaptorManager()
{
    Settings::read();
    Render::makeNoise();
    readWindowData();
    new DecoAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.kde.dsp.kdecoration2");
    QDBusConnection::sessionBus().registerObject("/DSPDecoAdaptor", this);
}

DSP::AdaptorManager::~AdaptorManager()
{
    QDBusConnection::sessionBus().unregisterService("org.kde.dsp.kdecoration2");
    QDBusConnection::sessionBus().unregisterObject("/DSPDecoAdaptor");
    s_instance = 0;
}

///-------------------------------------------------------------------------------------------------

KwinClient::KwinClient(KDecorationBridge *bridge, Factory *factory)
    : KDecoration(bridge, factory)
    , m_titleLayout(0)
    , m_contAware(false)
    , m_separator(true)
    , m_factory(factory)
    , m_opacity(0xff)
    , m_sizeGrip(0)
    , m_mem(0)
    , m_uno(true)
    , m_frameSize(0)
    , m_compositingActive(true)
    , m_hor(false)
    , m_noise(0)
{
    setParent(factory);
    Settings::read();
}

KwinClient::~KwinClient()
{
    if (m_mem && m_mem->isAttached())
        m_mem->detach();
    if (m_sizeGrip)
    {
        m_sizeGrip->deleteLater();
        m_sizeGrip = 0;
    }
//    XHandler::deleteXProperty(windowId(), XHandler::DecoData);
}

void
KwinClient::init()
{
    createMainWidget();
    widget()->installEventFilter(this);
    widget()->setAttribute(Qt::WA_NoSystemBackground);
    widget()->setMouseTracking(true);

    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setSpacing(SPACING);
    int tb(!isModal()*3);
    m_titleLayout->setContentsMargins(MARGIN, tb, MARGIN, tb);
    QVBoxLayout *l = new QVBoxLayout(widget());
    l->setSpacing(0);
    l->setContentsMargins(dConf.deco.frameSize, dConf.deco.frameSize, dConf.deco.frameSize, dConf.deco.frameSize);
    l->addLayout(m_titleLayout);
    l->addStretch();
    widget()->setLayout(l);
    m_separator = true;
    if (!isPreview() && windowId())
    {
        DSP::AdaptorManager::instance()->addDeco(this);
        if (m_wd = WindowData::memory(id, this))
        {
            if (!m_wd->value<bool>(WindowData::IsInited, true))
            {
                m_wd->setValue<bool>(WindowData::IsInited, true);
                m_wd->setValue<bool>(WindowData::Separator, true);
                m_wd->setValue<bool>(WindowData::Uno, true);
                m_wd->setValue<int>(WindowData::TitleHeight, titleHeight());
            }
        }
        else
            checkForDataFromWindowClass();
        ShadowHandler::installShadows(windowId());
        if (isResizable() && !m_sizeGrip && !dConf.deco.frameSize)
            m_sizeGrip = new SizeGrip(this);
    }
    else
        updateBgPixmap();

    reset(127);
}

void
KwinClient::populate(const QString &buttons, int &sz)
{
    int size(0);
    for (int i = 0; i < buttons.size(); ++i)
    {
        Button::Type t;
        bool supported(true);
        switch (buttons.at(i).toAscii())
        {
        /*
        * @li 'R' resize button
        */
        case 'H': t = Button::ContextHelp; break;
        case 'L': t = Button::Shade; break;
        case 'N': t = Button::ApplicationMenu; break;
        case 'B': t = Button::KeepBelow; break;
        case 'F': t = Button::KeepAbove; break;
        case 'M': t = Button::Menu; break;
        case 'S': t = Button::OnAllDesktops; break;
        case 'X': t = Button::Close; break;
        case 'I': t = Button::Minimize; break;
        case 'A': t = Button::Maximize; break;
        case '_': m_titleLayout->addSpacing(SPACING); supported = false; size += SPACING; break;
        default: supported = false; break;
        }

        if (t == Button::ContextHelp && !providesContextHelp())
            supported = false;

        if (supported)
        {
            DButton *b = new DButton(t, this);
            b->setButtonStyle(dConf.deco.buttons);
            m_buttons << b;
            size += 16+SPACING;
            m_titleLayout->addItem(b);
            if (i < buttons.size()-1)
                m_titleLayout->addSpacing(SPACING);
        }
    }
    sz = size+MARGIN;
}

void
KwinClient::updateMask()
{
    const int w(width()), h(height());
    if (isModal())
    {
        widget()->update();
        return;
    }

    if (m_compositingActive)
    {
        if (m_opacity != 0xff || dConf.deco.frameSize > 3)
            clearMask();
        else
        {
            QRegion r(2, 0, w-4, h);
            r += QRegion(1, 0, w-2, h-1);
            r += QRegion(0, 0, w, h-2);
            setMask(r);
        }
    }
    else
    {
        QRegion r(0, 2, w, h-4);
        r += QRegion(1, 1, w-2, h-2);
        r += QRegion(2, 0, w-4, h);
        setMask(r);
    }
    widget()->update();
}

void
KwinClient::update()
{
    widget()->update();
}

void
KwinClient::resize(const QSize &s)
{
    widget()->resize(s);
    updateMask();
}

void
KwinClient::borders(int &left, int &right, int &top, int &bottom) const
{
    const int f((maximizeMode()!=MaximizeFull)*dConf.deco.frameSize);
    left = right = bottom = f;
    top = titleHeight()+f;
}

void
KwinClient::captionChange()
{
    widget()->update();
}

QColor
KwinClient::fgColor(const bool *active) const
{
    if (m_wd)
    {
        const QColor &c = m_wd->bg();
        if (c.alpha() == 0xff)
            return c;
    }
    if (m_fg.isValid() && m_fg.alpha() == 0xff)
        return m_fg;
    return widget()->palette().color(QPalette::WindowText);
}

QColor
KwinClient::bgColor(const bool *active) const
{
    if (m_wd)
    {
        const QColor &c = m_wd->bg();
        if (c.alpha() == 0xff)
            return c;
    }
    if (m_bg.isValid() && m_bg.alpha() == 0xff)
        return m_bg;
    return widget()->palette().color(QPalette::Window);
}

bool
KwinClient::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type())
    {
    case QEvent::Paint:
    {
        if (m_compositingActive)
        {
            QPainter p(widget());
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(widget()->rect(), Qt::transparent);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
            paint(p);
            p.end();
        }
        else
        {
            QPixmap pix(widget()->size());
            pix.fill(bgColor());
            QPainter p(&pix);
            paint(p);
            p.end();
            p.begin(widget());
            p.drawPixmap(widget()->rect(), pix);
            p.end();
        }
        return true;
    }
    case QEvent::MouseButtonDblClick:
    {
        titlebarDblClickOperation();
        return true;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    {
        QMouseEvent *me = static_cast<QMouseEvent *>(e);
        for (int i = 0; i < m_buttons.size(); ++i)
        {
            DButton *button(m_buttons.at(i));
            if (button->buttonRect().contains(me->pos()) || me->type() == QEvent::MouseMove)
            {
                button->processMouseEvent(me);
                if (me->type() == QEvent::MouseMove)
                    continue;
                return true;
            }
        }
        if (me->type() == QEvent::MouseButtonPress)
            processMousePressEvent(me);
        return true;
    }
    case QEvent::Enter:
    case QEvent::HoverEnter:
    {
        for (int i = 0; i < m_buttons.size(); ++i)
        {
            DButton *button(m_buttons.at(i));
            if (button->type() == Button::Minimize || button->type() == Button::Maximize || button->type() == Button::Close)
                button->hover();
        }
        return true;
    }
    case QEvent::Leave:
    case QEvent::HoverLeave:
    {
        for (int i = 0; i < m_buttons.size(); ++i)
        {
            DButton *button(m_buttons.at(i));
            if (button->type() == Button::Minimize || button->type() == Button::Maximize || button->type() == Button::Close)
                button->unhover();
        }
        return true;
    }
    case QEvent::Wheel:
    {
        titlebarMouseWheelOperation(static_cast<QWheelEvent *>(e)->delta());
        return true;
    }
    case QEvent::Show:
    {
        for (int i = 0; i < m_buttons.size(); ++i)
            m_buttons.at(i)->unhover();
        widget()->repaint();
        return true;
    }
    default: break;
    }
    return KDecoration::eventFilter(o, e);
}

void
KwinClient::paint(QPainter &p)
{
    p.setBrushOrigin(widget()->rect().topLeft());
    if (dConf.deco.frameSize && maximizeMode() != MaximizeFull)
    {
        p.fillRect(widget()->rect(), Color::mid(widget()->palette().color(widget()->foregroundRole()), widget()->palette().color(widget()->backgroundRole()), 1, 8));
        QRectF r(widget()->layout()->geometry().adjusted(dConf.deco.frameSize, dConf.deco.frameSize, -dConf.deco.frameSize, -dConf.deco.frameSize));
        r.adjust(-0.5f, -0.5f, 0.5f, 0.5f);
        p.setPen(QColor(0, 0, 0, 127));
        p.setBrush(Qt::NoBrush);
        p.drawRect(r);
        p.setPen(QColor(0, 0, 0, 127));
        p.setBrush(QColor(0, 0, 0, 63));
        p.setRenderHint(QPainter::Antialiasing, false);
        p.drawRect(positionRect(PositionTopLeft));
        p.drawRect(positionRect(PositionTopRight));
        p.drawRect(positionRect(PositionBottomLeft));
        p.drawRect(positionRect(PositionBottomRight));

        p.setBrush(Qt::NoBrush);
        p.setPen(QColor(255, 255, 255, 63));
        int rnd(!isModal()*4);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawRoundedRect(QRectF(widget()->rect()).adjusted(0.5f, 0.5f, -0.5f, -0.5f), rnd, rnd);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        r.adjust(0.5f, 0.5f, -0.5f, -0.5f);
        p.fillRect(r, Qt::black);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

    }
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::NoBrush);
    QRect tr(m_titleLayout->geometry());
    if (tr.height() < titleHeight())
        tr.setHeight(titleHeight());

    if (!m_pix.isNull())
        p.drawTiledPixmap(tr, m_pix);
    else
        p.fillRect(tr, bgColor());

    const int bgLum(Color::luminosity(bgColor()));
    const bool isDark(Color::luminosity(fgColor()) > bgLum);

    //bevel
    if ((!dConf.deco.frameSize || maximizeMode() == MaximizeFull) && !isModal())
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
        p.drawPixmap(QRect(tr.topLeft(), m_bevelCorner[0].size()), m_bevelCorner[0]);
        p.drawPixmap(QRect(tr.topRight()-QPoint(m_bevelCorner[1].width()-1, 0), m_bevelCorner[1].size()), m_bevelCorner[1]);
        p.drawTiledPixmap(tr.adjusted(m_bevelCorner[0].width(), 0, -m_bevelCorner[1].width(), -(tr.height()-1)), m_bevelCorner[2]);
    }

    if (m_wd && m_wd->value<bool>(WindowData::ContAware))
    {
        if (!m_mem)
            m_mem = new QSharedMemory(QString::number(windowId()), this);
        if ((m_mem->isAttached() || m_mem->attach(QSharedMemory::ReadOnly)) && m_mem->lock())
        {
            const uchar *data(reinterpret_cast<const uchar *>(m_mem->constData()));
            p.drawImage(QPoint(0, 0),
                               QImage(data, width(), m_wd->value<int>(WindowData::UnoHeight), QImage::Format_ARGB32_Premultiplied),
                               m_titleLayout->geometry());
            m_mem->unlock();
        }
    }

    QFont f(p.font());
    f.setBold(isActive());
    p.setFont(f);

    QString text(caption());
    QRect textRect(p.fontMetrics().boundingRect(tr, Qt::AlignCenter, text));
    const int maxW(tr.width()-(qMax(m_leftButtons, m_rightButtons)*2));
    if (p.fontMetrics().width(text) > maxW)
    {
        text = p.fontMetrics().elidedText(text, Qt::ElideRight, maxW);
        textRect = p.fontMetrics().boundingRect(tr, Qt::AlignCenter, text);
    }
    if (isActive())
    {
        const int rgb(isDark?0:255);
        p.setPen(QColor(rgb, rgb, rgb, 127));
        p.drawText(textRect.translated(0, 1), Qt::AlignCenter, text);
    }

    p.setPen(fgColor());
    static const QString character[] = { " :: ", QString(" %1 ").arg(QChar(0x2013)), " - " };
    bool needPaint(true);
    for (int i = 0; i < 3; ++i)
    if (text.contains(character[i]))
    {
        needPaint = false;
        QString leftText(text.mid(0, text.lastIndexOf(character[i])));
        QString rightText(text.mid(text.lastIndexOf(character[i])));

        QRect leftTextRect(textRect.adjusted(0, 0, -p.fontMetrics().width(rightText), 0));
        QRect rightTextRect(textRect.adjusted(p.fontMetrics().width(leftText), 0, 0, 0));

        p.drawText(leftTextRect, Qt::AlignCenter, leftText);
        p.setPen(Color::mid(fgColor(), bgColor(), 2, 1));
        p.drawText(rightTextRect, Qt::AlignCenter, rightText);
        break;
    }

    if (needPaint)
        p.drawText(textRect, Qt::AlignCenter, text);

    //icon
    if ((!m_wd && dConf.deco.icon) || m_wd->value<bool>(WindowData::WindowIcon))
    {
        QRect ir(QPoint(), QSize(16, 16));
        ir.moveTop(tr.top()+(tr.height()/2-ir.height()/2));
        ir.moveRight(textRect.left()-4);
        if (ir.left() > m_leftButtons)
            icon().paint(&p, ir, Qt::AlignCenter, isActive()?QIcon::Active:QIcon::Disabled);
    }
    if ((!m_wd && m_separator) || m_wd->value<bool>(WindowData::Separator))
    {
        p.setPen(QColor(0, 0, 0, 32));
        p.drawLine(tr.bottomLeft(), tr.bottomRight());
    }
    if (isPreview())
    {
        p.setClipRegion(QRegion(widget()->rect())-QRegion(tr));
        p.fillRect(widget()->rect(), widget()->palette().color(QPalette::Window));
        p.setPen(widget()->palette().color(QPalette::WindowText));
        p.drawText(p.clipBoundingRect(), Qt::AlignCenter, "DSP");
        p.setClipping(false);
    }
    else if (!isModal() && m_compositingActive)
        Render::shapeCorners(&p, Render::All);

    for (int i = 0; i < m_buttons.count(); ++i)
        m_buttons.at(i)->paint(p);
}

QSize
KwinClient::minimumSize() const
{
    return QSize(256, 128);
}

void
KwinClient::activeChange()
{
    if (!isPreview() && maximizeMode() != MaximizeFull)
        ShadowHandler::installShadows(windowId(), isActive());
    widget()->update();
}

static const int corner = 24;
//below stolen from http://www.usermode.org/docs/kwintheme.html, unused atm
KDecorationDefines::Position
KwinClient::mousePosition(const QPoint &point) const
{

    Position pos;

    if (point.y() <= dConf.deco.frameSize) { // inside top frame
        if (point.x() <= corner)                 pos = PositionTopLeft;
        else if (point.x() >= (width()-corner))  pos = PositionTopRight;
        else                                     pos = PositionTop;
    } else if (point.y() >= (height()-dConf.deco.frameSize)) { // inside handle
        if (point.x() <= corner)                 pos = PositionBottomLeft;
        else if (point.x() >= (width()-corner))  pos = PositionBottomRight;
        else                                     pos = PositionBottom;
    } else if (point.x() <= dConf.deco.frameSize) { // on left frame
        if (point.y() <= corner)                 pos = PositionTopLeft;
        else if (point.y() >= (height()-corner)) pos = PositionBottomLeft;
        else                                     pos = PositionLeft;
    } else if (point.x() >= width()-dConf.deco.frameSize) { // on right frame
        if (point.y() <= corner)                 pos = PositionTopRight;
        else if (point.y() >= (height()-corner)) pos = PositionBottomRight;
        else                                     pos = PositionRight;
    } else { // inside the frame
        pos = PositionCenter;
    }
    return pos;
}

const QRect
KwinClient::positionRect(const KDecorationDefines::Position pos) const
{
    switch (pos)
    {
    case KDecorationDefines::PositionTopLeft: return QRect(0, 0, corner, corner);
    case KDecorationDefines::PositionTop: return QRect(corner, 0, width()-(corner*2), corner);
    case KDecorationDefines::PositionTopRight: return QRect(width()-corner, 0, corner, corner);
    case KDecorationDefines::PositionRight: return QRect(width()-corner, corner, corner, height()-(corner*2));
    case KDecorationDefines::PositionBottomRight: return QRect(width()-corner, height()-corner, corner, corner);
    case KDecorationDefines::PositionBottom: return QRect(corner, height()-corner, width()-(corner*2), corner);
    case KDecorationDefines::PositionBottomLeft: return QRect(0, height()-corner, corner, corner);
    case KDecorationDefines::PositionLeft: return QRect(0, corner, corner, height()-(corner*2));
    default: return QRect();
    }
}

const int
KwinClient::titleHeight() const
{
    return isModal()?16:22;
}

void
KwinClient::maximizeChange()
{
    const int fs((maximizeMode()!=MaximizeFull)*dConf.deco.frameSize);
    widget()->layout()->setContentsMargins(fs, fs, fs, fs);
    if (maximizeMode() != MaximizeFull)
        ShadowHandler::installShadows(windowId(), isActive());
    else
        ShadowHandler::removeShadows(windowId());
}

void
KwinClient::readCompositing()
{
    m_compositingActive = XHandler::compositingActive();
    updateMask();
}

void
KwinClient::updateButtons()
{
    m_buttons.clear();
    while (QLayoutItem *item = m_titleLayout->takeAt(0))
    {
        if (item->widget())
            delete item->widget();
        else if (item->spacerItem())
            delete item->spacerItem();
        else
            delete item;
    }
    populate(options()->titleButtonsLeft(), m_leftButtons);
    m_titleLayout->addStretch();
    populate(options()->titleButtonsRight(), m_rightButtons);
}

void
KwinClient::updateBgPixmap()
{
    QRect r(0, 0, 1, titleHeight());
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

void
KwinClient::updateData()
{
    if (isPreview())
        return;
    if (m_wd)
    {
        const int buttonStyle = m_wd->value<int>(WindowData::Buttons);
        const QList<Button *> buttons = findChildren<Button *>();
        for (int i = 0; i < buttons.count(); ++i)
            buttons.at(i)->setButtonStyle(buttonStyle);
    }
    QTimer::singleShot(2000, this, SLOT(readCompositing()));
}

void
KwinClient::checkForDataFromWindowClass()
{
    KWindowInfo info(windowId(), NET::WMWindowType|NET::WMVisibleName|NET::WMName, NET::WM2WindowClass);
    Data::decoData(info.windowClassClass(), this);
    updateBgPixmap();
}

/**
 * These flags specify which settings changed when rereading dConf.
 * Each setting in class KDecorationOptions specifies its matching flag.
 */
/**
    SettingDecoration  = 1 << 0, ///< The decoration was changed
    SettingColors      = 1 << 1, ///< The color palette was changed
    SettingFont        = 1 << 2, ///< The titlebar font was changed
    SettingButtons     = 1 << 3, ///< The button layout was changed
    SettingTooltips    = 1 << 4, ///< The tooltip setting was changed
    SettingBorder      = 1 << 5, ///< The border size setting was changed
    SettingCompositing = 1 << 6  ///< Compositing settings was changed
};*/

void
KwinClient::reset(unsigned long changed)
{
    if (changed & SettingButtons)
        updateButtons();
    if (changed & (SettingDecoration|SettingColors))
        checkForDataFromWindowClass();
    if (changed & SettingCompositing)
        updateData();
    updateMask();
}
