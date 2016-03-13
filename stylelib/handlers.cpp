#include "handlers.h"
#include "xhandler.h"
#include "windowdata.h"
#include "macros.h"
#include "color.h"
#include "gfx.h"
#include "ops.h"
#include "../config/settings.h"
#include "shadowhandler.h"
#include "fx.h"
#include "toolbarhelpers.h"
#include "titlewidget.h"

#include <QMainWindow>
#include <QCoreApplication>
#include <QTabBar>
#include <QToolBar>
#include <QEvent>
#include <QLayout>
#include <QStyle>
#include <QTimer>
#include <QPainter>
#include <QMenuBar>
#include <QDebug>
#include <QMouseEvent>
#include <QApplication>
#include <QStatusBar>
#include <QLabel>
#include <QDialog>
#include <QToolButton>
#include <qmath.h>
#include <QAbstractScrollArea>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QElapsedTimer>
#include <QBuffer>
#include <QHeaderView>
#include <QToolTip>
#include <QDesktopWidget>
#include <QGroupBox>
#include <QDockWidget>
#include <QWidgetAction>
#include <QX11Info>
#include "defines.h"
#include "windowhelpers.h"
#if HASDBUS
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include "stylelib/macmenu.h"
#endif

using namespace DSP;

static QRegion paintRegion(QMainWindow *win)
{
    if (!win)
        return QRegion();
    QRegion r(win->rect());
    QList<QToolBar *> children(win->findChildren<QToolBar *>());
    for (int i = 0; i < children.count(); ++i)
    {
        QToolBar *c(children.at(i));
        if (c->parentWidget() != win || win->toolBarArea(c) != Qt::TopToolBarArea
                || c->orientation() != Qt::Horizontal || !c->isVisible())
            continue;
        r -= QRegion(c->geometry());
    }
    if (QStatusBar *bar = win->findChild<QStatusBar *>())
        if (bar->isVisible() && bar->mapTo(win, bar->rect().bottomRight()).y() == win->rect().bottom())
        {
            if (bar->parentWidget() == win)
                r -= QRegion(bar->geometry());
            else
                r -= QRegion(QRect(bar->mapTo(win, bar->rect().topLeft()), bar->size()));
        }
    if (QTabBar *bar = win->findChild<QTabBar *>())
        if (bar->isVisible() && Ops::isSafariTabBar(bar))
        {
            QRect geo;
            if (bar->parentWidget() == win)
                geo = bar->geometry();
            else
                geo = QRect(bar->mapTo(win, bar->rect().topLeft()), bar->size());

            if (QTabWidget *tw = qobject_cast<QTabWidget *>(bar->parentWidget()))
            {
                int right(tw->mapTo(win, tw->rect().topRight()).x());
                int left(tw->mapTo(win, tw->rect().topLeft()).x());
                geo.setRight(right);
                geo.setLeft(left);
            }
            if (qApp->applicationName() == "konsole")
            {
                geo.setLeft(win->rect().left());
                geo.setRight(win->rect().right());
            }
            r -= QRegion(geo);
        }
    return r;
}


//-------------------------------------------------------------------------------------------------

using namespace Handlers;

ToolBar *ToolBar::s_instance;
QMap<QToolButton *, Sides> ToolBar::s_sides;
QMap<QToolBar *, bool> ToolBar::s_dirty;
QMap<QToolBar *, QAction *> ToolBar::s_spacers;

ToolBar
*ToolBar::instance()
{
    if (!s_instance)
        s_instance = new ToolBar();
    return s_instance;
}

void
ToolBar::manage(QWidget *child)
{
//    if (!qobject_cast<QToolBar *>(child->parentWidget()))
//        return;
    child->removeEventFilter(instance());
    child->installEventFilter(instance());
    if (qobject_cast<QToolButton *>(child))
    {
        child->disconnect(instance());
        connect(child, SIGNAL(destroyed(QObject*)), instance(), SLOT(toolBtnDeleted(QObject*)));
    }
}

void
ToolBar::manageToolBar(QToolBar *tb)
{
    if (!tb)
        return;
    tb->removeEventFilter(instance());
    tb->installEventFilter(instance());
    tb->disconnect(instance());
    s_dirty.insert(tb, true);
}

void
ToolBar::toolBarDeleted(QObject *toolBar)
{
    QToolBar *bar(static_cast<QToolBar *>(toolBar));
    if (s_dirty.contains(bar))
        s_dirty.remove(bar);
}

void
ToolBar::toolBtnDeleted(QObject *toolBtn)
{
    QToolButton *btn(static_cast<QToolButton *>(toolBtn));
    if (s_sides.contains(btn))
        s_sides.remove(btn);
}

void
ToolBar::checkForArrowPress(QToolButton *tb, const QPoint pos)
{
    QStyleOptionToolButton option;
    option.initFrom(tb);
    option.features = QStyleOptionToolButton::None;
    if (tb->popupMode() == QToolButton::MenuButtonPopup)
    {
        option.subControls |= QStyle::SC_ToolButtonMenu;
        option.features |= QStyleOptionToolButton::MenuButtonPopup;
        option.features |= QStyleOptionToolButton::HasMenu;
    }
    if (tb->arrowType() != Qt::NoArrow)
        option.features |= QStyleOptionToolButton::Arrow;
    if (tb->popupMode() == QToolButton::DelayedPopup)
        option.features |= QStyleOptionToolButton::PopupDelay;

    QRect r = tb->style()->subControlRect(QStyle::CC_ToolButton, &option, QStyle::SC_ToolButtonMenu, tb);
    if (r.contains(pos))
        tb->setProperty(MENUPRESS, true);
    else
        tb->setProperty(MENUPRESS, false);
}

