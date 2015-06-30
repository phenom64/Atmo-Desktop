#include "animhandler.h"
#include "ops.h"
#include <QWidget>
#include <QEvent>
#include <QDebug>
#include <QStyle>
#include <QSlider>
#include <QStyleOptionSlider>
#include <QScrollBar>
#include <QToolButton>
#include <QToolBar>
#include <QHoverEvent>
#include <QAbstractScrollArea>
#include <QAbstractItemView>

using namespace Anim;

static bool s_block(false);

Basic Basic::s_instance;

Basic
*Basic::instance()
{
    return &s_instance;
}

void
Basic::manage(QWidget *w)
{
    if (!w)
        return;
    w->removeEventFilter(instance());
    w->installEventFilter(instance());
    w->setAttribute(Qt::WA_Hover);
    if (qobject_cast<QSlider *>(w) || qobject_cast<QScrollBar *>(w))
        w->setAttribute(Qt::WA_MouseTracking);
    w->disconnect(instance());
    connect(w, SIGNAL(destroyed()), instance(), SLOT(removeSender()));
}

void
Basic::release(QWidget *w)
{
    if (!w)
        return;
    w->removeEventFilter(instance());
    w->disconnect(instance());
    instance()->remove(w);
}

Basic::Basic(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animate()));
}

void
Basic::remove(QWidget *w)
{
    m_vals.remove(w);
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
            if (QAbstractSlider *slider = qobject_cast<QAbstractSlider *>(w))
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
                QStyle::ComplexControl cc(qobject_cast<QSlider *>(w)?QStyle::CC_Slider:QStyle::CC_ScrollBar);
                QStyle::SubControl sc(qobject_cast<QSlider *>(w)?QStyle::SC_SliderHandle:QStyle::SC_ScrollBarSlider);
                QRect r = slider->style()->subControlRect(cc, &sopt, sc, slider);
                mouse = r.contains(slider->mapFromGlobal(QCursor::pos()));
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
    s_block = false;
}

bool
Basic::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;
    bool want(false);
    want |= e->type() == QEvent::Enter;
    want |= e->type() == QEvent::HoverEnter;
    want |= e->type() == QEvent::HoverLeave;
    want |= e->type() == QEvent::Leave;
    if (want)
    {
        if (e->type() == QEvent::Enter || e->type() == QEvent::HoverEnter)
            m_vals.insert(static_cast<QWidget *>(o), 0);
        m_timer->start(25);
        return false;
    }
    if (qobject_cast<QAbstractSlider *>(o))
        if (e->type() == QEvent::HoverMove && !(s_block && m_timer->isActive()))
        {
            if (!m_vals.contains(static_cast<QWidget *>(o)))
                m_vals.insert(static_cast<QWidget *>(o), 0);
            m_timer->start(25);
            s_block = true;
            return false;
        }
    return false;
}


//------------------------------------------------------------------------------------

Tabs Tabs::s_instance;

Tabs
*Tabs::instance()
{
    return &s_instance;
}

void
Tabs::manage(QTabBar *tb)
{
    if (!tb)
        return;
    tb->removeEventFilter(instance());
    tb->installEventFilter(instance());
    tb->setAttribute(Qt::WA_Hover);
    tb->setAttribute(Qt::WA_MouseTracking);
    tb->disconnect(instance());
    connect(tb, SIGNAL(destroyed()), instance(), SLOT(removeSender()));
}

void
Tabs::release(QTabBar *tb)
{
    if (!tb)
        return;
    tb->removeEventFilter(instance());
    tb->disconnect(instance());
    instance()->remove(tb);
}

Tabs::Tabs(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animate()));
}

