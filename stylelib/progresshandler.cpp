#include <QProgressBar>
#include <QRect>
#include <QEvent>
#include <QTimerEvent>
#include <QMap>
#include <QTimer>

#include "progresshandler.h"

using namespace DSP;

Q_DECL_EXPORT ProgressHandler *ProgressHandler::s_instance;

ProgressHandler::ProgressHandler(QObject *parent)
    : QObject(parent)
{
}

ProgressHandler
*ProgressHandler::instance()
{
    if (!s_instance)
        s_instance = new ProgressHandler();
    return s_instance;
}

void
ProgressHandler::manage(QProgressBar *bar)
{
    if (!bar)
        return;

    instance()->checkBar(bar);
}

void
ProgressHandler::release(QProgressBar *bar)
{
    bar->disconnect(instance());
    bar->removeEventFilter(instance());
}

void
ProgressHandler::checkBar(QProgressBar *bar)
{
    if (m_bars.contains(bar))
        return;
    m_bars << bar;
    bar->setAttribute(Qt::WA_Hover, false);
    bar->installEventFilter(this);
    connect(bar, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
    if (bar->isVisible())
        initBar(bar);
}

void
ProgressHandler::initBar(QProgressBar *bar)
{
    if (bar->minimum() == 0 && bar->maximum() == 0)
    {
        TimerData *d = m_data.value(bar, 0);
        if (!d)
        {
            d = new TimerData();
            m_data.insert(bar, d);
            d->timerId = bar->startTimer(25);
        }
    }
    else if (TimerData *d = m_data.value(bar, 0))
    {
        bar->killTimer(d->timerId);
        delete m_data.take(bar);
    }

}

void
ProgressHandler::valueChanged()
{
    initBar(static_cast<QProgressBar *>(sender()));
}

bool
ProgressHandler::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e)
        return false;
    if (!(e->type() == QEvent::Timer
          || e->type() == QEvent::Show
          || e->type() == QEvent::HoverEnter
          || e->type() == QEvent::HoverLeave)
          || !qobject_cast<QProgressBar *>(o))
        return false;

    QProgressBar *bar = static_cast<QProgressBar *>(o);
    if (e->type() == QEvent::Show)
    {
        initBar(bar);
        return false;
    }

    QTimerEvent *te = static_cast<QTimerEvent *>(e);
    TimerData *d(0);
    if (m_data.contains(bar))
        d = m_data.value(bar);
    if (!d || te->timerId() != d->timerId)
        return false;
    bar->update();
    return true;
}
