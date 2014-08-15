#include "animhandler.h"
#include <QWidget>
#include <QEvent>

using namespace Anim;

Q_DECL_EXPORT Basic *Basic::s_instance = 0;

Basic
*Basic::instance()
{
    if (!s_instance)
        s_instance = new Basic();
    return s_instance;
}


void
Basic::manage(QWidget *w)
{
    if (!w)
        return;
    w->installEventFilter(instance());
    w->setAttribute(Qt::WA_Hover);
    connect(w, SIGNAL(destroyed()), instance(), SLOT(removeSender()));
}

Basic::Basic(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animate()));
}

void
Basic::removeSender()
{
    //superfluos?
    static_cast<QWidget *>(sender())->removeEventFilter(this);
    m_vals.remove(static_cast<QWidget *>(sender()));
}

int
Basic::hoverLevel(const QWidget *widget)
{
    return m_vals.value(const_cast<QWidget *>(widget), 0);
}

void
Basic::animate()
{
    bool needRunning(false);
    QMapIterator<QWidget *, int> it(m_vals);
    while (it.hasNext())
    {
        QWidget *w(it.next().key());
        if (!w)
        {
            m_vals.remove(w);
            continue;
        }
        const bool mouse(w->underMouse());
        const int val(it.value());
        if (mouse && val < STEPS) //hovering...
        {
            needRunning = true;
            m_vals.insert(w, val+1);
        }
        else if (!mouse && val > 0) //left widget and animate out
        {
            needRunning = true;
            m_vals.insert(w, val-1);
        }
        else if (!mouse && val == 0) //animation done for this widget
            m_vals.remove(w);
        w->update();
    }
    if (!needRunning)
        m_timer->stop();
}

bool
Basic::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return false;
    if (e->type() == QEvent::Enter || e->type() == QEvent::Leave)
    {
        if (e->type() == QEvent::Enter)
            m_vals.insert(static_cast<QWidget *>(o), 0);
        m_timer->start(25);
    }
    return false;
}
