#include <QPainter>
#include <QMouseEvent>

#include "button.h"
#include "../stylelib/ops.h"
#include "../stylelib/color.h"

Button::Button(Type type, KwinClient *client, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_hasPress(false)
    , m_client(client)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_Hover);
    setFixedSize(16, 16);
    for (int i = 0; i < TypeCount; ++i)
        m_paintEvent[i] = 0;

    m_paintEvent[Close] = &Button::paintClose;
    m_paintEvent[Min] = &Button::paintMin;
    m_paintEvent[Max] = &Button::paintMax;
}

Button::~Button()
{

}

void
Button::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBrushOrigin(-mapToParent(rect().topLeft()));
//    p.fillRect(rect(), m_client->titleBar()->brush());
    p.setBrushOrigin(rect().topLeft());
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
        {
            emit clicked();
            switch (m_type)
            {
            case Close: m_client->closeWindow(); break;
            case Min: m_client->minimize(); break;
            case Max: m_client->maximize(e->button()); break;
            default: break;
            }
        }
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
    p.save();
    p.setPen(Qt::NoPen);
    r.adjust(2, 2, -2, -2);
    p.setBrush(Color::mid(c, Qt::black, 4, 1));
    p.drawEllipse(r);
    p.setBrush(c);
    r.adjust(1, 1, -1, -1);
    p.drawEllipse(r);
    p.restore();
#if 0
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
#endif
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
Button::paintClose(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
//    QRectF penRect(QRectF(rect()).adjusted(0.5f, 0.5f, -1.0f, -1.0f));
    QRect r(rect());
    drawBase(m_client->isActive()?fcolors[m_type]:QColor(127, 127, 127, 255), p, r);
    if (underMouse())
    {
        p.setBrush(m_client->options()->color(KDecoration::ColorFont));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
}

bool
Button::paintMax(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
//    QRectF penRect(QRectF(rect()).adjusted(0.5f, 0.5f, -1.0f, -1.0f));
    QRect r(rect());
    drawBase(m_client->isActive()?fcolors[m_type]:QColor(127, 127, 127, 255), p, r);
    if (underMouse())
    {
        p.setBrush(m_client->options()->color(KDecoration::ColorFont));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
}

bool
Button::paintMin(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
//    QRectF penRect(QRectF(rect()).adjusted(0.5f, 0.5f, -1.0f, -1.0f));
    QRect r(rect());
    drawBase(m_client->isActive()?fcolors[m_type]:QColor(127, 127, 127, 255), p, r);
    if (underMouse())
    {
        p.setBrush(m_client->options()->color(KDecoration::ColorFont));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
}
