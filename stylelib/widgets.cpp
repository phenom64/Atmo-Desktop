#include "widgets.h"
#include "settings.h"
#include "color.h"
#include "render.h"
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QImage>
#include "xhandler.h"

Button::Button(Type type, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_hasPress(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_Hover);
    setFixedSize(16, 16);
    for (int i = 0; i < TypeCount; ++i)
        m_paintEvent[i] = 0;

    m_paintEvent[Close] = &Button::paintCloseButton;
    m_paintEvent[Min] = &Button::paintMinButton;
    m_paintEvent[Max] = &Button::paintMaxButton;
    m_paintEvent[OnAllDesktops] = &Button::paintOnAllDesktopsButton;
    m_paintEvent[WindowMenu] = &Button::paintWindowMenuButton;
    m_paintEvent[KeepAbove] = &Button::paintKeepAboveButton;
    m_paintEvent[KeepBelow] = &Button::paintKeepBelowButton;
    m_paintEvent[Shade] = &Button::paintShadeButton;
    m_paintEvent[Resize] = &Button::paintResizeButton;
    m_paintEvent[QuickHelp] = &Button::paintQuickHelpButton;
    m_paintEvent[AppMenu] = &Button::paintApplicationMenuButton;
}

Button::~Button()
{

}

void
Button::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (!XHandler::compositingActive() && !m_bgPix.isNull())
        p.drawTiledPixmap(rect(), m_bgPix, geometry().topLeft());
    if (m_type < TypeCount && m_paintEvent[m_type] && (this->*m_paintEvent[m_type])(p))
        return;
    p.end();
}

void
Button::mousePressEvent(QMouseEvent *e)
{
//    QWidget::mousePressEvent(e);
    e->accept();
    m_hasPress = true;
}

void
Button::mouseReleaseEvent(QMouseEvent *e)
{
//    QWidget::mouseReleaseEvent(e);
    if (m_hasPress)
    {
        e->accept();
        m_hasPress = false;
        if (rect().contains(e->pos()))
            onClick(e, m_type);
    }
}

/**
 * @brief Button::drawBase
 * @param c
 * @param p
 * @param r
 * draw a maclike decobuttonbase using color c
 */

void
Button::drawBase(QColor c, QPainter &p, QRect &r) const
{
    switch (Settings::conf.deco.buttons)
    {
    case 0:
    {
        p.save();
        p.setPen(Qt::NoPen);
        r.adjust(2, 2, -2, -2);
        p.setBrush(Color::mid(c, Qt::black, 4, 1));
        p.drawEllipse(r);
        p.setBrush(c);
        r.adjust(1, 1, -1, -1);
        p.drawEllipse(r);
        p.restore();
        break;
    }
    case 1:
    {
        c.setHsv(c.hue(), qMax(164, c.saturation()), c.value(), c.alpha());
        const QColor low(Color::mid(c, Qt::black, 5, 3));
        const QColor high(QColor(255, 255, 255, 127));
        r.adjust(2, 2, -2, -2);
        p.setBrush(high);
        p.drawEllipse(r.translated(0, 1));
        p.setBrush(low);
        p.drawEllipse(r);
        r.adjust(1, 1, -1, -1);

        QRadialGradient rg(r.center()+QPoint(1, r.height()/2-1), r.height()-1);
        rg.setColorAt(0.0f, Color::mid(c, Qt::white));
        rg.setColorAt(0.5f, c);

        rg.setColorAt(1.0f, Color::mid(c, Qt::black));
        p.setBrush(rg);
        p.drawEllipse(r);

        QRect rr(r);
        rr.setWidth(6);
        rr.setHeight(3);
        rr.moveCenter(r.center());
        rr.moveTop(r.top()+1);
        QLinearGradient lg(rr.topLeft(), rr.bottomLeft());
        lg.setColorAt(0.0f, QColor(255, 255, 255, 192));
        lg.setColorAt(1.0f, QColor(255, 255, 255, 64));
        p.setBrush(lg);
        p.drawEllipse(rr);
        break;
    }
    case 2:
    {
        c.setHsv(c.hue(), qMax(164, c.saturation()), c.value(), c.alpha());
        const QColor low(Color::mid(c, Qt::black, 5, 3));
        const QColor high(QColor(255, 255, 255, 127));
        r.adjust(2, 2, -2, -2);
        p.setBrush(high);
        p.drawEllipse(r.translated(0, 1));
        p.setBrush(low);
        p.drawEllipse(r);
        r.adjust(1, 1, -1, -1);

        QRadialGradient rg(r.center()+QPoint(1, r.height()*0.2f), r.height()*0.66f);
        rg.setColorAt(0.0f, c);
        rg.setColorAt(0.33f, c);
        rg.setColorAt(1.0f, Qt::transparent);
        p.setBrush(rg);
        p.drawEllipse(r);
        break;
    }
    default: break;
    }
}