bool
ToolBar::isArrowPressed(const QToolButton *tb)
{
    return tb&&tb->property(MENUPRESS).toBool();
}

void
ToolBar::setDirty(QToolBar *bar)
{
    s_dirty.insert(bar, true);
}

bool
ToolBar::isDirty(QToolBar *bar)
{
    return bar&&s_dirty.value(bar, true);
}

static QList<qulonglong> s_toolBarQuery;

void
ToolBar::queryToolBarLater(QToolBar *bar)
{
    const qulonglong tb = (qulonglong)bar;
    if (!s_toolBarQuery.contains(tb))
    {
        s_toolBarQuery << tb;
        QMetaObject::invokeMethod(instance(), "queryToolBar", Qt::QueuedConnection, Q_ARG(qulonglong,tb));
    }
}

void
ToolBar::queryToolBar(qulonglong toolbar)
{
    s_toolBarQuery.removeOne(toolbar);
    QToolBar *bar = Ops::getChild<QToolBar *>(toolbar);
    if (!bar || !bar->isVisible() /*|| !bar->window()->isVisible()*/)
        return;

    if (isDirty(bar))
    {
//        QEvent e(QEvent::StyleChange);
//        QApplication::sendEvent(bar, &e);
        const QSize is = bar->iconSize();
        bar->removeEventFilter(this);
        bar->setIconSize(is - QSize(1, 1));
        bar->setIconSize(is);
        bar->installEventFilter(this);
//        s_dirty.insert(bar, false);
//        return;
    }
    const QList<QToolButton *> buttons(bar->findChildren<QToolButton *>());
    for (int i = 0; i < buttons.count(); ++i)
    {
        QToolButton *btn(buttons.at(i));
        Sides sides(All);
        if (bar->orientation() == Qt::Horizontal)
        {
            if (qobject_cast<QToolButton *>(bar->childAt(btn->geometry().topLeft()-QPoint(1, 0))))
                sides&=~Left;
            if (qobject_cast<QToolButton *>(bar->childAt(btn->geometry().topRight()+QPoint(2, 0))))
                sides&=~Right;
        }
        else
        {
            if (qobject_cast<QToolButton *>(bar->childAt(btn->geometry().topLeft()-QPoint(0, 1))))
                sides&=~Top;
            if (qobject_cast<QToolButton *>(bar->childAt(btn->geometry().bottomLeft()+QPoint(0, 2))))
                sides&=~Bottom;
        }
        s_sides.insert(btn, sides);
    }
    s_dirty.insert(bar, false);
}

Sides
ToolBar::sides(const QToolButton *btn)
{
    if (btn && qobject_cast<QToolBar *>(btn->parentWidget()) && isDirty(static_cast<QToolBar *>(btn->parentWidget())))
        queryToolBarLater(static_cast<QToolBar *>(btn->parentWidget()));
    return s_sides.value(const_cast<QToolButton *>(btn), All);
}

void
ToolBar::changeLater(QWidget *w)
{
    QMetaObject::invokeMethod(instance(), "change", Qt::QueuedConnection, Q_ARG(qulonglong, (qulonglong)w));
}

void
ToolBar::change(const qulonglong w)
{
    QWidget *widget = Ops::getChild<QWidget *>(w);
    if (!widget)
        return;
    QEvent e(QEvent::StyleChange);
    QApplication::sendEvent(widget, &e);
}

bool
ToolBar::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;

    switch (e->type())
    {
    case QEvent::ChildAdded:
    case QEvent::ChildRemoved:
    {
        if (qobject_cast<QToolBar *>(o) && static_cast<QChildEvent *>(e)->child()->isWidgetType())
            s_dirty.insert(static_cast<QToolBar *>(o), true);
        return false;
    }
    case QEvent::HideToParent:
    case QEvent::ShowToParent:
    {
        if (QToolBar *tb = qobject_cast<QToolBar *>(o))
        {
            if (e->type() == QEvent::ShowToParent && !TitleWidget::isManaging(tb))
                ToolbarHelpers::fixSpacerLater(tb);
            s_dirty.insert(tb, true);
//            queryToolBarLater(tb, true);
        }
        return false;
    }
    case QEvent::Show:
    case QEvent::Hide:
//    case QEvent::Move:
    {
        if (QToolBar *tb = qobject_cast<QToolBar *>(o))
        {
            s_dirty.insert(tb, true);
            if (!TitleWidget::isManaging(tb))
                ToolbarHelpers::fixSpacerLater(tb);
            ToolbarHelpers::adjustMarginsLater(tb);
//            queryToolBarLater(tb, true);
        }
        else if (QToolButton *tbn = qobject_cast<QToolButton *>(o))
        {
            QToolBar *tb = qobject_cast<QToolBar *>(tbn->parentWidget());
            if (!tb)
            {
//                changeLater(tbn);
                return false;
            }
            s_dirty.insert(tb, true);
//            queryToolBarLater(tb, true);
        }
        return false;
    }
    case QEvent::ActionChanged:
    {
        if (QToolBar *tb = qobject_cast<QToolBar *>(o))
            tb->update();
        return false;
    }
    case QEvent::MouseButtonPress:
    {
        QToolButton *tbn = qobject_cast<QToolButton *>(o);
        if (tbn && tbn->popupMode() == QToolButton::MenuButtonPopup)
            checkForArrowPress(tbn, static_cast<QMouseEvent *>(e)->pos());
        return false;
    }
    case QEvent::Resize:
    {
        QResizeEvent *re(static_cast<QResizeEvent *>(e));
        QToolBar *tb = qobject_cast<QToolBar *>(o);
        if (tb && dConf.uno.enabled && re->oldSize().height() != re->size().height())
            WindowHelpers::updateWindowDataLater(tb->window());
        return false;
    }
    default: return false;
    }
    return false;
}

