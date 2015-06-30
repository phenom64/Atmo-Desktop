#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/widgets.h"

#include <QSplitterHandle>
#include <QStyle>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QResizeEvent>
#include <QTabBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <qmath.h>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include <QMap>
#include <QStackedWidget>
#include <QMainWindow>

OverlayHandler OverlayHandler::s_instance;
static QList<Overlay *> s_overLays;

void
OverlayHandler::manage(Overlay *o)
{
    o->window()->removeEventFilter(instance());
    o->window()->installEventFilter(instance());
    if (!s_overLays.contains(o))
        s_overLays << o;
    connect(o, SIGNAL(destroyed()), instance(), SLOT(overlayDeleted()));
}

void
OverlayHandler::overlayDeleted()
{
    Overlay *o(static_cast<Overlay *>(sender()));
    if (s_overLays.contains(o))
        s_overLays.removeOne(o);
}

bool
OverlayHandler::eventFilter(QObject *o, QEvent *e)
{
    if (SplitterExt::isActive())
        return false;
    if (o->isWidgetType())
    {
        if (e->type() == QEvent::LayoutRequest
                || e->type() == QEvent::ZOrderChange
                || e->type() == QEvent::HideToParent
                || e->type() == QEvent::ShowToParent)
            for (int i = 0; i < s_overLays.count(); ++i)
                QMetaObject::invokeMethod(s_overLays.at(i), "updateOverlay", Qt::QueuedConnection);
        return false;
    }
    return false;
}

Overlay::Overlay(QWidget *parent, int opacity)
    : QWidget(parent)
    , m_alpha(opacity)
    , m_lines(All)
    , m_frame(parent)
    , m_hasFocus(false)
{
    if (!m_frame->parentWidget())
    {
        deleteLater();
        return;
    }
    OverlayHandler::manage(this);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_frame->installEventFilter(this);  //m_frame guaranteed by manage()
    const QList<QStackedWidget *> stacks(m_frame->window()->findChildren<QStackedWidget *>());
    for (int i = 0; i < stacks.count(); ++i)
        stacks.at(i)->installEventFilter(this);
}

Overlay::~Overlay()
{
    m_frame = 0;
}

void
Overlay::paintEvent(QPaintEvent *)
{
    if (!isVisible())
        return;

    raise();
    QPainter p(this);
#define FOCUS 0
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
Overlay::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;
    if (o == m_frame)
    {
        switch (e->type())
        {
        case QEvent::ZOrderChange:
        case QEvent::LayoutRequest:
        case QEvent::Show:
            setMask(mask());
            QMetaObject::invokeMethod(this, "updateOverlay", Qt::QueuedConnection);
            return false;
        case QEvent::Resize:
            resize(m_frame->size());
            setMask(mask());
            return false;
        default:
            return false;
        }
    }
    return false;
}

static QRect windowGeo(QWidget *widget)
{
    return QRect(widget->mapTo(widget->window(), QPoint(0, 0)), widget->size());
}

void
Overlay::updateOverlay()
{
//    if (!(m_frame->frameShape() == QFrame::StyledPanel && m_frame->frameShadow() == QFrame::Sunken))
//    {
//        hide();
//        deleteLater();
//        return;
//    }
    static QMap<QWidget *, QSize> sm;
    if (sm.value(m_frame, QSize()) != m_frame->size())
    {
        sm.insert(m_frame, m_frame->size());
        return;
    }

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

        int (QWidget::*widthOrHeight)() const = (pos[i]==West||pos[i]==East)?&Overlay::width:&Overlay::height;
        const bool isSplitter((qobject_cast<QSplitterHandle *>(w) || w->objectName() == "qt_qmainwindow_extended_splitter") && w->style()->pixelMetric(QStyle::PM_SplitterWidth) == 1);
        const bool isStatusBar(Ops::isOrInsideA<QStatusBar *>(w) && l[i] != Top);
        const bool isTabBar(qobject_cast<QTabBar *>(w) && static_cast<QTabBar *>(w)->documentMode());
        if ( isStatusBar || isSplitter || isTabBar )
            sides &= ~l[i];
        else if (Overlay *ol = hasOverlay(getFrameForWidget(w, pos[i])))
        {
            if (ol->lines() & wl[i] && (ol->*widthOrHeight)() < (this->*widthOrHeight)())
                sides &= ~l[i];
            else
                sides |= l[i];
        }
    }
    QRect winRect = m_frame->window()->rect();
    winRect.moveTopLeft(m_frame->mapFrom(m_frame->window(), winRect.topLeft()));
    if (r.top() <= winRect.top())
        sides &= ~Top;
    if (r.left() <= winRect.left())
        sides &= ~Left;
    if (r.bottom() >= winRect.bottom())
        sides &= ~Bottom;
    if (r.right() >= winRect.right())
        sides &= ~Right;

    if (m_lines != sides)
    {
        m_lines = sides;
        update();
    }
    raise();
}

bool
Overlay::manage(QWidget *frame, int opacity)
{
    if (!frame || hasOverlay(frame))
        return false;

    if (qobject_cast<QMainWindow *>(frame->window()))
    {
        new Overlay(frame, opacity);
        return true;
    }
    return false;
}

bool
Overlay::release(QWidget *frame)
{
    if (Overlay *o = frame->findChild<Overlay *>())
        if (o->parent() == frame)
        {
            o->deleteLater();
            return true;
        }
    return false;
}

QRegion
Overlay::mask() const
{
    const int &d = m_hasFocus ? 6 : 1;
    const QRegion &outer(rect()), &inner(rect().adjusted(d, d, -d, -d));
    return outer-inner;
}

QRect
Overlay::mappedRect(const QWidget *widget)
{
    QPoint topLeft(widget->mapTo(widget->window(), widget->rect().topLeft()));
    return QRect(topLeft, widget->size());
}

bool
Overlay::frameIsInteresting(const QFrame *frame, const Position pos) const
{
    if (frame && frame->frameShadow() == QFrame::Sunken && frame->frameShape() == QFrame::StyledPanel)
        if (mappedRect(frame).contains(m_position[pos]))
            return true;
    return false;
}

QFrame
*Overlay::getFrameForWidget(QWidget *w, const Position pos) const
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
Overlay::parentChanged()
{
//    if (m_frame->parentWidget())
//        m_frame->window() = m_frame->window();
//    else
//        m_frame->window() = 0;
}

Overlay
*Overlay::hasOverlay(const QWidget *frame)
{
    if (!frame)
        return 0;
    if (Overlay *o = frame->findChild<Overlay *>())
        if (o->parentWidget() == frame)
            return o;
    return 0;
}
