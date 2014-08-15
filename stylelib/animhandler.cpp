#include "animhandler.h"
#include "../styleproject.h"
#include <QWidget>
#include <QEvent>
#include <QDebug>
#include <QStyle>
#include <QSlider>
#include <QStyleOptionSlider>
#include <QScrollBar>

using namespace Anim;

static QStyle *s_style = 0;
static bool s_block(false);

Q_DECL_EXPORT Basic *Basic::s_instance = 0;

void Anim::setStyle(QStyle *style)
{
    s_style = style;
}

Basic
*Basic::instance()
{
    if (!s_instance)
        s_instance = new Basic(s_style);
    return s_instance;
}


void
Basic::manage(QWidget *w)
{
    if (!w)
        return;
    w->installEventFilter(instance());
    w->setAttribute(Qt::WA_Hover);
    if (qobject_cast<QSlider *>(w) || qobject_cast<QScrollBar *>(w))
        w->setAttribute(Qt::WA_MouseTracking);
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
        bool mouse(w->underMouse());
        if (mouse)
        {
            if (castObj(QSlider *, slider, w))
            {
                QStyleOptionSlider sopt;
                sopt.initFrom(slider);
                sopt.sliderPosition = slider->sliderPosition();
                sopt.sliderValue = slider->value();
                sopt.minimum = slider->minimum();
                sopt.maximum = slider->maximum();
                sopt.orientation = slider->orientation();
                sopt.pageStep = slider->pageStep();
                sopt.singleStep = slider->singleStep();
                sopt.upsideDown = slider->invertedAppearance();
                QRect r = s_style->subControlRect(QStyle::CC_Slider, &sopt, QStyle::SC_SliderHandle, slider);
                mouse = r.contains(slider->mapFromGlobal(QCursor::pos()));
            }
            else if (castObj(QScrollBar *, scroller, w))
            {
                QStyleOptionSlider sopt;
                sopt.initFrom(scroller);
                sopt.sliderPosition = scroller->sliderPosition();
                sopt.sliderValue = scroller->value();
                sopt.minimum = scroller->minimum();
                sopt.maximum = scroller->maximum();
                sopt.orientation = scroller->orientation();
                sopt.pageStep = scroller->pageStep();
                sopt.singleStep = scroller->singleStep();
                sopt.upsideDown = scroller->invertedAppearance();
                QRect r = s_style->subControlRect(QStyle::CC_ScrollBar, &sopt, QStyle::SC_ScrollBarSlider, scroller);
                mouse = r.contains(scroller->mapFromGlobal(QCursor::pos()));
            }
        }
        const int val(it.value());
        if (mouse && val < STEPS) //hovering...
        {
            needRunning = true;
            m_vals.insert(w, val+2);
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
//    s_block = false;
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
        return false;
    }
    if (qobject_cast<QSlider *>(o) || qobject_cast<QScrollBar *>(o))
        if (e->type() == QEvent::HoverMove /*&& !(s_block && m_timer->isActive())*/)
        {
            if (!m_vals.contains(static_cast<QWidget *>(o)))
                m_vals.insert(static_cast<QWidget *>(o), 0);
            m_timer->start(25);
//            s_block = true;
            return false;
        }
    return false;
}


//------------------------------------------------------------------------------------

Q_DECL_EXPORT Tabs *Tabs::s_instance = 0;

Tabs
*Tabs::instance()
{
    if (!s_instance)
        s_instance = new Tabs(s_style);
    return s_instance;
}


void
Tabs::manage(QTabBar *tb)
{
    if (!tb)
        return;
    tb->installEventFilter(instance());
    tb->setAttribute(Qt::WA_Hover);
    tb->setAttribute(Qt::WA_MouseTracking);
    connect(tb, SIGNAL(destroyed()), instance(), SLOT(removeSender()));
}

Tabs::Tabs(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animate()));
}

void
Tabs::removeSender()
{
    //superfluos?
    static_cast<QTabBar *>(sender())->removeEventFilter(this);
    m_vals.remove(static_cast<QTabBar *>(sender()));
}

int
Tabs::hoverLevel(const QTabBar *tb, Tab tab)
{
    QMap<Tab, Level> levels(m_vals.value(const_cast<QTabBar *>(tb)));
    return levels.value(tab);
}

void
Tabs::animate()
{
    bool needRunning(false);
    QMapIterator<QTabBar *, QMap<Tab, Level> > it(m_vals);
    while (it.hasNext())
    {
        QTabBar *tb(it.next().key());
        if (!tb)
        {
            m_vals.remove(tb);
            continue;
        }
        QMap<Tab, Level> levels(it.value());
        for (int tab = 0; tab < tb->count(); ++tab)
        {
            if (!levels.contains(tab))
                continue;
            const bool mouse(tb->tabRect(tab).contains(tb->mapFromGlobal(QCursor::pos())));
            const int val(levels.value(tab));
            if (mouse && val < STEPS)
            {
                needRunning = true;
                levels.insert(tab, val+2);
            }
            else if (!mouse && val > 0)
            {
                needRunning = true;
                levels.insert(tab, val-1);
            }
            else if (!mouse && val == 0)
                levels.remove(tab);
        }
        m_vals.insert(tb, levels);
        tb->update();
    }
    if (!needRunning)
        m_timer->stop();
    s_block = false;
}

bool
Tabs::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Leave)
    {
        m_timer->start(25);
        return false;
    }
    if (!o->isWidgetType())
        return false;
    if (e->type() == QEvent::HoverMove && !(s_block && m_timer->isActive()))
    {
        QTabBar *tb = static_cast<QTabBar *>(o);
        Tab tab(tb->tabAt(tb->mapFromGlobal(QCursor::pos())));
        if (tab > -1)
        {
            QMap<Tab, Level> levels;
            if (m_vals.contains(tb))
                levels = m_vals.value(tb);
            if (!levels.contains(tab))
                levels.insert(tab, 0);
            m_vals.insert(tb, levels);
            s_block = true;
            m_timer->start(25);
        }
    }
    return false;
}