static void applyMask(QWidget *widget)
{
    if (XHandler::opacity() < 1.0f)
    {
        widget->clearMask();
        if (!widget->windowFlags().testFlag(Qt::FramelessWindowHint))
        {
            widget->setWindowFlags(widget->windowFlags()|Qt::FramelessWindowHint);
            widget->show();
            ShadowHandler::manage(widget);
        }
        unsigned int d(0);
        XHandler::setXProperty<unsigned int>(widget->winId(), XHandler::_KDE_NET_WM_BLUR_BEHIND_REGION, XHandler::Long, &d);
    }
    else
    {
        const int w(widget->width()), h(widget->height());
        QRegion r(0, 2, w, h-4);
        r += QRegion(1, 1, w-2, h-2);
        r += QRegion(2, 0, w-4, h);
        widget->setMask(r);
    }
    widget->update();
}

//-------------------------------------------------------------------------------------------------

Window *Window::s_instance;

Window::Window(QObject *parent)
    : QObject(parent)
{
#if HASDBUS
    static const QString service("org.kde.dsp.kwindeco"),
            interface("org.kde.dsp.deco"),
            path("/DSPDecoAdaptor");
    QDBusInterface *iface = new QDBusInterface(service, path, interface);
//    iface->connection().connect(service, path, interface, "windowActiveChanged", this, SLOT(decoActiveChanged(QDBusMessage)));
    iface->connection().connect(service, path, interface, "dataChanged", this, SLOT(dataChanged(QDBusMessage)));
#endif
}

void
Window::manage(QWidget *w)
{
    if (!w)
        return;
    w->removeEventFilter(instance());
    w->installEventFilter(instance());
//    instance()->updateWindowData((qulonglong)w);
}

void
Window::release(QWidget *w)
{
    if (!w)
        return;
    w->removeEventFilter(instance());
}

Window
*Window::instance()
{
    if (!s_instance)
        s_instance = new Window();
    return s_instance;
}

void
Window::addCompactMenu(QWidget *w)
{
    if (instance()->m_menuWins.contains(w)
            || !qobject_cast<QMainWindow *>(w))
        return;
    instance()->m_menuWins << w;
    w->removeEventFilter(instance());
    w->installEventFilter(instance());
}

void
Window::menuShow()
{
    QMenu *m(static_cast<QMenu *>(sender()));
    m->clear();
    QMainWindow *mw(static_cast<QMainWindow *>(m->parentWidget()->window()));
    if (QMenuBar *menuBar = mw->findChild<QMenuBar *>())
        m->addActions(menuBar->actions());
}

