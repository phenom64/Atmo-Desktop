#include "stackanimator.h"
#include "color.h"
#include <QStackedLayout>
#include <QDebug>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QPaintEvent>


static QMap<QStackedLayout *, QPixmap> s_pix;

StackAnimator::StackAnimator(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_stack(static_cast<QStackedLayout *>(parent))
    , m_step(0)
    , m_widget(0)
    , m_prevIndex(-1)
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animate()));
    connect(m_stack, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
    if (m_stack->parentWidget())
    {
        m_widget = new QWidget(m_stack->parentWidget());
        m_widget->installEventFilter(this);
        m_widget->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_widget->hide();
        m_stack->parentWidget()->installEventFilter(this);
    }
    else
        deleteLater();
    m_prevIndex = m_stack->currentIndex();
}

void
StackAnimator::manage(QStackedLayout *l)
{
    if (StackAnimator *sa = l->findChild<StackAnimator *>())
        if (sa->parent() == l)
            return;
    new StackAnimator(l);
}

void
StackAnimator::currentChanged(int i)
{
    m_widget->setUpdatesEnabled(false);
    m_widget->show();
    m_widget->raise();
    if (QWidget *w = m_stack->widget(m_prevIndex))
    {
        m_pix = QPixmap(m_widget->size());
        m_pix.fill(Qt::transparent);
        w->render(&m_pix);
    }
    m_prevIndex = m_stack->currentIndex();
    if (QWidget *w = m_stack->currentWidget())
    {
        m_activePix = QPixmap(m_widget->size());
        m_activePix.fill(Qt::transparent);
        w->render(&m_activePix);
        w->setUpdatesEnabled(false);
        m_step = 0;
        m_timer->start(20);
    }
}

void
StackAnimator::animate()
{
    if (m_step < Steps)
    {
        ++m_step;
        m_widget->setUpdatesEnabled(true);
        m_widget->repaint();
    }
    else
    {
        m_step = 0;
        m_timer->stop();
        m_widget->hide();
        if (QWidget *w = m_stack->currentWidget())
            w->setUpdatesEnabled(true);
    }

}

bool
StackAnimator::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Paint && o == m_widget && m_step)
    {
        QPainter pixP(&m_pix);
        pixP.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pixP.fillRect(m_widget->rect(), QColor(0, 0, 0, m_step*(255.0f/Steps)));
        pixP.end();
        QPainter p(m_widget);
        p.drawPixmap(m_widget->rect(), m_activePix);
        p.drawPixmap(m_widget->rect(), m_pix);
        p.end();
        return true;
    }
    if (e->type() == QEvent::Resize && o == m_stack->parentWidget())
        m_widget->resize(m_stack->parentWidget()->size());
    return false;
}

