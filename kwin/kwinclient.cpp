
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

#define TITLEHEIGHT 22
#define MARGIN 6
#define SPACING 4

///-------------------------------------------------------------------

DButton::DButton(const Type &t, KwinClient *client, QWidget *parent)
    : Button(t, parent)
    , m_client(client)
{
}

const DButton::ButtonStyle
DButton::buttonStyle() const
{
    return m_client->m_buttonStyle;
}

const bool
DButton::isDark() const
{
    QColor fgc(m_client->options()->color(KDecoration::ColorFont, m_client->isActive()));
    if (m_client->m_custcol[Fg].isValid())
        fgc = m_client->m_custcol[Fg];
//    if (c == Fg)
//        return fgc;
    QColor bgc(m_client->options()->color(KDecoration::ColorTitleBar, m_client->isActive()));
    if (m_client->m_custcol[Bg].isValid())
        bgc = m_client->m_custcol[Bg];
//    if (c == Bg)
//        return bgc;
//    return Color::mid(fgc, bgc);
    return Color::luminosity(fgc) > Color::luminosity(bgc);
}

const QColor
DButton::color(const ColorRole &c) const
{
    return Button::color(c);
//    QColor fgc(m_client->options()->color(KDecoration::ColorFont, m_client->isActive()));
//    if (m_client->m_custcol[Fg].isValid())
//        fgc = m_client->m_custcol[Fg];
//    if (c == Fg)
//        return fgc;
//    QColor bgc(m_client->options()->color(KDecoration::ColorTitleBar, m_client->isActive()));
//    if (m_client->m_custcol[Bg].isValid())
//        bgc = m_client->m_custcol[Bg];
//    if (c == Bg)
//        return bgc;
//    return Color::mid(fgc, bgc);
}

bool
DButton::isActive() const
{
    return m_client?m_client->isActive():true;
}

void
DButton::onClick(QMouseEvent *e, const Type &t)
{
    switch (t)
    {
    case Close: m_client->closeWindow(); break;
    case Min: m_client->minimize(); break;
    case Max: m_client->maximize(e->button()); break;
    case OnAllDesktops: m_client->toggleOnAllDesktops(); break;
    case WindowMenu: m_client->showWindowMenu(mapToGlobal(rect().bottomLeft())); break;
    case KeepAbove: m_client->setKeepAbove(!m_client->keepAbove()); break;
    case KeepBelow: m_client->setKeepBelow(!m_client->keepBelow()); break;
    case AppMenu: m_client->showApplicationMenu(mapToGlobal(rect().bottomLeft())); break;
    case Shade: m_client->setShade(!m_client->isShade()); break;
    case QuickHelp: m_client->showContextHelp(); break;
    default: break;
    }
    QList<DButton *> buttons(m_client->widget()->findChildren<DButton *>());
    for (int i = 0; i < buttons.count(); ++i)
        buttons.at(i)->update();
}