bool
Window::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;
    QWidget *w = static_cast<QWidget *>(o);
    switch (e->type())
    {
    case QEvent::MouseButtonPress:
    {
        if (w->isWindow() && !qobject_cast<QDialog *>(w))
            return false;
        QMouseEvent *me(static_cast<QMouseEvent *>(e));
        if (QTabBar *tb = qobject_cast<QTabBar *>(w))
            if (tb->tabAt(me->pos()) != -1)
                return false;
        if (w->cursor().shape() != Qt::ArrowCursor
                || QApplication::overrideCursor()
                || w->mouseGrabber()
                || me->button() != Qt::LeftButton)
            return false;
        return false;
    }
    case QEvent::Paint:
    {
        if (dConf.hackDialogs
                && dConf.uno.enabled
                && qobject_cast<QDialog *>(w)
                && w->testAttribute(Qt::WA_TranslucentBackground)
                && w->windowFlags() & Qt::FramelessWindowHint)
        {
            QPainter p(w);
            p.setPen(Qt::NoPen);
            QColor bgColor(w->palette().color(w->backgroundRole()));
            if (XHandler::opacity() < 1.0f)
                bgColor.setAlpha(XHandler::opacity()*255.0f);
            p.setBrush(bgColor);
            p.setRenderHint(QPainter::Antialiasing);
            p.drawRoundedRect(w->rect(), 4, 4);
            p.end();
            return false;
        }
        else if (w->isWindow())
        {
            const QColor bgColor(w->palette().color(w->backgroundRole()));
            QPainter p(w);
            if (dConf.uno.enabled)
            {
                if (XHandler::opacity() < 1.0f
                        && qobject_cast<QMainWindow *>(w))
                    p.setClipRegion(paintRegion(static_cast<QMainWindow *>(w)));

            }
            GFX::drawWindowBg(&p, w, bgColor);
            if (WindowData *data = WindowData::memory(w->winId(), w))
            {
                QRect line(0, WindowHelpers::unoHeight(w), w->width(), 1);
                if (!data->value<bool>(WindowData::Separator, true) && data->value<bool>(WindowData::Uno))
                {
                    p.fillRect(line, QColor(0, 0, 0, dConf.shadows.opacity));
                    line.translate(0, 1);
                }
                p.fillRect(line, QColor(255, 255, 255, dConf.shadows.illumination));
            }
            p.end();
        }
        return false;
    }
    case QEvent::Show:
    {
        if (w->isWindow())
        {
            ///WINDOW SHOW, might need activation
//            if (!w->property(s_active).isValid())
//            {
//                if (WindowData *d = WindowData::memory(w->winId(), w))
//                    d->setValue<bool>(WindowData::IsActiveWindow, true);
//                w->setProperty(s_active, true);
//            }
            WindowHelpers::updateWindowDataLater(w);
        }
//        if (!dConf.uno.enabled)
//            QMetaObject::invokeMethod(this,
//                                      "updateDecoBg",
//                                      Qt::QueuedConnection,
//                                      Q_ARG(QWidget*,w));
        if (dConf.hackDialogs
                && qobject_cast<QDialog *>(w)
                && w->isModal())
        {
            QWidget *p = w->parentWidget();

            if  (!p && qApp)
                p = qApp->activeWindow();

            if (!p
                    || !qobject_cast<QMainWindow *>(p)
                    || p == w
                    || p->objectName() == "BE::Desk")
                return false;

            w->setWindowFlags(w->windowFlags() | Qt::FramelessWindowHint);
            w->setVisible(true);

            if (XHandler::opacity() < 1.0f)
            {
                w->setAttribute(Qt::WA_TranslucentBackground);
                unsigned int d(0);
                XHandler::setXProperty<unsigned int>(w->winId(), XHandler::_KDE_NET_WM_BLUR_BEHIND_REGION, XHandler::Long, &d);
            }

            int y(p->mapToGlobal(p->rect().topLeft()).y());
            if (p->windowFlags() & Qt::FramelessWindowHint)
            if (QToolBar *bar = p->findChild<QToolBar *>())
            if (bar->orientation() == Qt::Horizontal
                    && bar->geometry().topLeft() == QPoint(0, 0))
                y+=bar->height();
            const int x(p->mapToGlobal(p->rect().center()).x()-(w->width()/2));

            w->move(x, y);
        }
        else if (dConf.compactMenu
                 && w->testAttribute(Qt::WA_WState_Created)
                 && m_menuWins.contains(w))
        {
            QMainWindow *mw(static_cast<QMainWindow *>(w));
            QMenuBar *menuBar = mw->findChild<QMenuBar *>();
            if (!menuBar)
                return false;

            QToolBar *tb(mw->findChild<QToolBar *>());
            if (!tb || tb->findChild<QToolButton *>("DSP_menubutton"))
                return false;

            QToolButton *tbtn(new QToolButton(tb));
            tbtn->setText("Menu");
            QMenu *m = new QMenu(tbtn);
            m->addActions(menuBar->actions());
            connect(m, SIGNAL(aboutToShow()), this, SLOT(menuShow()));
            tbtn->setObjectName("DSP_menubutton");
            tbtn->setMenu(m);
            tbtn->setPopupMode(QToolButton::InstantPopup);
            QAction *before(tb->actions().first());
            tb->insertWidget(before, tbtn);
            tb->insertSeparator(before);
            menuBar->hide();
        }
        return false;
    }
//    case QEvent::Resize:
//    {
//        if (!dConf.uno.enabled && !dConf.windows.gradient.isEmpty() && w->isWindow())
//        {
//            QResizeEvent *re = static_cast<QResizeEvent *>(e);
//            const bool horChanged = dConf.windows.hor && re->oldSize().width() != re->size().width();
//            const bool verChanged = !dConf.windows.hor && re->oldSize().height() != re->size().height();
//            QSize sz(re->size());
//            if (verChanged)
//                if (unsigned char th = unoHeight(w, TitleBar))
//                    sz.rheight() += th;
//            if (horChanged  || verChanged)
//                updateDecoBg(w);
//        }
//        return false;
//    }
    case QEvent::PaletteChange:
    {
        if (w->isWindow())
            WindowHelpers::updateWindowDataLater(w);
        return false;
    }
    case QEvent::ApplicationPaletteChange:
    {
        GFX::generateData();
        return false;
    }
//    case QEvent::WindowTitleChange:
//    {
//        if (TitleWidget *w = o->findChild<TitleWidget *>())
//            w->update();
//    }
    default: break;
    }
    return false;
}

void
Window::updateDecoBg(QWidget *w)
{
    qDebug() << "DSP: Please fix Window::updateDecoBg(QWidget *w), its a dummy.";
//    QSize sz(w->size());
//    if (unsigned char *th = XHandler::getXProperty<unsigned char>(w->winId(), XHandler::DecoTitleHeight))
//    {
//        sz.rheight() += *th;
//        XHandler::freeData(th);
//    }
}