void
Tabs::remove(QTabBar *tb)
{
    m_vals.remove(tb);
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
    if (m_vals.contains(const_cast<QTabBar *>(tb)))
        return m_vals.value(const_cast<QTabBar *>(tb)).value(tab);
    return 0;
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
    if (!o || !e)
        return false;
    if (e->type() == QEvent::Leave)
    {
        m_timer->start(25);
        return false;
    }
    if (!o->isWidgetType())
        return false;
    QTabBar *tb = static_cast<QTabBar *>(o);
    if (e->type() == QEvent::HoverMove && !(s_block && m_timer->isActive()))
    {
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
    else if (e->type() == QEvent::Resize)
        tb->window()->update();
    return false;
}


//------------------------------------------------------------------------------------

ToolBtns ToolBtns::s_instance;

ToolBtns
*ToolBtns::instance()
{
    return &s_instance;
}

void
ToolBtns::manage(QToolButton *tb)
{
    if (!tb)
        return;
    tb->removeEventFilter(instance());
    tb->installEventFilter(instance());
    tb->setAttribute(Qt::WA_Hover);
    if (Ops::hasMenu(tb))
        tb->setAttribute(Qt::WA_MouseTracking);
    if (QToolBar *bar = qobject_cast<QToolBar *>(tb->parentWidget()))
    {
        bar->setMouseTracking(true);
        bar->removeEventFilter(instance());
        bar->installEventFilter(instance());
    }
    tb->disconnect(instance());
    connect(tb, SIGNAL(destroyed()), instance(), SLOT(removeSender()));
}

void
ToolBtns::release(QToolButton *tb)
{
    if (!tb)
        return;
    tb->removeEventFilter(instance());
    tb->disconnect(instance());
    instance()->remove(tb);
}

ToolBtns::ToolBtns(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animate()));
}

void
ToolBtns::remove(QToolButton *tb)
{
    m_vals.remove(tb);
    if (tb == m_hovered)
        m_hovered = 0;
}

void
ToolBtns::removeSender()
{
    //superfluos?
    static_cast<QToolButton *>(sender())->removeEventFilter(this);
    m_vals.remove(static_cast<QToolButton *>(sender()));
    if (sender() == m_hovered)
        m_hovered = 0;
}

int
ToolBtns::hoverLevel(const QToolButton *tb, bool arrow)
{
    if (m_vals.contains(const_cast<QToolButton *>(tb)))
    {
        QPair<Level, ArrowLevel> vals(m_vals.value(const_cast<QToolButton *>(tb)));
        if (arrow)
            return vals.second;
        else
            return vals.first;
    }
    return 0;
}

void
ToolBtns::animate()
{
    bool needRunning(false);
    QMapIterator<QToolButton *, QPair<Level, ArrowLevel> > it(m_vals);
    while (it.hasNext())
    {
        QToolButton *tb(it.next().key());
        if (!tb)
        {
            m_vals.remove(tb);
            continue;
        }
        QPair<Level, ArrowLevel> levels(it.value());
        bool arrowMouse(false);
        bool btnMouse(false);
        if (Ops::hasMenu(tb))
        {
            QStyleOptionToolButton option;
            option.initFrom(tb);
            option.features = QStyleOptionToolButton::None;
            if (tb->popupMode() == QToolButton::MenuButtonPopup) {
                option.subControls |= QStyle::SC_ToolButtonMenu;
                option.features |= QStyleOptionToolButton::MenuButtonPopup;
            }
            if (tb->arrowType() != Qt::NoArrow)
                option.features |= QStyleOptionToolButton::Arrow;
            if (tb->popupMode() == QToolButton::DelayedPopup)
                option.features |= QStyleOptionToolButton::PopupDelay;
            if (tb->menu())
                option.features |= QStyleOptionToolButton::HasMenu;
            QRect r = tb->style()->subControlRect(QStyle::CC_ToolButton, &option, QStyle::SC_ToolButtonMenu, tb);
            QPoint pos(tb->mapFromGlobal(QCursor::pos()));
            arrowMouse = bool(Ops::hasMenu(tb, &option) && r.contains(pos));
            btnMouse = bool(tb->underMouse() && tb == m_hovered && !arrowMouse);
        }
        else
        {
            btnMouse = tb->underMouse() && tb == m_hovered;
        }

        Level lvl(levels.first);
        ArrowLevel arLvl(levels.second);

        if (btnMouse && lvl < STEPS)
            lvl+=2;
        if (!btnMouse && lvl > 0)
            --lvl;

        if (arrowMouse && arLvl < STEPS)
            arLvl+=2;
        if (!arrowMouse && arLvl > 0)
            --arLvl;

        QPair<Level, ArrowLevel> newLvls(lvl, arLvl); 
        if (newLvls != levels)
        {
            m_vals.insert(tb, newLvls);
            if (!needRunning)
                needRunning = true;
        }

        if (lvl==0 && arLvl==0)
            m_vals.remove(tb);

        tb->repaint();
    }
    if (!needRunning)
        m_timer->stop();
}

