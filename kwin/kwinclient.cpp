
#include <X11/Xatom.h>
#include "../stylelib/xhandler.h"

#include <kwindowinfo.h>
#include <QPainter>
#include <QHBoxLayout>
#include <QTimer>
#include <QPixmap>
#include <QCoreApplication>
#include <QBuffer>

#include "kwinclient.h"
#include "../stylelib/ops.h"
#include "../stylelib/shadowhandler.h"
#include "../stylelib/render.h"
#include "../stylelib/color.h"
#include "../config/settings.h"

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
    return Color::mid(fgc, bgc);
}

void
DButton::onClick(const Qt::MouseButton &button)
{
    switch (type())
    {
    case Close: m_client->closeWindow(); break;
    case Min: m_client->minimize(); break;
    case Max: m_client->maximize(button); break;
    case OnAllDesktops: m_client->toggleOnAllDesktops(); break;
    case WindowMenu: m_client->showWindowMenu(m_client->widget()->mapToGlobal(buttonRect().bottomLeft())); break;
    case KeepAbove: m_client->setKeepAbove(!m_client->keepAbove()); break;
    case KeepBelow: m_client->setKeepBelow(!m_client->keepBelow()); break;
    case AppMenu: m_client->showApplicationMenu(m_client->widget()->mapToGlobal(buttonRect().bottomLeft())); break;
    case Shade: m_client->setShade(!m_client->isShade()); break;
    case QuickHelp: m_client->showContextHelp(); break;
    default: break;
    }
    m_client->widget()->repaint();
}