bool
Window::drawUnoPart(QPainter *p, QRect r, const QWidget *w, QPoint offset)
{
    QWidget *win(w->window());
    WindowData *data = WindowData::memory(win->winId(), win);
    if (!data)
        return false;

    const int clientUno = WindowHelpers::unoHeight(win);
    if (!clientUno)
        return false;
    if (const QToolBar *tb = qobject_cast<const QToolBar *>(w))
        if (tb->geometry().top() > clientUno)
            return false;

    if (!w->isWindow() && w->height() > w->width())
        return false;

    if (qobject_cast<QMainWindow *>(w->parentWidget()) && w->parentWidget()->parentWidget()) //weird case where mainwindow is embedded in another window
        return false;

    offset.ry() += data->value<int>(WindowData::TitleHeight, 0);
    if (data->lock())
    {
        QPoint bo(p->brushOrigin());
        p->setBrushOrigin(-offset);
        p->fillRect(r, data->image());
        p->setBrushOrigin(bo);
        data->unlock();
    }
    if (dConf.uno.contAware && w->mapTo(win, QPoint(0, 0)).y() < clientUno)
        ScrollWatcher::drawContBg(p, win, r, offset);
    return true;
}

//-------------------------------------------------------------------------------------------------

Drag Drag::s_instance;

Drag
*Drag::instance()
{
    return &s_instance;
}

void
Drag::manage(QWidget *w)
{
    w->removeEventFilter(instance());
    w->installEventFilter(instance());
}

bool
Drag::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType() || e->type() != QEvent::MouseButtonPress || QApplication::overrideCursor() || QWidget::mouseGrabber())
        return false;
    QWidget *w(static_cast<QWidget *>(o));
    if (w->cursor().shape() != Qt::ArrowCursor)
        return false;
    QMouseEvent *me(static_cast<QMouseEvent *>(e));
    if (!w->rect().contains(me->pos()) || me->button() != Qt::LeftButton)
        return false; 
    bool cd(canDrag(w));
    if (QTabBar *tabBar = qobject_cast<QTabBar *>(w))
        cd = (tabBar->tabAt(me->pos()) == -1);
    if (!cd)
        return false;

    XHandler::mwRes(w->mapTo(w->window(), me->pos()), me->globalPos(), w->window()->winId());
    return false;
}

bool
Drag::canDrag(QWidget *w)
{
    return (qobject_cast<QToolBar *>(w)
            || qobject_cast<QLabel *>(w)
            || qobject_cast<QTabBar *>(w)
            || qobject_cast<QStatusBar *>(w)
            || qobject_cast<QDialog *>(w));
}

//-------------------------------------------------------------------------------------------------

Q_DECL_EXPORT ScrollWatcher ScrollWatcher::s_instance;
QMap<QMainWindow *, QSharedMemory *> ScrollWatcher::s_mem;
static QList<QWidget *> s_watched;

void
ScrollWatcher::watch(QAbstractScrollArea *area)
{
    if (!area || s_watched.contains(area->viewport()))
        return;
    if (!dConf.uno.contAware || !qobject_cast<QMainWindow *>(area->window()))
        return;
    s_watched << area->viewport();
    connect(area->viewport(), SIGNAL(destroyed(QObject*)), instance(), SLOT(vpDeleted(QObject*)));
    area->viewport()->installEventFilter(instance());
    area->window()->installEventFilter(instance());
}

void
ScrollWatcher::vpDeleted(QObject *vp)
{
    s_watched.removeOne(static_cast<QWidget *>(vp));
}

void
ScrollWatcher::detachMem(QMainWindow *win)
{
    if (s_mem.contains(win))
    {
        QSharedMemory *m(s_mem.value(win));
        if (m->isAttached())
            m->detach();
    }
}

void
ScrollWatcher::updateWin(QWidget *mainWin)
{
    QMainWindow *win = static_cast<QMainWindow *>(mainWin);
    regenBg(win);
    const QList<QToolBar *> toolBars(win->findChildren<QToolBar *>());
    const int uno = WindowHelpers::unoHeight(win);
    for (int i = 0; i < toolBars.count(); ++i)
    {
        QToolBar *bar(toolBars.at(i));
        if (!bar->isVisible())
            continue;

        if (bar->geometry().bottom() < uno)
            bar->update();
    }
    if (QTabBar *tb = win->findChild<QTabBar *>())
        if (tb->isVisible())
            if (tb->mapTo(win, tb->rect().bottomLeft()).y() <=  uno)
            {
                if (QTabWidget *tw = qobject_cast<QTabWidget *>(tb->parentWidget()))
                {
                    const QList<QWidget *> kids(tw->findChildren<QWidget *>());
                    for (int i = 0; i < kids.count(); ++i)
                    {
                        QWidget *kid(kids.at(i));
                        if (kid->isVisible() && !kid->geometry().top())
                            kid->update();
                    }
                }
                else
                    tb->update();
            }
    if (WindowData *wd = WindowData::memory(win->winId(), win))
//        wd->sync();
        QMetaObject::invokeMethod(wd, "sync", Qt::QueuedConnection);
}

QSharedMemory
*ScrollWatcher::mem(QMainWindow *win)
{
    QSharedMemory *m(0);
    if (s_mem.contains(win))
        m = s_mem.value(win);
    else
    {
        m = new QSharedMemory(QString::number(win->winId()), win);
        s_mem.insert(win, m);
    }
    if (!m->isAttached())
    {
        const int dataSize(2048*128*4);
        if (!m->create(dataSize))
            return 0;
//        if (m->lock())
//        {
//            uchar *data = reinterpret_cast<uchar *>(m->data());
            /* Set N bytes of S to C.  */
//            memset(data, 0, dataSize);
//            QImage img(data, 2048, 128, QImage::Format_ARGB32_Premultiplied);
//            img.fill(Qt::transparent);
//            m->unlock();
//        }
    }
    return m;
}

