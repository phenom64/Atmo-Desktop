#include <QPainter>
#include <QMouseEvent>

#include "button.h"
#include "../stylelib/ops.h"

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
    if (m_type < TypeCount && m_paintEvent[m_type] && (this->*m_paintEvent[m_type])())
        return;
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

void
Button::drawBase(const QColor &c, QPainter &p, QRect &r) const
{
    const QColor bg(QColor(255, 255, 255, 127));
    const QColor fg(QColor(0, 0, 0, 127));
    r.adjust(2, 2, -2, -2);
    p.setBrush(Ops::mid(Qt::white, bg));
    p.drawEllipse(r.translated(0, 1));
    p.setBrush(Ops::mid(Qt::white, fg));
    p.drawEllipse(r);
    r.adjust(1, 1, -1, -1);
    QRadialGradient rg(r.center()+QPoint(1, 2), r.width()/2);
    rg.setColorAt(0.4f, c);
    rg.setColorAt(1.0f, Ops::mid(c, Qt::black, 3, 1));
    p.setBrush(rg);
    p.drawEllipse(r);
}

static uint fcolors[3] = { 0xFFD86F6B, 0xFFD8CA6B, 0xFF76D86B };

bool
Button::paintClose()
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
//    QRectF penRect(QRectF(rect()).adjusted(0.5f, 0.5f, -1.0f, -1.0f));
    QRect r(rect());
    drawBase(fcolors[m_type], p, r);
    if (underMouse())
    {
        p.setBrush(m_client->options()->color(KDecoration::ColorFont));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
}

bool
Button::paintMax()
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
//    QRectF penRect(QRectF(rect()).adjusted(0.5f, 0.5f, -1.0f, -1.0f));
    QRect r(rect());
    drawBase(fcolors[m_type], p, r);
    if (underMouse())
    {
        p.setBrush(m_client->options()->color(KDecoration::ColorFont));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
}

bool
Button::paintMin()
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
//    QRectF penRect(QRectF(rect()).adjusted(0.5f, 0.5f, -1.0f, -1.0f));
    QRect r(rect());
    drawBase(fcolors[m_type], p, r);
    if (underMouse())
    {
        p.setBrush(m_client->options()->color(KDecoration::ColorFont));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
}