const DButton::ButtonStyle
DButton::buttonStyle() const
{
    return m_client->m_buttonStyle;
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

///-------------------------------------------------------------------

KwinClient::KwinClient(KDecorationBridge *bridge, Factory *factory)
    : KDecoration(bridge, factory)
    , m_titleLayout(0)
    , m_headHeight(0)
    , m_needSeparator(true)
    , m_factory(factory)
    , m_opacity(1.0f)
    , m_sizeGrip(0)
    , m_mem(0)
    , m_contAware(false)
    , m_uno(true)
    , m_frameSize(0)
    , m_compositingActive(true)
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
    m_headHeight = titleHeight();
    m_needSeparator = true;
    if (!isPreview() && windowId())
    {
        unsigned char height(titleHeight());
        XHandler::setXProperty<unsigned char>(windowId(), XHandler::DecoData, XHandler::Byte, &height); //never higher then 255...
        ShadowHandler::installShadows(windowId());
        if (isResizable() && !m_sizeGrip && !dConf.deco.frameSize)
            m_sizeGrip = new SizeGrip(this);
    }
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
        case 'H': t = Button::QuickHelp; break;
        case 'L': t = Button::Shade; break;
        case 'N': t = Button::AppMenu; break;
        case 'B': t = Button::KeepBelow; break;
        case 'F': t = Button::KeepAbove; break;
        case 'M': t = Button::WindowMenu; break;
        case 'S': t = Button::OnAllDesktops; break;
        case 'X': t = Button::Close; break;
        case 'I': t = Button::Min; break;
        case 'A': t = Button::Max; break;
        case '_': m_titleLayout->addSpacing(SPACING); supported = false; size += SPACING; break;
        default: supported = false; break;
        }

        if (t == Button::QuickHelp && !providesContextHelp())
            supported = false;

        if (supported)
        {
            DButton *b = new DButton(t, this);
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
        return;

    if (m_compositingActive)
    {
        if (m_opacity < 1.0f || dConf.deco.frameSize > 3)
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
    widget()->repaint();
}

QColor
KwinClient::fgColor() const
{
    if (m_custcol[Text].isValid())
        return m_custcol[Text];
    return options()->color(ColorFont, isActive());
}

QColor
KwinClient::bgColor() const
{
    if (m_custcol[Bg].isValid())
        return m_custcol[Bg];
    return options()->color(ColorTitleBar, isActive());
}

bool
KwinClient::eventFilter(QObject *o, QEvent *e)
{
    if (o != widget())
        return false;
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
            p.setRenderHint(QPainter::Antialiasing);
            paint(p);
            p.end();
        }
        else
        {
            QPixmap pix(widget()->size());
            pix.fill(bgColor());
            QPainter p(&pix);
            p.setRenderHint(QPainter::Antialiasing);
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
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(Qt::NoBrush);
        p.setPen(QColor(255, 255, 255, 63));
        int rnd(!isModal()*4);
        p.drawRoundedRect(QRectF(widget()->rect()).adjusted(0.5f, 0.5f, -0.5f, -0.5f), rnd, rnd);
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
    if (m_compositingActive)
        p.setOpacity(m_opacity);

    if (unsigned long *bg = XHandler::getXProperty<unsigned long>(windowId(), XHandler::DecoBgPix))
        p.drawTiledPixmap(tr, QPixmap::fromX11Pixmap(*bg), tr.topLeft());
    else if (!m_bgPix[isActive()].isNull())
        p.drawTiledPixmap(tr, m_bgPix[isActive()]);
    else
        p.fillRect(tr, bgColor());
    p.setOpacity(1.0f);

    const int bgLum(Color::luminosity(bgColor()));
    const bool isDark(Color::luminosity(fgColor()) > bgLum);

    //bevel
    if ((!dConf.deco.frameSize || maximizeMode() == MaximizeFull) && !isModal())
    {
        static QPixmap bevelCorner[3];
        static int prevLum(0);
        if (bevelCorner[0].isNull() || prevLum != bgLum)
        {
            prevLum = bgLum;
            if (!bevelCorner[0])
            {
                for (int i = 0; i < 2; ++i)
                {
                    bevelCorner[i] = QPixmap(5, 5);
                    bevelCorner[i].fill(Qt::transparent);
                }
                bevelCorner[2] = QPixmap(1, 1);
                bevelCorner[2].fill(Qt::transparent);
            }

            QPixmap tmp(11, 10);
            tmp.fill(Qt::transparent);
            QPainter pt(&tmp);
            pt.setRenderHint(QPainter::Antialiasing);
            const QRectF bevel(0.5f, 0.5f, tmp.width()-0.5f, tmp.height());
            QLinearGradient lg(bevel.topLeft(), bevel.bottomLeft());
            lg.setColorAt(0.0f, QColor(255, 255, 255, qMin(255.0f, bgLum*1.1f)));
            lg.setColorAt(0.5f, Qt::transparent);
            pt.setBrush(Qt::NoBrush);
            pt.setPen(QPen(lg, 1.0f));
            pt.drawRoundedRect(bevel, 5, 5);
            pt.end();
            bevelCorner[0] = tmp.copy(QRect(0, 0, 5, 5));
            bevelCorner[1] = tmp.copy(QRect(6, 0, 5, 5));
            bevelCorner[2] = tmp.copy(5, 0, 1, 1);
        }
        p.drawPixmap(QRect(tr.topLeft(), bevelCorner[0].size()), bevelCorner[0]);
        p.drawPixmap(QRect(tr.topRight()-QPoint(bevelCorner[1].width(), 0), bevelCorner[1].size()), bevelCorner[1]);
        p.drawTiledPixmap(tr.adjusted(bevelCorner[0].width(), 0, -bevelCorner[1].width(), -(tr.height()-1)), bevelCorner[2]);
    }

    if (m_contAware)
    {
        if (!m_mem)
            m_mem = new QSharedMemory(QString::number(windowId()), this);
        if ((m_mem->isAttached() || m_mem->attach(QSharedMemory::ReadOnly)) && m_mem->lock())
        {
            p.setOpacity(dConf.uno.opacity);
            const uchar *data(reinterpret_cast<const uchar *>(m_mem->constData()));
            p.drawImage(QPoint(0, 0), QImage(data, widget()->width(), m_headHeight, QImage::Format_ARGB32_Premultiplied), tr);
            p.setOpacity(1.0f);
            m_mem->unlock();
        }
    }

    QFont f(p.font());
    f.setBold(isActive());
    p.setFont(f);

    QString text(caption());
    const QRect textRect(tr.adjusted(dConf.deco.icon&&!icon().isNull()*20, 0, 0, 0));
    const int maxW(textRect.width()-(qMax(m_leftButtons, m_rightButtons)*2));
    if (p.fontMetrics().width(text) > maxW)
        text = p.fontMetrics().elidedText(text, Qt::ElideRight, maxW);

    if (isActive())
    {
        const int rgb(isDark?0:255);
        p.setPen(QColor(rgb, rgb, rgb, 127));
        p.drawText(textRect.translated(0, 1), Qt::AlignCenter, text);
    }

    p.setPen(fgColor());
    p.drawText(textRect, Qt::AlignCenter, text);

    if (dConf.deco.icon && !icon().isNull())
    {
        QRect ir(p.fontMetrics().boundingRect(textRect, Qt::AlignCenter, text).left()-20, tr.height()/2-8, 16, 16);
        ir.moveTop(tr.top()+(tr.height()/2-ir.height()/2));
        if (ir.left() > m_leftButtons)
            icon().paint(&p, ir, Qt::AlignCenter, isActive()?QIcon::Active:QIcon::Disabled);
    }

    if (m_needSeparator)
    {
        QRectF r(tr);
        r.translate(-0.5f, -0.5f);
        p.setPen(QColor(0, 0, 0, 32));
        p.drawLine(r.bottomLeft(), r.bottomRight());
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
        Render::shapeCorners(widget(), &p, Render::All);

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
    if (!isPreview())
        ShadowHandler::installShadows(windowId(), isActive());
    widget()->update();
}

void
KwinClient::updateContBg()
{
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
}

void
KwinClient::readCompositing()
{
    m_compositingActive = XHandler::compositingActive();
    updateMask();
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

    if (changed & SettingDecoration)
    {
        bool needBg(true);
        WindowData *wd = reinterpret_cast<WindowData *>(XHandler::getXProperty<unsigned int>(windowId(), XHandler::WindowData));
        if (wd && !isPreview())
        {
            const int height((wd->data & WindowData::UnoHeight) >> 16);
            m_contAware = wd->data & WindowData::ContAware;
            m_headHeight = height;
            m_needSeparator = wd->data & WindowData::Separator;
            m_custcol[Text] = QColor::fromRgba(wd->text);
            m_custcol[Bg] = QColor::fromRgba(wd->bg);
            m_opacity = (float)((wd->data & WindowData::Opacity) >> 8)/100.0f;
            m_uno = wd->data & WindowData::Uno;
            m_buttonStyle = ((wd->data & WindowData::Buttons) >> 24) -1;
            m_frameSize = (wd->data & WindowData::Frame) >> 28;
            XFree(wd);
        }
        else if (needBg && m_uno)
        {
            m_needSeparator = true;
            m_buttonStyle = dConf.deco.buttons;
            for (int i = 0; i < 2; ++i)
            {
                QRect r(0, 0, width(), m_headHeight);
                QLinearGradient lg(r.topLeft(), r.bottomLeft());
                lg.setColorAt(0.0f, Color::mid(options()->color(ColorTitleBar, i), Qt::white, 4, 1));
                lg.setColorAt(1.0f, Color::mid(options()->color(ColorTitleBar, i), Qt::black, 4, 1));
                QPixmap p(Render::noise().size().width(), m_headHeight);
                p.fill(Qt::transparent);
                QPainter pt(&p);
                pt.fillRect(p.rect(), lg);
                pt.end();
                m_bgPix[i] = Render::mid(p, Render::noise(), 40, 1);
            }
        }
    }
    if (changed & SettingCompositing)
        QTimer::singleShot(2000, this, SLOT(readCompositing()));
    updateMask();
}
