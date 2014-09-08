
#include <X11/Xatom.h>
#include "../stylelib/xhandler.h"

#include <kwindowinfo.h>
#include <QPainter>
#include <QHBoxLayout>
#include <QTimer>
#include <QPixmap>

#include "kwinclient.h"
#include "button.h"
#include "widgets.h"
#include "../stylelib/ops.h"
#include "../stylelib/shadowhandler.h"
#include "../stylelib/render.h"
#include "../stylelib/color.h"
#include "../stylelib/settings.h"

#define TITLEHEIGHT 22
#define MARGIN 6
#define SPACING 4

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
        * @li 'N' application menu button
        * @li 'M' window menu button
        * @li 'S' on_all_desktops button
        * @li 'H' quickhelp button
        * @li 'I' minimize ( iconify ) button
        * @li 'A' maximize button
        * @li 'X' close button
        * @li 'F' keep_above_others button
        * @li 'B' keep_below_others button
        * @li 'L' shade button
        * @li 'R' resize button
        * @li '_' spacer
        */
        case 'X': t = Button::Close; break;
        case 'I': t = Button::Min; break;
        case 'A': t = Button::Max; break;
        case '_': m_titleLayout->addSpacing(SPACING); supported = false; size += SPACING; break;
        default: supported = false; break;
        }
        if (supported)
        {
            Button *b = new Button(t, this, widget());
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
    Render::renderMask(tr, &p, m_brush, 4, Render::All & ~Render::Bottom);
    p.setOpacity(1.0f);
    QLinearGradient lg(tr.topLeft(), tr.bottomLeft());
    lg.setColorAt(0.0f, QColor(255, 255, 255, 127));
    lg.setColorAt(0.5f, Qt::transparent);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(lg, 1.0f));
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(QRectF(tr).translated(0.5f, 0.5f), 5, 5);
    QFont f(p.font());
    f.setBold(isActive());
    p.setFont(f);

    QString text(caption());
    const int maxW(tr.width()-(qMax(buttonCornerWidth(true), buttonCornerWidth(false))*2));
    if (p.fontMetrics().width(text) > maxW)
        text = p.fontMetrics().elidedText(text, Qt::ElideRight, maxW);

    if (isActive())
    {
        p.setPen(QColor(255, 255, 255, 127));
        p.drawText(tr.translated(0, 1), Qt::AlignCenter, text);
    }
    if (m_textColor.isValid())
        p.setPen(m_textColor);
    else
        p.setPen(options()->color(KwinClient::ColorFont, isActive()));

    p.drawText(tr, Qt::AlignCenter, text);

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
    }
    int n(0);
    WindowData *data = reinterpret_cast<WindowData *>(XHandler::getXProperty<unsigned int>(windowId(), XHandler::WindowData, n));
    if (data && !isPreview())
    {
        m_titleColor[0] = QColor::fromRgba(data->top);
        m_titleColor[1] = QColor::fromRgba(data->bottom);
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
    QRect r(0, 0, width(), m_headHeight);
    m_unoGradient = QLinearGradient(r.topLeft(), r.bottomLeft());
    m_unoGradient.setColorAt(0.0f, m_titleColor[0]);
    m_unoGradient.setColorAt(1.0f, m_titleColor[1]);
    QPixmap p(Render::noise().size().width(), m_headHeight);
    p.fill(Qt::transparent);
    QPainter pt(&p);
    pt.fillRect(p.rect(), m_unoGradient);
    pt.end();
    m_brush = QBrush(Render::mid(p, Render::noise(), 40, 1));
    widget()->update();
    if (isPreview())
        return;

    ShadowHandler::installShadows(windowId());
    if (!m_sizeGrip)
        m_sizeGrip = new SizeGrip(this);
}
