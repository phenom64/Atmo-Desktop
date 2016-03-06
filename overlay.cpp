#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/widgets.h"
#include "config/settings.h"

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
#include <QLayout>
#include <QDialog>
#include <QDockWidget>
#include <QStyleFactory>


using namespace DSP;

OverlayHandler *OverlayHandler::s_instance = 0;
static QList<Overlay *> s_overLays;

OverlayHandler
*OverlayHandler::instance()
{
    if (!s_instance)
        s_instance = new OverlayHandler();
    return s_instance;
}

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
        if (e->type() == QEvent::LayoutRequest)
            for (int i = 0; i < s_overLays.count(); ++i)
                QMetaObject::invokeMethod(s_overLays.at(i), "updateOverlay", Qt::QueuedConnection);
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------

static QList<const QWidget *> s_unsupported;

bool
Overlay::isSupported(const QWidget *frame)
{
    if (!frame || dConf.app == Settings::Eiskalt || s_unsupported.contains(frame))
        return false;
    if (frame->inherits("KTextEditor::ViewPrivate"))
        return true;
    const QFrame *f = qobject_cast<const QFrame *>(frame);
    if (!f)
        return false;
    if (f->frameShadow() != QFrame::Sunken || f->frameShape() != QFrame::StyledPanel)
        return false;
    if (f->inherits("KMultiTabBarInternal"))
        return true;

    QWidget *p = f->parentWidget();
    if (!p)
        return false;

    QLayout *l = p->layout();
    static const QMargins m(0, 0, 0, 0);
    const bool dock = qobject_cast<QDockWidget *>(p);
    if (dock || ((l && l->spacing() == 0 && l->contentsMargins() == m) || (!l && p->contentsMargins() == m) || f->size() == p->size()))
    {
//        qDebug() << p << f;
        return true;
    }
    return false;
}

bool
Overlay::manage(QWidget *frame, int opacity)
{
    if (!frame || overlay(frame))
        return false;
//    QMetaObject::invokeMethod(OverlayHandler::instance(), "manageOverlay", Qt::QueuedConnection, Q_ARG(QWidget*, frame));
    if (isSupported(frame))
        return new Overlay(frame, opacity);
    return false;
}

bool
Overlay::release(QWidget *frame)
{
    if (Overlay *o = overlay(frame))
    {
        o->deleteLater();
        return true;
    }
    return false;
}

Overlay::Overlay(QWidget *parent, int opacity)
    : QWidget(parent)
    , m_alpha(opacity)
    , m_sides(All)
    , m_frame(parent)
    , m_hasFocus(false)
    , m_shown(false)
{
    if (!m_frame->parentWidget())
    {
        deleteLater();
        return;
    }
    OverlayHandler::manage(this);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_frame->installEventFilter(this);  //m_frame guaranteed by manage()
    raise();
}

