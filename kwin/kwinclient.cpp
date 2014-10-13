
#include <X11/Xatom.h>
#include "../stylelib/xhandler.h"

#include <kwindowinfo.h>
#include <QPainter>
#include <QHBoxLayout>
#include <QTimer>
#include <QPixmap>
#include <QCoreApplication>

#include "kwinclient.h"
#include "../stylelib/ops.h"
#include "../stylelib/shadowhandler.h"
#include "../stylelib/render.h"
#include "../stylelib/color.h"
#include "../stylelib/settings.h"

#define TITLEHEIGHT 22
#define MARGIN 6
#define SPACING 4

///-------------------------------------------------------------------

DButton::DButton(const Type &t, KwinClient *client, QWidget *parent)
    : Button(t, parent)
    , m_client(client)
{
}

bool
DButton::isActive()
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
    case WindowMenu: m_client->showWindowMenu(e->globalPos()); break;
    case KeepAbove: m_client->setKeepAbove(!m_client->keepAbove()); break;
    case KeepBelow: m_client->setKeepBelow(!m_client->keepBelow()); break;
    case AppMenu: m_client->showApplicationMenu(e->globalPos()); break;
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
    if (Settings::conf.deco.buttons == -1)
    {
        const int s(rect().width()/8);
        QRect r = rect().adjusted(s, s, -s, -s);
        const QPen pen(palette().color(m_client->maximizeMode()==KDecoration::MaximizeFull?QPalette::Highlight:foregroundRole()), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
        p.drawTiledPixmap(rect(), sunkenized(rect(), pix));
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
    const QPen pen(palette().color(foregroundRole()), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
    p.drawTiledPixmap(rect(), sunkenized(rect(), pix));
    p.end();
    return true;
}

bool
DButton::paintWindowMenuButton(QPainter &p)
{
    const int s(rect().width()/8);
    QRect r = rect().adjusted(s, s, -s, -s);
    const QPen pen(palette().color(foregroundRole()), s, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
    p.drawTiledPixmap(rect(), sunkenized(rect(), pix));
    p.end();
    return true;
}

bool
DButton::paintKeepAboveButton(QPainter &p)
{
    const int s(rect().width()/8);
    QRect r = rect().adjusted(s, s, -s, -s);
    const QPen pen(palette().color(m_client->keepAbove()?QPalette::Highlight:foregroundRole()), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
    p.drawTiledPixmap(rect(), sunkenized(rect(), pix));
    p.end();
    return true;
}

bool
DButton::paintKeepBelowButton(QPainter &p)
{
    const int s(rect().width()/8);
    QRect r = rect().adjusted(s, s, -s, -s);
    const QPen pen(palette().color(m_client->keepBelow()?QPalette::Highlight:foregroundRole()), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
    p.drawTiledPixmap(rect(), sunkenized(rect(), pix));
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
    const QPen pen(palette().color(m_client->isShade()?QPalette::Highlight:foregroundRole()), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
    p.drawTiledPixmap(rect(), sunkenized(rect(), pix));
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
    pt.setPen(QPen(palette().color(foregroundRole()), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    pt.setBrush(Qt::NoBrush);
    pt.drawPath(path.translated(rnd, 0));
    pt.setPen(Qt::NoPen);
    pt.setBrush(palette().color(foregroundRole()));
    pt.drawEllipse(QPoint(x+rnd+rnd, y+rnd*4), 2, 2);
    pt.end();
    p.drawTiledPixmap(rect(), sunkenized(rect(), pix));
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
    , m_sizeGrip(0)
    , m_opacity(1.0f)
{
    setParent(factory);
    Settings::read();
}

KwinClient::~KwinClient()
{
    if (m_sizeGrip)
        delete m_sizeGrip;
//    XHandler::deleteXProperty(windowId(), XHandler::DecoData);
}

void
KwinClient::init()
{
    if (!isPreview())
    {
        unsigned char height(TITLEHEIGHT);
        XHandler::setXProperty<unsigned char>(windowId(), XHandler::DecoData, XHandler::Byte, &height); //never higher then 255...
        ShadowHandler::installShadows(windowId());
        setAlphaEnabled(true);
    }
    createMainWidget();
    widget()->installEventFilter(this);
    widget()->setAttribute(Qt::WA_NoSystemBackground);
    widget()->setAutoFillBackground(false);

    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setSpacing(SPACING);
    m_titleLayout->setContentsMargins(MARGIN, 3, MARGIN, 3);
    QVBoxLayout *l = new QVBoxLayout(widget());
    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
    l->addLayout(m_titleLayout);
    l->addStretch();
    widget()->setLayout(l);
    m_titleColor[0] = Color::mid(options()->color(ColorTitleBar), Qt::white, 4, 1);
    m_titleColor[1] = Color::mid(options()->color(ColorTitleBar), Qt::black, 4, 1);
    m_needSeparator = true;
    if (isPreview())
        reset(63);
    else
        QTimer::singleShot(1, this, SLOT(postInit()));
}

void
KwinClient::postInit()
{
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
KwinClient::resize(const QSize &s)
{
    widget()->resize(s);
//    m_stretch->setFixedHeight(widget()->height()-TITLEHEIGHT);
    const int w(s.width()), h(s.height());
    if (compositingActive())
    {
        if (m_opacity < 1.0f)
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
}

void
KwinClient::borders(int &left, int &right, int &top, int &bottom) const
{
    left = 0;
    right = 0;
    top = TITLEHEIGHT;
    bottom = 0;
}

void
KwinClient::captionChange()
{
    widget()->update();
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
//    case QEvent::Show:
//    case QEvent::Resize:
//        m_titleLayout->setGeometry(QRect(0, 0, width(), TITLEHEIGHT));
//        m_stretch->setFixedHeight(widget()->height()-TITLEHEIGHT);
    default: break;
    }
    return KDecoration::eventFilter(o, e);
}

void
KwinClient::paint(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::NoBrush);
    const QRect tr(m_titleLayout->geometry());
    p.setClipRect(tr);
    p.setOpacity(m_opacity);
    if (!m_bgPix[0].isNull())
//        p.drawTiledPixmap(tr, m_bgPix[0]);
        Render::renderMask(tr, &p, m_bgPix[0], 4, Render::All & ~Render::Bottom);
    else
        Render::renderMask(tr, &p, m_brush, 4, Render::All & ~Render::Bottom);
    p.setOpacity(1.0f);
    QLinearGradient lg(tr.topLeft(), tr.bottomLeft());
    lg.setColorAt(0.0f, QColor(255, 255, 255, 127));
    lg.setColorAt(0.5f, Qt::transparent);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(lg, 1.0f));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(QRectF(tr).translated(0.5f, 0.5f), 5, 5);
    if (!m_bgPix[1].isNull())
    {
        p.setOpacity(0.33f);
        Render::renderMask(tr, &p, m_bgPix[1], 4, Render::All & ~Render::Bottom);
        p.setOpacity(1.0f);
    }

    QFont f(p.font());
    f.setBold(isActive());
    p.setFont(f);

    QString text(caption());
    const QRect textRect(tr.adjusted(Settings::conf.deco.icon&&!icon().isNull()*20, 0, 0, 0));
    const int maxW(textRect.width()-(qMax(m_leftButtons, m_rightButtons)*2));
    if (p.fontMetrics().width(text) > maxW)
        text = p.fontMetrics().elidedText(text, Qt::ElideRight, maxW);

    if (isActive())
    {
        p.setPen(QColor(255, 255, 255, 127));
        p.drawText(textRect.translated(0, 1), Qt::AlignCenter, text);
    }
    if (m_textColor.isValid())
        p.setPen(m_textColor);
    else
        p.setPen(options()->color(KwinClient::ColorFont, isActive()));

    p.drawText(textRect, Qt::AlignCenter, text);
    if (Settings::conf.deco.icon && !icon().isNull())
    {
        QRect ir(p.fontMetrics().boundingRect(textRect, Qt::AlignCenter, text).left()-20, tr.height()/2-8, 16, 16);
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
        ShadowHandler::installShadows(windowId());
    widget()->update();
    for (int i = 0; i < m_buttons.count(); ++i)
        m_buttons.at(i)->update();
    resize(widget()->size());
}

/**
 * These flags specify which settings changed when rereading Settings::conf.
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
    if ((changed & SettingButtons) || isPreview())
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
    int n(0);
    WindowData *data = reinterpret_cast<WindowData *>(XHandler::getXProperty<unsigned int>(windowId(), XHandler::WindowData, n));
    if (data && !isPreview())
    {
        m_headHeight = data->height;
        m_needSeparator = data->separator;
        m_textColor = QColor::fromRgba(data->text);
        m_opacity = (float)data->opacity/100.0f;
        XFree(data);
    }
    else
    {
        m_titleColor[0] = Color::mid(options()->color(ColorTitleBar), Qt::white, 4, 1);
        m_titleColor[1] = Color::mid(options()->color(ColorTitleBar), Qt::black, 4, 1);
        m_needSeparator = true;
    }
    if (unsigned long *bg = XHandler::getXProperty<unsigned long>(windowId(), XHandler::DecoBgPix))
        if (*bg && *bg != m_bgPix[0].handle())
            m_bgPix[0] = QPixmap::fromX11Pixmap(*bg, QPixmap::ExplicitlyShared);
    if (unsigned long *bg = XHandler::getXProperty<unsigned long>(windowId(), XHandler::ContPix))
        if (*bg && *bg != m_bgPix[1].handle())
            m_bgPix[1] = QPixmap::fromX11Pixmap(*bg, QPixmap::ExplicitlyShared);
    QRect r(0, 0, width(), m_headHeight);
    m_unoGradient = QLinearGradient(r.topLeft(), r.bottomLeft());
    m_unoGradient.setColorAt(0.0f, m_titleColor[0]);
    m_unoGradient.setColorAt(1.0f, m_titleColor[1]);
    QPixmap p(Render::noise().size().width(), m_headHeight);
    p.fill(Qt::transparent);
    QPainter pt(&p);
    pt.fillRect(p.rect(), m_unoGradient);
    pt.end();
    const QPixmap &bg = Render::mid(p, Render::noise(), 40, 1);
    for (int i = 0; i < m_buttons.count(); ++i)
    {
        m_buttons.at(i)->setBgPix(m_bgPix[0].isNull()?bg:m_bgPix[0]);
        m_buttons.at(i)->update();
    }
    m_brush = QBrush(bg);
    widget()->update();
    if (isPreview())
        return;

    ShadowHandler::installShadows(windowId());
    if (!m_sizeGrip)
        m_sizeGrip = new SizeGrip(this);
}