void
ScrollWatcher::regenBg(QMainWindow *win)
{
    const int uno = WindowHelpers::unoHeight(win, true);
    const int title = WindowHelpers::unoHeight(win, false, true);
    if (QSharedMemory *m = mem(win))
    if (m->lock())
    {
        uchar *data = reinterpret_cast<uchar *>(m->data());
        QImage img(data, win->width(), uno, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);

        const QList<QAbstractScrollArea *> areas(win->findChildren<QAbstractScrollArea *>());
        for (int i = 0; i < areas.count(); ++i)
        {
            QAbstractScrollArea *area(areas.at(i));

            if (!area->isVisible()
                    || area->verticalScrollBar()->minimum() == area->verticalScrollBar()->maximum()
                    || area->verticalScrollBar()->value() == area->verticalScrollBar()->minimum())
                continue;
            const QPoint topLeft(area->mapTo(win, QPoint(0, 0)));
            if (topLeft.y()-1 > WindowHelpers::unoHeight(win) || area->verticalScrollBar()->value() == area->verticalScrollBar()->minimum())
                continue;

            QWidget *vp(area->viewport());
            vp->setAttribute(Qt::WA_UpdatesDisabled, true);
            vp->setAttribute(Qt::WA_ForceUpdatesDisabled, true);
            const bool hadFilter(s_watched.contains(vp));
            if (hadFilter)
                vp->removeEventFilter(this);
            area->blockSignals(true);
            area->verticalScrollBar()->blockSignals(true);
            vp->blockSignals(true);
            const int offset(vp->mapToParent(QPoint(0, 0)).y());
            QAbstractItemView *view = qobject_cast<QAbstractItemView *>(area);
            QAbstractItemView::ScrollMode mode;
            if (view)
            {
                mode = view->verticalScrollMode();
                view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            }

            const int prevVal(area->verticalScrollBar()->value());
            const int realVal(qMax(0, prevVal-(uno+offset)));

            area->verticalScrollBar()->setValue(realVal);
            vp->setAttribute(Qt::WA_NoSystemBackground); //this disables the viewport bg to get painted in the uno area, only items get painted
            vp->render(&img, vp->mapTo(win, QPoint(0, title-qMin(uno+offset, prevVal))), QRegion(0, 0, area->width(), uno), 0);
            vp->setAttribute(Qt::WA_NoSystemBackground, false);
            area->verticalScrollBar()->setValue(prevVal);
            if (view)
            {
                mode = view->verticalScrollMode();
                view->setVerticalScrollMode(mode);
            }
            vp->blockSignals(false);
            area->verticalScrollBar()->blockSignals(false);
            area->blockSignals(false);
            if (hadFilter)
                vp->installEventFilter(this);
            vp->setAttribute(Qt::WA_ForceUpdatesDisabled, false);
            vp->setAttribute(Qt::WA_UpdatesDisabled, false);
        }

        if (dConf.uno.opacity < 1.0f)
        {
            QPainter p(&img);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.fillRect(img.rect(), QColor(0, 0, 0, 255-(dConf.uno.opacity*255.0f)));
            p.end();
        }
        if (dConf.uno.blur)
            FX::expblur(img, dConf.uno.blur);
        m->unlock();
    }
}

void
ScrollWatcher::drawContBg(QPainter *p, QWidget *w, const QRect r, const QPoint &offset)
{
    QSharedMemory *m(0);
    if (s_mem.contains(static_cast<QMainWindow *>(w)))
        m = s_mem.value(static_cast<QMainWindow *>(w));
    if (!m || !m->isAttached())
        return;

    if (m->lock())
    {
        const uchar *data(reinterpret_cast<const uchar *>(m->constData()));
        p->drawImage(QPoint(0, 0), QImage(data, w->width(), WindowHelpers::unoHeight(w, true, true), QImage::Format_ARGB32_Premultiplied), r.translated(offset));
        m->unlock();
    }
}

bool
ScrollWatcher::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Close && qobject_cast<QMainWindow *>(o))
            detachMem(static_cast<QMainWindow *>(o));
    else if (e->type() == QEvent::Paint
             && qobject_cast<QAbstractScrollArea *>(o->parent()))
    {
        QWidget *w(static_cast<QWidget *>(o));
        QWidget *win(w->window());
        QPaintEvent *pe(static_cast<QPaintEvent *>(e));
        QAbstractScrollArea *a(static_cast<QAbstractScrollArea *>(w->parentWidget()));
        const int uno = WindowHelpers::unoHeight(win);
        if (!s_watched.contains(w)
                || a->mapTo(win, QPoint()).y()-1 > uno
                || a->verticalScrollBar()->minimum() == a->verticalScrollBar()->maximum()
                || pe->rect().top() > uno)
            return false;

        o->removeEventFilter(this);
        QCoreApplication::sendEvent(o, e);
        o->installEventFilter(this);
        if (a->mapTo(win, QPoint(0, 0)).y()-1 <= uno)
            QMetaObject::invokeMethod(this, "updateWin", Qt::QueuedConnection, Q_ARG(QWidget*,win));
        return true;
    }
    else if (e->type() == QEvent::Hide && qobject_cast<QAbstractScrollArea *>(o->parent()))
        updateWin(static_cast<QWidget *>(o)->window());
    return false;
}