Overlay::~Overlay()
{
    new Restorer((qulonglong)m_frame);
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
    if (sides() & Top)
    {
        p.drawLine(r.topLeft(), r.topRight());
        r.setTop(r.top()+1);
    }
    if (sides() & Right)
    {
        p.drawLine(r.topRight(), r.bottomRight());
        r.setRight(r.right()-1);
    }
    if (sides() & Bottom)
    {
        p.drawLine(r.bottomLeft(), r.bottomRight());
        r.setBottom(r.bottom()-1);
    }
    if (sides() & Left)
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
        //        case QEvent::LayoutRequest:
        case QEvent::Show:
        {
            if (!(qobject_cast<QMainWindow *>(m_frame->window())||qobject_cast<QDockWidget *>(m_frame->window())))
            {
                s_unsupported << m_frame;
                hide();
                QMetaObject::invokeMethod(this, "updateOverlay", Qt::QueuedConnection);
                return false;
            }
            if (!m_shown)
            {
                //this forces re-reading of pixelmetric apparently...
                QEvent e(QEvent::StyleChange);
                QApplication::sendEvent(m_frame, &e);
                m_shown = true;
            }
            setMask(mask());
            raise();
            QTimer::singleShot(250, this, SLOT(updateOverlay()));
            return false;
        }
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

static QRect windowGeo(const QWidget *widget)
{
    return QRect(widget->mapTo(widget->window(), QPoint(0, 0)), widget->size());
}

static const QString sideString[4] = { "Left", "Top", "Right", "Bottom" };
static const QString reversedSideString[4] = { "Right", "Bottom", "Left", "Top" };
static const Position position[4] = { West, North, East, South };

void
Overlay::removeSide(const Side s)
{
//    static const Position pos[4] = { West, North, East, South };
//    qDebug() << "removing side" << s << "from" << m_frame;
    sides() &= ~s;
    update();
}

void
Overlay::addSide(const Side s)
{
//    static const Position pos[4] = { West, North, East, South };
//    qDebug() << "adding side" << sideString[pos[s]] << "to" << m_frame;
    sides() |= s;
    update();
}

void
Overlay::updateOverlay()
{
    if (isHidden())
        deleteLater();
    if (!m_frame->isVisible())
        return;
    static QMap<QWidget *, QSize> sm;
    if (sm.value(m_frame, QSize()) != m_frame->size())
    {
        sm.insert(m_frame, m_frame->size());
        return;
    }
    const QRect r(m_frame->rect());
    const QRect frameGeo = windowGeo(m_frame);
    if (frameGeo.x() < 0 || frameGeo.y() < 0)
    {
        QTimer::singleShot(500, this, SLOT(updateOverlay()));
        return;
    }

    static const Side l[4] = { Left, Top, Right, Bottom };
    static const Side wl[4] = { Right, Bottom, Left, Top }; //reversed/adjacent
    static const int d(1);
#define GP(_X_, _Y_) m_frame->mapTo(m_frame->window(), QPoint(_X_, _Y_))
    m_position[West] = GP(r.x()-d, r.center().y());
    m_position[North] = GP(r.center().x(), r.y()-d);
    m_position[East] = GP(r.right()+1+d, r.center().y());
    m_position[South] = GP(r.center().x(), r.bottom()+1+d);
#undef GP

    for (int i = 0; i < PosCount; ++i)
    {
        QWidget *w = m_frame->window()->childAt(m_position[i]);
        if (!w || w->isAncestorOf(m_frame))
            continue;

        const bool splitter = isSplitter(w, (Position)i);
        const bool statusBar(Ops::isOrInsideA<QStatusBar *>(w) && l[i] != Top);
        const bool tabBar(qobject_cast<QTabBar *>(w) && static_cast<QTabBar *>(w)->documentMode() && l[i] == Top);
        if ( statusBar || splitter || tabBar )
        {
            removeSide(l[i]);
        }
        else if (Overlay *o = overlay(w, true))
        {
            int (QWidget::*widthOrHeight)() const = (position[i]==West||position[i]==East) ? &Overlay::width : &Overlay::height;
            if ((o->sides() & wl[i]) && (o->*widthOrHeight)() >= (this->*widthOrHeight)())
            {
                removeSide(l[i]);
            }
            else
            {
                o->removeSide(wl[i]);
                addSide(l[i]);
            }
        }
        else
        {
            addSide(l[i]);
        }
    }
    QRect winRect = m_frame->window()->rect();
    if (frameGeo.top() <= winRect.top())
        removeSide(Top);
    if (frameGeo.left() <= winRect.left())
        removeSide(Left);
    if (frameGeo.bottom() >= winRect.bottom())
        removeSide(Bottom);
    if (frameGeo.right() >= winRect.right())
        removeSide(Right);

    raise();
    update();
}

QRegion
Overlay::mask() const
{
    const int d = m_hasFocus ? 6 : 1;
    const QRegion outer(rect()), inner(rect().adjusted(d, d, -d, -d));
    return outer-inner;
}

Overlay
*Overlay::overlay(const QWidget *frame, const bool recursive)
{
    if (!frame)
        return 0;
    if (const QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(frame->parentWidget()))
        return overlay(area, recursive);
    if (Overlay *o = frame->findChild<Overlay *>())
    {
        if (!recursive)
        {
            if (o->parentWidget() == frame)
                return o;
            return 0;
        }
        if (o->size() == frame->size())
            return o;
    }
    return 0;
}

bool
Overlay::isSplitter(QWidget *w, const Position p)
{
    if (!(qobject_cast<QSplitterHandle *>(w) || (w->objectName() == "qt_qmainwindow_extended_splitter")))
        return false;

    QRect geo = windowGeo(w);
    if (w->height() == 5)
    {
        geo.setY(geo.y()+2+(position[p]==South));
        geo.setHeight(1);
        return geo.contains(m_position[p]);
    }
    else if (w->width() == 5)
    {
        geo.setX(geo.x()+2+(position[p]==East));
        geo.setWidth(1);
        return geo.contains(m_position[p]);
    }
    else
        return false;
}

//------------------------------------------------------------------------------------------------------------

Restorer::Restorer(qulonglong widget)
    : QTimer()
    , m_widget(widget)
{
    if (QWidget *w = Ops::getChild<QWidget *>(m_widget))
        setParent(w);
    setSingleShot(true);
    connect(this, SIGNAL(timeout()), this, SLOT(restore()));
    start(0);
}

void
Restorer::restore()
{
    deleteLater();
    QWidget *w = Ops::getChild<QWidget *>(m_widget);
    if (!w)
        return;

#if 0
    static QStyle *first = QStyleFactory::create(QStyleFactory::keys().first());
    w->setStyle(first);
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
    w->updateGeometry();
    w->setStyle(w->style());
#endif
    //apparently this makes qt reread the pixelmetric and we get a normal frame
    //this is needed as the overlay'ed widgets have a pixelmetric framewidth of 0
    QEvent e(QEvent::StyleChange);
    QApplication::sendEvent(w, &e);
}
