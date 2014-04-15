#include <QPainter>
#include <QMouseEvent>

#include "button.h"

Button::Button(Type type, KwinClient *client, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_hasPress(false)
    , m_client(client)
{
    setFixedSize(16, 16);
}

Button::~Button()
{

}

void
Button::paintEvent(QPaintEvent *)
{
    QColor c;
    switch (m_type)
    {
    case Close: c = Qt::red; break;
    case Min: c = Qt::yellow; break;
    case Max: c = Qt::green; break;
    default: c = Qt::black; break;
    }
    QPainter p(this);
    p.fillRect(rect(), c);
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
