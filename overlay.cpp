#include "overlay.h"
#include "stylelib/ops.h"

OverLay::OverLay(QFrame *parent, int opacity)
    : QWidget(parent)
    , m_alpha(opacity)
    , m_lines(All)
    , m_frame(parent)
    , m_timer(new QTimer(this))
    , m_hasFocus(false)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_frame->installEventFilter(this);  //m_frame guaranteed by manage()
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateOverlay()));
    m_timer->setInterval(50);
    foreach (QStackedWidget *stack, m_frame->window()->findChildren<QStackedWidget *>())
        stack->installEventFilter(this);
}

void
OverLay::paintEvent(QPaintEvent *)
{
    if (!isVisible())
        return;

    raise();
    QPainter p(this);

#define FOCUS 1
#if FOCUS
    if (m_hasFocus)
    {
        p.setBrush(Qt::NoBrush);
        p.translate(0.5f, 0.5f);
        p.setRenderHint(QPainter::Antialiasing);
        QColor h(m_frame->palette().color(QPalette::Highlight));
        QRect ret(rect().adjusted(0, 0, -1, -1));
        while (h.alpha())
        {
            p.setPen(h);
            p.drawRect(ret);
            ret.adjust(1, 1, -1, -1);
            h.setAlpha(qFloor(float(h.alpha()/1.8f)));
        }
        p.end();
        return;
    }

#endif
#undef FOCUS
    p.setPen(QPen(QColor(0, 0, 0, m_alpha), 1));
    p.setBrush(Qt::NoBrush);
    QRect r(rect());

    if (m_lines & Top)
    {
        p.drawLine(r.topLeft(), r.topRight());
        r.setTop(r.top()+1);
    }
    if (m_lines & Right)
    {
        p.drawLine(r.topRight(), r.bottomRight());
        r.setRight(r.right()-1);
    }
    if (m_lines & Bottom)
    {
        p.drawLine(r.bottomLeft(), r.bottomRight());
        r.setBottom(r.bottom()-1);
    }
    if (m_lines & Left)
        p.drawLine(r.topLeft(), r.bottomLeft());
    p.end();
}

bool
OverLay::eventFilter(QObject *o, QEvent *e)
{
    if (o != m_frame && e->type() == QEvent::ChildRemoved)
    {
        m_timer->start();
        return false;
    }
    if (o == m_frame)
        switch (e->type())
        {
        case QEvent::ZOrderChange:
        case QEvent::Show:
            show();
            resize(m_frame->size());
            repaint();
            setMask(mask());
            m_timer->start();
            return false;
        case QEvent::Resize:
            resize(m_frame->size());
            repaint();
            setMask(mask());
            m_timer->start(300);
            return false;
        case QEvent::FocusIn:
            m_hasFocus = true;
            repaint();
            setMask(mask());
            updateOverlay();
            m_timer->start();
            return false;
        case QEvent::FocusOut:
            m_hasFocus = false;
            repaint();
            setMask(mask());
            updateOverlay();
            m_timer->start();
            return false;
        case QEvent::Paint:
            raise();
            return false;
        case QEvent::Hide:
            hide();
            return false;
        case QEvent::ParentChange:
            parentChanged();
            return false;
        default:
            return false;
        }
    return false;
}

void
OverLay::updateOverlay()
{
    static QMap<QFrame *, QSize> sm;
    if (sm.value(m_frame, QSize()) != m_frame->size())
    {
        sm.insert(m_frame, m_frame->size());
        return;
    }

    m_timer->stop();
    const QRect &r(m_frame->rect());
    const int d(1);

#define GP(_X_, _Y_) m_frame->mapTo(m_frame->window(), QPoint(_X_, _Y_))
    m_position[West] = GP(r.x()-d, r.center().y());
    m_position[North] = GP(r.center().x(), r.y()-d);
    m_position[East] = GP(r.right()+1+d, r.center().y());
    m_position[South] = GP(r.center().x(), r.bottom()+1+d);
#undef GP

    static Position pos[4] = { West, North, East, South };
    static Side l[4] = { Left, Top, Right, Bottom };
    static Side wl[4] = { Right, Bottom, Left, Top }; //reversed
    //debugging purposes
//    static QString s[4] = { "Left", "Top", "Right", "Bottom" };
//    static QString ws[4] = { "Right", "Bottom", "Left", "Top" };

    Sides sides(All);

    for (int i = 0; i < PosCount; ++i)
    {
        QWidget *w = m_frame->window()->childAt(m_position[i]);
        if (!w || w->isAncestorOf(m_frame))
            continue;

        if (w->objectName() == "qt_qmainwindow_extended_splitter"
                || Ops::isOrInsideA<QStatusBar *>(w)
                || (l[i] == Top && qobject_cast<QTabBar *>(w))
                || qobject_cast<QSplitterHandle *>(w)
                )
        {
            sides &= ~l[i];
        }
        else if (QFrame *f = getFrameForWidget(w, pos[i]))
        {
            if (OverLay *lay = f->findChild<OverLay*>())
//                if (lay->size() == f->size())
                    if (lay->lines() & wl[i])
                        sides &= ~l[i];
                    else
                        sides |= l[i];
        }
    }
    QRect winRect = m_frame->window()->rect();
    winRect.moveTopLeft(m_frame->mapFrom(m_frame->window(), winRect.topLeft()));
    if(r.left() <= winRect.left())
        sides &= ~Left;
    if(r.bottom() >= winRect.bottom())
        sides &= ~Bottom;
    if(r.right() >= winRect.right())
        sides &= ~Right;

    if (m_lines != sides)
    {
        m_lines = sides;
        update();
    }
    raise();
}

bool
OverLay::manage(QFrame *frame, int opacity)
{
    if (!frame)
        return false;
    if (frame->findChild<OverLay*>())
        return false;
    if (frame->frameShadow() == QFrame::Sunken && frame->frameShape() == QFrame::StyledPanel)
    {
        new OverLay(frame, opacity);
        return true;
    }
    return false;
}

QRegion
OverLay::mask() const
{
    const int &d = m_hasFocus ? 6 : 1;
    const QRegion &outer(rect()), &inner(rect().adjusted(d, d, -d, -d));
    return outer-inner;
}

QRect
OverLay::mappedRect(const QWidget *widget)
{
    QPoint topLeft(widget->mapTo(widget->window(), widget->rect().topLeft()));
    QPoint bottomRight(widget->mapTo(widget->window(), widget->rect().bottomRight()));
    return QRect(topLeft, bottomRight);
}

bool
OverLay::frameIsInteresting(const QFrame *frame, const Position &pos) const
{
    if (frame && frame->frameShadow() == QFrame::Sunken && frame->frameShape() == QFrame::StyledPanel)
        if (mappedRect(frame).contains(m_position[pos]))
            return true;
    return false;
}

QFrame
*OverLay::getFrameForWidget(QWidget *w, const Position &pos) const
{
    QFrame *frame = 0;
    while (w->parentWidget())
    {
        frame = qobject_cast<QFrame *>(w);
        if (frameIsInteresting(frame, pos))
            return frame;
        QList<QFrame *> frames = w->findChildren<QFrame *>();
        for (int i = 0; i < frames.count(); ++i)
        {
            frame = frames.at(i);
            if (frameIsInteresting(frame, pos))
                return frame;
        }
        w = w->parentWidget();
    }
    return 0;
}

void
OverLay::parentChanged()
{
//    if (m_frame->parentWidget())
//        m_frame->window() = m_frame->window();
//    else
//        m_frame->window() = 0;
}