/** Stolen colors from bespin....
 *  none of them are perfect, but
 *  the uncommented ones are 'good enough'
 *  for me, search the web/pick better
 *  ones yourself if not happy
 */

// static uint fcolors[3] = {0x9C3A3A/*0xFFBF0303*/, 0xFFEB55/*0xFFF3C300*/, 0x77B753/*0xFF00892B*/};
// Font
//static uint fcolors[3] = {0xFFBF0303, 0xFFF3C300, 0xFF00892B};
// Aqua
// static uint fcolors[3] = { 0xFFD86F6B, 0xFFD8CA6B, 0xFF76D86B };

// static uint fcolors[3] = { 0xFFFF7E71, 0xFFFBD185, 0xFFB1DE96 }; <<<-best from bespin
// Aqua2
// static uint fcolors[3] = { 0xFFBF2929, 0xFF29BF29, 0xFFBFBF29 };

static uint fcolors[3] = { 0xFFFF7E71, 0xFFFBD185, 0xFF37CC40 };

bool
Button::paintCloseButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(rect());
    if (Settings::conf.deco.buttons == -1)
    {
        const int s(rect().width()/8);
        r = rect().adjusted(s, s, -s, -s);
        const QPen pen(palette().color(foregroundRole()), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        r.adjust(s, s, -s, -s);
        QPixmap pix(rect().size());
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        pt.drawLine(r.topLeft(), r.bottomRight());
        pt.drawLine(r.topRight(), r.bottomLeft());
        pt.end();
        p.drawTiledPixmap(rect(), sunkenized(rect(), pix));
    }
    else
    {
        drawBase(isActive()?fcolors[m_type]:QColor(127, 127, 127, 255), p, r);
        if (underMouse())
        {
            p.setBrush(palette().color(foregroundRole()));
            p.drawEllipse(r.adjusted(3, 3, -3, -3));
        }
    }
    p.end();
    return true;
}

bool
Button::paintMaxButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(rect());
    drawBase(isActive()?fcolors[m_type]:QColor(127, 127, 127, 255), p, r);
    if (underMouse())
    {
        p.setBrush(palette().color(foregroundRole()));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
}

bool
Button::paintMinButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(rect());
    if (Settings::conf.deco.buttons == -1)
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
        pt.end();
        p.drawTiledPixmap(rect(), sunkenized(rect(), pix));
    }
    else
    {
        drawBase(isActive()?fcolors[m_type]:QColor(127, 127, 127, 255), p, r);
        if (underMouse())
        {
            p.setBrush(palette().color(foregroundRole()));
            p.drawEllipse(r.adjusted(3, 3, -3, -3));
        }
    }
    p.end();
    return true;
}

QPixmap
Button::sunkenized(const QRect &r, const QPixmap &source)
{
    QImage img(r.size(), QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.fillRect(img.rect(), QColor(255, 255, 255, 127));
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawTiledPixmap(r, source);
    p.end();
    const QImage blur = Render::blurred(img, img.rect(), 2);

    QPixmap pix(r.size());
    pix.fill(Qt::transparent);
    p.begin(&pix);
    p.drawTiledPixmap(pix.rect(), source);
    p.drawPixmap(pix.rect().translated(0, 1), QPixmap::fromImage(blur));
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawPixmap(pix.rect(), source);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    p.drawPixmap(pix.rect().translated(0, 1), QPixmap::fromImage(img));
    p.end();
    return pix;
}