bool
DButton::paintMaxButton(QPainter &p)
{
    if (m_client->m_buttonStyle == -1)
    {
        const int s(rect().width()/8);
        QRect r = rect().adjusted(s, s, -s, -s);
        const QPen pen(m_client->maximizeMode()==KDecoration::MaximizeFull?palette().color(QPalette::Highlight):color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        r.adjust(s, s, -s, -s);
        QPixmap pix(rect().size());
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        int x, y, w, h;
        r.getRect(&x, &y, &w, &h);
        pt.drawLine(x+w/2, y, x+w/2, y+h);
        pt.drawLine(x, y+h/2, x+w, y+h/2);
        pt.end();
        p.drawTiledPixmap(rect(), Render::sunkenized(rect(), pix, isDark(), color(Mid)));
        p.end();
        return true;
    }
    else
        return Button::paintMaxButton(p);
}

bool
DButton::paintOnAllDesktopsButton(QPainter &p)
{
    const int s(rect().width()/8);
    QRect r = rect().adjusted(s, s, -s, -s);
    const QPen pen(color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    r.adjust(s, s, -s, -s);
    QPixmap pix(rect().size());
    pix.fill(Qt::transparent);
    QPainter pt(&pix);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.setPen(pen);
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);

    pt.drawLine(x, y+h/2, x+w, y+h/2);
    if (!m_client->isOnAllDesktops())
        pt.drawLine(x+w/2, y, x+w/2, y+h);
    pt.end();
    p.drawTiledPixmap(rect(), Render::sunkenized(rect(), pix, isDark(), color(Mid)));
    p.end();
    return true;
}

bool
DButton::paintWindowMenuButton(QPainter &p)
{
    const int s(rect().width()/8);
    QRect r = rect().adjusted(s, s, -s, -s);
    const QPen pen(color(Mid), s, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    r.adjust(s, s, -s, -s);
    QPixmap pix(rect().size());
    pix.fill(Qt::transparent);
    QPainter pt(&pix);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.setPen(pen);
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);
    for (int i = 0; i < 3; ++i)
        pt.drawLine(x, y+i*4, x+w, y+i*4);
    pt.end();
    p.drawTiledPixmap(rect(), Render::sunkenized(rect(), pix, isDark(), color(Mid)));
    p.end();
    return true;
}

bool
DButton::paintKeepAboveButton(QPainter &p)
{
    const int s(rect().width()/8);
    QRect r = rect().adjusted(s, s, -s, -s);
    const QPen pen(m_client->keepAbove()?palette().color(QPalette::Highlight):color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    r.adjust(s, s, -s, -s);
    QPixmap pix(rect().size());
    pix.fill(Qt::transparent);
    QPainter pt(&pix);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.setPen(pen);
    pt.setBrush(pt.pen().color());
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);
    static const int points[] = { x,y+h/2, x+w/2,y, x+w,y+h/2 };
    QPolygon polygon;
    polygon.setPoints(3, points);
    pt.drawPolygon(polygon);
    pt.end();
    p.drawTiledPixmap(rect(), Render::sunkenized(rect(), pix, isDark(), color(Mid)));
    p.end();
    return true;
}

bool
DButton::paintKeepBelowButton(QPainter &p)
{
    const int s(rect().width()/8);
    QRect r = rect().adjusted(s, s, -s, -s);
    const QPen pen(m_client->keepBelow()?palette().color(QPalette::Highlight):color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    r.adjust(s, s, -s, -s);
    QPixmap pix(rect().size());
    pix.fill(Qt::transparent);
    QPainter pt(&pix);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.setPen(pen);
    pt.setBrush(pt.pen().color());
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);
    static const int points[] = { x,y+h/2, x+w/2,y+h, x+w,y+h/2 };
    QPolygon polygon;
    polygon.setPoints(3, points);
    pt.drawPolygon(polygon);
    pt.end();
    p.drawTiledPixmap(rect(), Render::sunkenized(rect(), pix, isDark(), color(Mid)));
    p.end();
    return true;
}

bool
DButton::paintApplicationMenuButton(QPainter &p)
{
    return paintWindowMenuButton(p);
}

bool
DButton::paintShadeButton(QPainter &p)
{
    const int s(rect().width()/8);
    QRect r = rect().adjusted(s, s, -s, -s);
    const QPen pen(m_client->isShade()?palette().color(QPalette::Highlight):color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    r.adjust(s, s, -s, -s);
    QPixmap pix(rect().size());
    pix.fill(Qt::transparent);
    QPainter pt(&pix);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.setPen(pen);
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);
    pt.drawLine(x, y+h/3, x+w, y+h/3);
    pt.end();
    p.drawTiledPixmap(rect(), Render::sunkenized(rect(), pix, isDark(), color(Mid)));
    p.end();
    return true;
}

bool
DButton::paintQuickHelpButton(QPainter &p)
{
    const int s(rect().height()/8);
    QRectF r(rect().adjusted(s, s, -s, -s));
    QFont f(p.font());
    f.setWeight(QFont::Black);
    f.setPixelSize(r.height()*1.5f);
    QPixmap pix(rect().size());
    pix.fill(Qt::transparent);
    const float rnd(r.height()/4.0f);
    const float x(r.x());
    const float y(r.y());
    QPainterPath path;
    path.moveTo(x, y+rnd);
    path.quadTo(r.topLeft(), QPointF(x+rnd, y));
    path.quadTo(QPointF(x+rnd*2, y), QPointF(x+rnd*2, y+rnd));
    path.quadTo(QPointF(x+rnd*2, y+rnd*2), QPointF(x+rnd, y+rnd*2));
    path.lineTo(QPointF(x+rnd, y+rnd*2+rnd/2));

    QPainter pt(&pix);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.setPen(QPen(color(Mid), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    pt.setBrush(Qt::NoBrush);
    pt.drawPath(path.translated(rnd, 0));
    pt.setPen(Qt::NoPen);
    pt.setBrush(color(Mid));
    pt.drawEllipse(QPoint(x+rnd+rnd, y+rnd*4), 2, 2);
    pt.end();
    p.drawTiledPixmap(rect(), Render::sunkenized(rect(), pix, isDark(), color(Mid)));
    p.end();
    return true;
}

///-------------------------------------------------------------------

KwinClient::KwinClient(KDecorationBridge *bridge, Factory *factory)
    : KDecoration(bridge, factory)
    , m_titleLayout(0)
    , m_headHeight(TITLEHEIGHT)
    , m_needSeparator(true)
    , m_factory(factory)
    , m_opacity(1.0f)
    , m_sizeGrip(0)
    , m_mem(0)
    , m_contAware(false)
    , m_uno(true)
    , m_frameSize(0)
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

    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setSpacing(SPACING);
    m_titleLayout->setContentsMargins(MARGIN, 3, MARGIN, 3);
    QVBoxLayout *l = new QVBoxLayout(widget());
    l->setSpacing(0);
    l->setContentsMargins(dConf.deco.frameSize, dConf.deco.frameSize, dConf.deco.frameSize, dConf.deco.frameSize);
    l->addLayout(m_titleLayout);
    l->addStretch();
    widget()->setLayout(l);
    m_needSeparator = true;
    if (!isPreview() && windowId())
    {
        unsigned char height(TITLEHEIGHT);
        XHandler::setXProperty<unsigned char>(windowId(), XHandler::DecoData, XHandler::Byte, &height); //never higher then 255...
        ShadowHandler::installShadows(windowId());
        if (isResizable() && !m_sizeGrip)
            m_sizeGrip = new SizeGrip(this);
    }
    reset(63);
}

bool
KwinClient::compositingActive() const
{
    return m_factory->compositingActive();
}

void
KwinClient::populate(const QString &buttons, bool left)
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
            DButton *b = new DButton(t, this, widget());
            size += b->width()+SPACING;
            m_titleLayout->addWidget(b);
        }
    }
    if (left)
        m_leftButtons = size+MARGIN;
    else
        m_rightButtons = size+MARGIN;
}