bool
ToolBtns::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;
    if (m_hovered && e->type() == QEvent::MouseMove && qobject_cast<QToolBar *>(o))
    {
        if (!(static_cast<QToolBar *>(o)->childAt(static_cast<QMouseEvent *>(e)->pos())) && m_hovered)
        {
            m_hovered = 0;
            if (!m_timer->isActive())
                m_timer->start(25);
        }
        return false;
    }
    if (e->type() == QEvent::Leave || e->type() == QEvent::HoverLeave)
    {
        m_hovered = 0;
        if (!m_timer->isActive())
            m_timer->start(25);
        return false;
    }
    if (qobject_cast<QToolButton *>(o))
    if (e->type() == QEvent::Enter || e->type() == QEvent::HoverEnter
            || (e->type() == QEvent::HoverMove && !m_timer->isActive() && Ops::hasMenu(static_cast<const QToolButton *>(o))))
    {
        QToolButton *tb = static_cast<QToolButton *>(o);
        if (!tb->isEnabled())
            return false;
        if (tb->underMouse())
            m_hovered = tb;
        if (!m_vals.contains(tb))
            m_vals.insert(tb, QPair<Level, ArrowLevel>(0, 0));
        if (!m_timer->isActive())
            m_timer->start(25);
    }
    return false;
}

//-------------------------------------------------------------------------------------------

ScrollAnimator::ScrollAnimator(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_up(false)
    , m_delta(0)
    , m_step(0)
{
    if (!parent)
    {
        deleteLater();
        return;
    }
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateScrollValue()));
    static_cast<QAbstractScrollArea *>(parent)->viewport()->installEventFilter(this);
}

void
ScrollAnimator::manage(QAbstractScrollArea *area)
{
    if (!area)
        return;
    ScrollAnimator *sa = area->findChild<ScrollAnimator *>();
    if (sa && sa->parent() == area)
        return;

    if (QAbstractItemView *v = qobject_cast<QAbstractItemView *>(area))
        if (v->verticalScrollMode() == QAbstractItemView::ScrollPerItem)
            v->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    new ScrollAnimator(area);
}

void
ScrollAnimator::updateScrollValue()
{
    if (m_delta > 0)
    {
        QScrollBar *bar = static_cast<QAbstractScrollArea *>(parent())->verticalScrollBar();
        if ((m_up && bar->value() == bar->minimum())
               || (!m_up && bar->value() == bar->maximum()))
        {
            m_delta = 0;
            m_timer->stop();
            return;
        }
        bar->setValue(bar->value() + (m_up?-m_delta:m_delta));
        m_delta -= m_step;
    }
    else
    {
        m_delta = 0;
        m_timer->stop();
    }
}

bool
ScrollAnimator::processWheelEvent(QWheelEvent *e)
{
    if (e->orientation() == Qt::Horizontal || e->modifiers())
        return false;
    const bool wasUp(m_up);
    m_up = e->delta() > 0;
    if (m_up != wasUp)
        m_delta = 0;

    m_delta+=qBound(2, static_cast<QAbstractScrollArea *>(parent())->verticalScrollBar()->singleStep(), 10);
    m_step=qMax(1, m_delta/10);
    if (!m_timer->isActive())
        m_timer->start(20);
    return true;
}

bool
ScrollAnimator::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Wheel)
        return processWheelEvent(static_cast<QWheelEvent *>(e));
    return false;
}

