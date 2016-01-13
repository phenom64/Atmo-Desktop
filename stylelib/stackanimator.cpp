#include "stackanimator.h"
#include "color.h"
#include <QStackedLayout>
#include <QDebug>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QPaintEvent>
#include <QApplication>
#include <QElapsedTimer>

using namespace DSP;

StackAnimator::StackAnimator(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_stack(static_cast<QStackedLayout *>(parent))
    , m_step(0)
    , m_widget(0)
    , m_prevIndex(-1)
    , m_isActivated(false)
{
    if (m_stack->parentWidget())
    {
        m_widget = new QWidget(m_stack->parentWidget());
        m_widget->installEventFilter(this);
        m_widget->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_widget->hide();
        m_stack->parentWidget()->installEventFilter(this);
        QTimer::singleShot(250, this, SLOT(activate()));
    }
    else
        deleteLater();
}

StackAnimator::~StackAnimator()
{

}

void
StackAnimator::activate()
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animate()));
    connect(m_stack, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
    connect(m_stack, SIGNAL(widgetRemoved(int)), this, SLOT(widgetRemoved(int)));
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
    m_prevIndex = m_stack->currentIndex();
    m_isActivated = true;
    m_widget->hide();
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
StackAnimator::widgetRemoved(int i)
{
    m_prevPix = m_activePix = m_pix = QPixmap();
}

//static QElapsedTimer timer;

void
StackAnimator::currentChanged(int i)
{
    if (m_timer->isActive())
    {
        m_stack->currentWidget()->hide(); //sometimes this gets called while timer is active (mainly systemsettings does this)
        return;
    }
    if (m_stack->parentWidget()->isHidden() || QCoreApplication::closingDown())
        return;

    m_widget->setAttribute(Qt::WA_UpdatesDisabled, true);
    m_widget->move(m_stack->parentWidget()->mapTo(m_stack->parentWidget(), QPoint()));
    m_widget->resize(m_stack->parentWidget()->size());
    m_widget->show();
    m_widget->raise();
    m_widget->setAttribute(Qt::WA_UpdatesDisabled, false);
    m_pix = QPixmap(m_widget->size());

    if (QWidget *w = m_stack->widget(m_prevIndex))
    {
        m_prevPix = QPixmap(m_widget->size());
        m_prevPix.fill(Qt::transparent);
        QRect geo(m_widget->mapTo(w->window(), QPoint()), m_widget->size());
//        w->window()->render(&m_prevPix, QPoint(), geo, QWidget::DrawWindowBackground);
        w->render(&m_prevPix, w->mapTo(m_stack->parentWidget(), QPoint())/*, QRegion(), QWidget::DrawChildren*/);
        m_pix = m_prevPix;
        m_widget->repaint();
    }
    m_prevIndex = i;
    if (QWidget *w = m_stack->widget(i))
    {
        m_activePix = QPixmap(m_widget->size());
        m_activePix.fill(Qt::transparent);
        QRect geo(m_widget->mapTo(w->window(), QPoint()), m_widget->size());
//        w->window()->render(&m_activePix, QPoint(), geo, QWidget::DrawWindowBackground);
        w->render(&m_activePix, w->mapTo(m_stack->parentWidget(), QPoint())/*, QRegion(), QWidget::DrawChildren*/);
        w->setAttribute(Qt::WA_UpdatesDisabled, true);
        m_widget->setAttribute(Qt::WA_UpdatesDisabled, false);
        m_step = 0;
        animate();
        m_timer->start(20);
    }
}

void
StackAnimator::animate()
{
    if (m_step < Steps && !m_prevPix.isNull())
    {
        ++m_step;
        m_pix = QPixmap(m_widget->size());
        m_pix.fill(Qt::transparent);
        QPixmap tmp(m_prevPix);
        QPainter tmpP(&tmp);
        tmpP.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        tmpP.fillRect(m_pix.rect(), QColor(0, 0, 0, m_step*(255.0f/Steps)));
        tmpP.setCompositionMode(QPainter::CompositionMode_SourceOver);
        tmpP.end();

        QPainter p(&m_pix);
        p.drawPixmap(m_activePix.rect(), m_activePix);
        p.drawPixmap(tmp.rect(), tmp);
        p.end();
        m_widget->repaint();
    }
    else
    {
        m_step = 0;
        m_timer->stop();
        m_widget->setAttribute(Qt::WA_UpdatesDisabled);
        m_widget->hide();
        if (QWidget *w = m_stack->currentWidget())
        {
            w->setAttribute(Qt::WA_UpdatesDisabled, false);
            w->show();
            w->update();
        }
        m_stack->parentWidget()->update();
        const QList<QWidget *> kids = m_stack->parentWidget()->findChildren<QWidget *>();
        for (int i = 0; i < kids.count(); ++i)
            kids.at(i)->update();
    }
}

bool
StackAnimator::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Paint && o == m_widget && !m_pix.isNull())
    {
        QPainter p(m_widget);
        p.drawPixmap(m_widget->rect(), m_pix);
        p.end();
        return true;
    }
//    if (e->type() == QEvent::Resize && o == m_stack->parentWidget())
//        m_widget->resize(m_stack->parentWidget()->size());
//    if (e->type() == QEvent::Move && o == m_stack->parentWidget())
//        m_widget->move(m_stack->parentWidget()->mapTo(m_stack->parentWidget()->window(), QPoint()));
    if (e->type() == QEvent::Close && o == m_stack->parentWidget())
//        disconnect(m_stack, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
        deleteLater();
    return false;
}