static QPainterPath balloonPath(const QRect &rect)
{
    QPainterPath path;
    static const int m(8);
    path.addRoundedRect(rect.adjusted(8, 8, -8, -8), 4, 4);
    path.closeSubpath();
    const int halfH(rect.x()+(rect.width()/2.0f));
    const int halfV(rect.y()+(rect.height()/2.0f));
    QPolygon poly;
    switch (Handlers::BalloonHelper::mode())
    {
    case Handlers::BalloonHelper::Left:
    {
        const int pt[] = { rect.x(),halfV, rect.x()+m,halfV-m, rect.x()+m,halfV+m };
        poly.setPoints(3, pt);
        break;
    }
    case Handlers::BalloonHelper::Top:
    {
        const int pt[] = { halfH,rect.y(), halfH-m,rect.y()+m, halfH+m,rect.y()+m };
        poly.setPoints(3, pt);
        break;
    }
    case Handlers::BalloonHelper::Right:
    {
        break;
    }
    case Handlers::BalloonHelper::Bottom:
    {
        break;
    }
    default:
        break;
    }
    path.addPolygon(poly);
    path.closeSubpath();
    return path.simplified();
}

static QImage balloonTipShadow(const QRect &rect, const int radius)
{
    QImage img(rect.size()+QSize(radius*2, radius*2), QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawPath(balloonPath(img.rect().adjusted(radius, radius, -radius, -radius)));
    p.end();

    FX::expblur(img, radius);
    return img;
}

Balloon::Balloon() : QWidget(), m_label(new QLabel(this))
{
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setAutoFillBackground(true);
//    setWindowFlags(windowFlags()|Qt::FramelessWindowHint|Qt::X11BypassWindowManagerHint);
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setSpacing(0);
    l->addWidget(m_label);
    setLayout(l);
    setParent(0, windowFlags()|Qt::ToolTip);
    QPalette pal(palette());
    pal.setColor(QPalette::ToolTipBase, pal.color(QPalette::WindowText));
    pal.setColor(QPalette::ToolTipText, pal.color(QPalette::Window));
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(pal);
    m_label->setPalette(pal);
    QFont f(m_label->font());
    f.setPointSize(10);
    f.setBold(true);
    m_label->setFont(f);
}

static bool s_isInit(false);
static XHandler::XPixmap pix[8];

static void clearPix()
{
    for (int i = 0; i < 8; ++i)
        XHandler::freePix(pix[i]);
    s_isInit = false;
}

Balloon::~Balloon()
{
    if (s_isInit)
        clearPix();
}

void
Balloon::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.fillRect(rect(), palette().color(QPalette::ToolTipBase));
    p.end();
}

static const int s_padding(20);

void
Balloon::genPixmaps()
{
    if (s_isInit)
        clearPix();

    QImage img(size()+QSize(s_padding*2, s_padding*2), QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    static const int m(8);
    const QRect r(img.rect().adjusted(m, m, -m, -m));
    QPainter p(&img);
    p.drawImage(img.rect().topLeft(), balloonTipShadow(r.translated(0, 4), 8));
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(255, 255, 255, 255), 2.0f));
    p.setBrush(palette().color(QPalette::ToolTipBase));
    p.drawPath(balloonPath(r));
    p.end();

    pix[0] = XHandler::x11Pixmap(img.copy(s_padding, 0, img.width()-s_padding*2, s_padding)); //top
    pix[1] = XHandler::x11Pixmap(img.copy(img.width()-s_padding, 0, s_padding, s_padding)); //topright
    pix[2] = XHandler::x11Pixmap(img.copy(img.width()-s_padding, s_padding, s_padding, img.height()-s_padding*2)); //right
    pix[3] = XHandler::x11Pixmap(img.copy(img.width()-s_padding, img.height()-s_padding, s_padding, s_padding)); //bottomright
    pix[4] = XHandler::x11Pixmap(img.copy(s_padding, img.height()-s_padding, img.width()-s_padding*2, s_padding)); //bottom
    pix[5] = XHandler::x11Pixmap(img.copy(0, img.height()-s_padding, s_padding, s_padding)); //bottomleft
    pix[6] = XHandler::x11Pixmap(img.copy(0, s_padding, s_padding, img.height()-s_padding*2)); //left
    pix[7] = XHandler::x11Pixmap(img.copy(0, 0, s_padding, s_padding)); //topleft
    s_isInit = true;
}

void
Balloon::updateShadow()
{
    XHandler::XPixmap data[12];
    genPixmaps();
    for (int i = 0; i < 8; ++i)
        data[i] = pix[i];
    for (int i = 8; i < 12; ++i)
        data[i] = s_padding;
    XHandler::setXProperty<XHandler::XPixmap>(winId(), XHandler::_KDE_NET_WM_SHADOW, XHandler::Long, data, 12);
}

void
Balloon::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    BalloonHelper::updateBallon();
    updateShadow();
}

void
Balloon::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    BalloonHelper::updateBallon();
    updateShadow();
}

static Balloon *s_toolTip(0);

void
Balloon::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    s_toolTip = 0;
    deleteLater();
}