void
KwinClient::updateMask()
{
    const int w(width()), h(height());
    if (compositingActive())
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
    top = TITLEHEIGHT+f;
}

void
KwinClient::captionChange()
{
    widget()->update();
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
        QPainter p(widget());
        p.setRenderHint(QPainter::Antialiasing);
        paint(p);
        p.end();
        return true;
    }
    case QEvent::MouseButtonDblClick:
        titlebarDblClickOperation();
        return true;
    case QEvent::MouseButtonPress:
        processMousePressEvent(static_cast<QMouseEvent *>(e));
        return true;
    case QEvent::Wheel:
        titlebarMouseWheelOperation(static_cast<QWheelEvent *>(e)->delta());
        return true;
    default: break;
    }
    return KDecoration::eventFilter(o, e);
}

void
KwinClient::paint(QPainter &p)
{
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(widget()->rect(), Qt::transparent);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.setBrushOrigin(widget()->rect().topLeft());

    p.fillRect(widget()->rect(), Color::mid(widget()->palette().color(widget()->foregroundRole()), widget()->palette().color(widget()->backgroundRole())));
    if (dConf.deco.frameSize && maximizeMode() != MaximizeFull)
    {
        QRectF r(widget()->layout()->geometry().adjusted(dConf.deco.frameSize, dConf.deco.frameSize, -dConf.deco.frameSize, -dConf.deco.frameSize));
        r.adjust(-0.5f, -0.5f, 0.5f, 0.5f);
        p.setPen(QColor(0, 0, 0, 127));
        p.setBrush(Qt::NoBrush);
        p.drawRect(r);
        p.setPen(QColor(0, 0, 0, 127));
        p.setBrush(QColor(0, 0, 0, 63));
        p.drawRect(QRectF(positionRect(PositionTopLeft)).adjusted(-0.5f, -0.5f, 0.5f, 0.5f));
        p.drawRect(QRectF(positionRect(PositionTopRight)).adjusted(-0.5f, -0.5f, 0.5f, 0.5f));
        p.drawRect(QRectF(positionRect(PositionBottomLeft)).adjusted(-0.5f, -0.5f, 0.5f, 0.5f));
        p.drawRect(QRectF(positionRect(PositionBottomRight)).adjusted(-0.5f, -0.5f, 0.5f, 0.5f));
        p.setBrush(Qt::NoBrush);
        p.setPen(QColor(255, 255, 255, 63));
        p.drawRoundedRect(QRectF(widget()->rect()).adjusted(0.5f, 0.5f, -0.5f, -0.5f), 4, 4);
    }

    p.setPen(Qt::NoPen);
    p.setBrush(Qt::NoBrush);
    const QRect tr(m_titleLayout->geometry());
    p.setOpacity(m_opacity);

    if (!m_bgPix[isActive()].isNull())
        p.drawTiledPixmap(tr, m_bgPix[isActive()]);
    else
        p.fillRect(tr, bgColor());
    p.setOpacity(1.0f);

    const int bgLum(Color::luminosity(bgColor()));
    const bool isDark(Color::luminosity(fgColor()) > bgLum);

    if (!dConf.deco.frameSize || maximizeMode() == MaximizeFull)
    {
        const QRectF bevel(0.5f, 0.5f, width()-0.5f, 10.0f);
        QLinearGradient lg(bevel.topLeft(), bevel.bottomLeft());
        lg.setColorAt(0.0f, QColor(255, 255, 255, qMin(255.0f, bgLum*1.1f)));
        lg.setColorAt(0.5f, Qt::transparent);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(lg, 1.0f));
        p.drawRoundedRect(bevel, 5, 5);
    }

    if (m_contAware)
    {
        if (!m_mem)
            m_mem = new QSharedMemory(QString::number(windowId()), this);
        if ((m_mem->isAttached() || m_mem->attach(QSharedMemory::ReadOnly)))
            if (m_mem->lock())
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
            p.drawTiledPixmap(ir, icon().pixmap(16));
    }

    if (m_needSeparator)
    {
        QRectF r(tr);
        r.translate(-0.5f, -0.5f);
        p.setPen(QColor(0, 0, 0, 32));
        p.drawLine(r.bottomLeft(), r.bottomRight());
    }
    p.setClipping(false);
    if (isPreview())
    {
        p.setClipRegion(QRegion(widget()->rect())-QRegion(tr));
        p.fillRect(widget()->rect(), widget()->palette().color(QPalette::Window));
        p.setPen(widget()->palette().color(QPalette::WindowText));
        p.drawText(p.clipBoundingRect(), Qt::AlignCenter, "DSP");
        p.setClipping(false);
    }
    else
        Render::shapeCorners(widget(), &p, Render::All);
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
    for (int i = 0; i < m_buttons.count(); ++i)
        m_buttons.at(i)->update();
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

void
KwinClient::maximizeChange()
{
    const int fs((maximizeMode()!=MaximizeFull)*dConf.deco.frameSize);
    widget()->layout()->setContentsMargins(fs, fs, fs, fs);
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
        populate(options()->titleButtonsLeft(), true);
        m_titleLayout->addStretch();
        populate(options()->titleButtonsRight(), false);
        m_buttons = widget()->findChildren<Button * >();
    }

    if (changed & SettingDecoration)
    {
        bool needBg(true);
        if (unsigned long *bg = XHandler::getXProperty<unsigned long>(windowId(), XHandler::DecoBgPix))
        {
            m_bgPix[0] = m_bgPix[1] = QPixmap::fromX11Pixmap(*bg);
            needBg = false;
        }

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
    for (int i = 0; i < m_buttons.count(); ++i)
        m_buttons.at(i)->update();

    updateMask();
}