void
Balloon::setToolTipText(const QString &text)
{
    m_label->setText(text);
}

BalloonHelper BalloonHelper::s_instance;

Balloon
*BalloonHelper::balloon()
{
    if (!s_toolTip)
        s_toolTip = new Balloon();
    return s_toolTip;
}

void
BalloonHelper::manage(QWidget *w)
{
    w->setAttribute(Qt::WA_Hover);
    w->removeEventFilter(instance());
    w->installEventFilter(instance());
}

static QWidget *s_widget(0);
static BalloonHelper::Mode s_mode(BalloonHelper::Top);

BalloonHelper::Mode
BalloonHelper::mode()
{
    return s_mode;
}

void
BalloonHelper::updateBallon()
{
    if (!s_widget)
        return;
    if (balloon()->toolTipText() != s_widget->toolTip())
        balloon()->setToolTipText(s_widget->toolTip());
    QPoint globPt;
    const QRect globRect(s_widget->mapToGlobal(s_widget->rect().topLeft()), s_widget->size());
    globPt = QPoint(globRect.center().x()-(balloon()->width()/2), globRect.bottom()+s_padding);
    if (balloon()->pos() != globPt)
        balloon()->move(globPt);
}

bool
BalloonHelper::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return false;
    if (o->inherits("QTipLabel"))
    {
        if (e->type() == QEvent::Show && s_widget)
        {
            static_cast<QWidget *>(o)->hide();
            return true;
        }
        return false;
    }
    switch (e->type())
    {
    case QEvent::ToolTip:
    case QEvent::ToolTipChange:
    {
        if (e->type() == QEvent::ToolTipChange && (!balloon()->isVisible() || !static_cast<QWidget *>(o)->underMouse()))
            return false;

        if (qobject_cast<QAbstractButton *>(o)
                    || qobject_cast<QSlider *>(o)
                    || qobject_cast<QLabel *>(o))
        {
            s_widget = static_cast<QWidget *>(o);
            const bool isBeClock(o->inherits("BE::Clock"));
            if (isBeClock) //beclock sets the tooltip when a tooltip is requested, needs an extra push
            {
                o->removeEventFilter(this);
                QCoreApplication::sendEvent(o, e);
                o->installEventFilter(this);
            }
            const bool hasToolTip(!s_widget->toolTip().isEmpty());
            if (hasToolTip)
                updateBallon();
            if (QToolTip::isVisible())
                QToolTip::hideText();
            if (!balloon()->isVisible() && hasToolTip)
                balloon()->show();
            return true;
        }
        return false;
    }
    case QEvent::HoverLeave:
    {
        if (balloon()->isVisible())
            balloon()->hide();
        s_widget = 0;
        return false;
    }
    default: return false;
    }
}

//-------------------------------------------------------------------------------------------------

Dock *Dock::s_instance = 0;

Dock
*Dock::instance()
{
    if (!s_instance)
        s_instance = new Dock();
    return s_instance;
}

void
Dock::manage(QWidget *win)
{
    QMetaObject::invokeMethod(instance(), "lockWindowLater", Qt::QueuedConnection, Q_ARG(qulonglong, (qulonglong)win));
}

void
Dock::lockWindowLater(const qulonglong w)
{
    QWidget *win = Ops::getChild<QWidget *>(w);
    if (!win)
        return;
    QAction *a = new QAction("DSP::DockLocker", win);
    a->setObjectName("DSP::DockLocker");
    a->setShortcut(QKeySequence("Ctrl+Alt+D"));
    a->setShortcutContext(Qt::ApplicationShortcut);
    a->setCheckable(true);
    a->setChecked(false);
    connect(a, SIGNAL(toggled(bool)), instance(), SLOT(lockDocks(bool)));
    win->addAction(a);
    a->toggle();
}

void
Dock::release(QWidget *win)
{
    if (QAction *a = win->findChild<QAction *>("DSP::DockLocker"))
        a->deleteLater();
}

void
Dock::lockDocks(const bool locked)
{
    if (!sender())
        return;
    QWidget *win = static_cast<QWidget *>(sender()->parent());
    QList<QDockWidget *> docks = win->findChildren<QDockWidget *>();
    for (int i = 0; i < docks.count(); ++i)
    {
        if (locked)
            lockDock(docks.at(i));
        else
            unlockDock(docks.at(i));
    }
}

static QMap<QDockWidget *, QWidget *> s_customDockTitleBars;

void
Dock::lockDock(QDockWidget *dock)
{
    if (QWidget *w = dock->titleBarWidget())
        s_customDockTitleBars.insert(dock, w);

    QLabel *l = new QLabel(dock);
    l->setContentsMargins(0, -l->fontMetrics().height(), 0, -l->fontMetrics().height());
    l->setObjectName("DSP::DockLockerTitleBar");
    dock->setTitleBarWidget(l);
}

void
Dock::unlockDock(QDockWidget *dock)
{
    if (!dock->titleBarWidget() || dock->titleBarWidget()->objectName() != "DSP::DockLockerTitleBar")
        return;
    QWidget *w = dock->titleBarWidget();
    if (s_customDockTitleBars.contains(dock))
        dock->setTitleBarWidget(s_customDockTitleBars.value(dock));
    else
        dock->setTitleBarWidget(0);
    w->deleteLater();
}
