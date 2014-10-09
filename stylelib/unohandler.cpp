#include "unohandler.h"
#include "xhandler.h"
#include "macros.h"
#include "color.h"
#include "render.h"
#include "ops.h"
#include "settings.h"
#include "shadowhandler.h"

#include <QMainWindow>
#include <QTabBar>
#include <QToolBar>
#include <QDBusMessage>
#include <QDBusConnection>
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
#include <QScrollBar>
#include <QElapsedTimer>

void
DButton::onClick(QMouseEvent *e, const Type &t)
{
    switch (t)
    {
    case Close: window()->close(); break;
    case Min: window()->showMinimized(); break;
    case Max: window()->isMaximized()?window()->showNormal():window()->showMaximized(); break;
    default: break;
    }
}

Buttons::Buttons(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *bl(new QHBoxLayout(this));
    bl->setContentsMargins(4, 4, 4, 4);
    bl->setSpacing(4);
    bl->addWidget(new DButton(Button::Close, this));
    bl->addWidget(new DButton(Button::Min, this));
    bl->addWidget(new DButton(Button::Max, this));
    setLayout(bl);
    if (parent)
        parent->window()->installEventFilter(this);
}

bool
Buttons::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;

    if (e->type() == QEvent::WindowDeactivate || e->type() == QEvent::WindowActivate || e->type() == QEvent::WindowTitleChange)
    {
        update();
        if (parentWidget())
            parentWidget()->update();
    }
    return false;
}

//-------------------------------------------------------------------------------------------------

void
TitleWidget::paintEvent(QPaintEvent *)
{
    const QString title(window()->windowTitle());
    QPainter p(this);
    QFont f(p.font());
    f.setBold(window()->isActiveWindow());
    f.setPointSize(f.pointSize()*1.2f);
    p.setFont(f);
    int align(Qt::AlignVCenter);
    switch (Settings::conf.titlePos)
    {
    case Left:
        align |= Qt::AlignLeft;
        break;
    case Center:
        align |= Qt::AlignHCenter;
        break;
    case Right:
        align |= Qt::AlignRight;
        break;
    default: break;
    }

    style()->drawItemText(&p, rect(), align, palette(), true, p.fontMetrics().elidedText(title, Qt::ElideRight, rect().width()), foregroundRole());
    p.end();
}


Q_DECL_EXPORT WinHandler WinHandler::s_instance;

WinHandler::WinHandler(QObject *parent)
    : QObject(parent)
{
}

void
WinHandler::manage(QWidget *w)
{
    if (!w)
        return;
    w->removeEventFilter(instance());
    w->installEventFilter(instance());
}

void
WinHandler::release(QWidget *w)
{
    if (!w)
        return;
    w->removeEventFilter(instance());
}

WinHandler
*WinHandler::instance()
{
    return &s_instance;
}

void
WinHandler::addCompactMenu(QWidget *w)
{
    if (instance()->m_menuWins.contains(w))
        return;
    instance()->m_menuWins << w;
    w->installEventFilter(instance());
}

bool
WinHandler::eventFilter(QObject *o, QEvent *e)
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
        if (castObj(QTabBar *, tb, w))
            if (tb->tabAt(me->pos()) != -1)
                return false;
        if (w->cursor().shape() != Qt::ArrowCursor
                || QApplication::overrideCursor()
                || w->mouseGrabber()
                || me->button() != Qt::LeftButton)
            return false;
        XHandler::mwRes(me->globalPos(), w->window()->winId());
        return false;
    }
    case QEvent::Paint:
    {
        if (qobject_cast<QDialog *>(w)
                && w->testAttribute(Qt::WA_TranslucentBackground)
                && w->windowFlags() & Qt::FramelessWindowHint)
        {
            QPainter p(w);
            p.setPen(Qt::NoPen);
            p.setRenderHint(QPainter::Antialiasing);
            p.setBrush(w->palette().color(w->backgroundRole()));
            p.setOpacity(XHandler::opacity());
            p.drawRoundedRect(w->rect(), 4, 4);
            p.end();
            return true;
        }
        return false;
    }
    case QEvent::Show:
    {
        if (Settings::conf.hackDialogs)
        if (qobject_cast<QDialog *>(w)
                && w->isModal())
        {
            QWidget *p = w->parentWidget();
            if  (!p)
                p = qApp->activeWindow();
            if (!p || p == w || p->objectName() == "BE::Desk")
                return false;

            w->setWindowFlags(w->windowFlags() | Qt::FramelessWindowHint);
            w->setVisible(true);
            if (XHandler::opacity() < 1.0f)
            {
                w->setAttribute(Qt::WA_TranslucentBackground);
                unsigned int d(0);
                XHandler::setXProperty<unsigned int>(w->winId(), XHandler::KwinBlur, XHandler::Long, &d);
            }
            int y(p->mapToGlobal(p->rect().topLeft()).y());
            if (p->windowFlags() & Qt::FramelessWindowHint
                    && qobject_cast<QMainWindow *>(p))
                if (QToolBar *bar = p->findChild<QToolBar *>())
                    if (bar->orientation() == Qt::Horizontal
                            && bar->geometry().topLeft() == QPoint(0, 0))
                        y+=bar->height();
            int x(p->mapToGlobal(p->rect().center()).x()-(w->width()/2));
            w->move(x, y);
        }
        if (Settings::conf.compactMenu && w->testAttribute(Qt::WA_WState_Created) && qobject_cast<QMainWindow *>(w) && m_menuWins.contains(w))
        {
            QMainWindow *mw(static_cast<QMainWindow *>(w));
            mw->menuBar()->hide();
            QToolBar *tb(mw->findChild<QToolBar *>());
            if (!tb || tb->findChild<QToolButton *>("DSP_menubutton"))
                return false;

            QToolButton *tbtn(new QToolButton(tb));
            tbtn->setText("Menu");
            QMenu *m = new QMenu(tbtn);
            m->addActions(mw->menuBar()->actions());
            connect(m, SIGNAL(aboutToShow()), this, SLOT(menuShow()));
            tbtn->setObjectName("DSP_menubutton");
            tbtn->setMenu(m);
            tbtn->setPopupMode(QToolButton::InstantPopup);
            QAction *before(tb->actions().first());
            tb->insertWidget(before, tbtn);
            tb->insertSeparator(before);
        }
        return false;
    }
    default: break;
    }
    return false;
}

void
WinHandler::menuShow()
{
    QMenu *m(static_cast<QMenu *>(sender()));
    m->clear();
    QMainWindow *mw(static_cast<QMainWindow *>(m->parentWidget()->window()));
    m->addActions(mw->menuBar()->actions());
}

bool
WinHandler::canDrag(QWidget *w)
{
    if (qobject_cast<QToolBar *>(w)
            || qobject_cast<QLabel *>(w)
            || qobject_cast<QTabBar *>(w)
            || qobject_cast<QStatusBar *>(w)
            || qobject_cast<QDialog *>(w))
        return true;
    return false;
}

//-------------------------------------------------------------------------------------------------

Q_DECL_EXPORT ScrollWatcher ScrollWatcher::s_instance;
QMap<qlonglong, QPixmap> ScrollWatcher::s_winBg;
QMap<QWidget *, Qt::HANDLE> ScrollWatcher::s_handles;

ScrollWatcher::ScrollWatcher(QObject *parent) : QObject(parent), m_timer(new QTimer(this))
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateLater()));
}

void
ScrollWatcher::watch(QAbstractScrollArea *area)
{
    if (!Settings::conf.contAware)
        return;
    area->window()->removeEventFilter(instance());
    area->window()->installEventFilter(instance());
    connect(area->window(), SIGNAL(destroyed()), instance(), SLOT(removeFromQueue()));
    area->viewport()->removeEventFilter(instance());
    area->viewport()->installEventFilter(instance());
}

void
ScrollWatcher::removeFromQueue()
{
    QMainWindow *mw(static_cast<QMainWindow *>(sender()));
    if (m_wins.contains(mw))
        m_wins.removeOne(mw);
    if (s_handles.contains(mw))
        XHandler::freePix(s_handles.value(mw));
}

void
ScrollWatcher::updateLater()
{
    for (int i = 0; i < m_wins.count(); ++i)
        updateWin(m_wins.at(i));
    m_wins.clear();
    m_timer->stop();
}

void
ScrollWatcher::updateWin(QMainWindow *win)
{
    regenBg(win);
    const QList<QToolBar *> toolBars(win->findChildren<QToolBar *>());
    for (int i = 0; i < toolBars.count(); ++i)
        toolBars.at(i)->update();
    if (QStatusBar *sb = win->findChild<QStatusBar *>())
        sb->update();
    if (QTabBar *tb = win->findChild<QTabBar *>())
        tb->update();
}

static bool s_block;

void
ScrollWatcher::regenBg(QMainWindow *win)
{
//    QElapsedTimer t;
//    t.start();
    s_block = true;
    const QRegion clipReg(paintRegion(win));
    const QRect bound(clipReg.boundingRect());
    const int height(bound.top());
    QImage img(win->width(), (height+22)/6/*+QSize(0, 22)*/, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);

    win->render(&p, QPoint(), QRect(QPoint(0, height+1), img.size()), QWidget::DrawChildren);

    QLinearGradient lg(img.rect().bottomLeft(), img.rect().topLeft());
    lg.setColorAt(0.0f, QColor(0, 0, 0, 160));
    lg.setColorAt(1.0f, Qt::transparent);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.fillRect(img.rect(), lg);
    p.end();
    img = img.scaled(img.width(), height+22);
    img = Render::blurred(img.mirrored(), img.rect(), 8);

    QPixmap pix(QPixmap::fromImage(img));


    QPixmap decoPix = XHandler::x11Pix(pix.copy(0, 0, pix.width(), 22), s_handles.value(win, 0));
    Qt::HANDLE d(decoPix.handle());
    if (!s_handles.contains(win))
        s_handles.insert(win, d);
    XHandler::setXProperty<unsigned long>(win->winId(), XHandler::ContPix, XHandler::Long, &d);
    UNO::Handler::updateWindow(win->winId());
    decoPix.detach();
    s_winBg.insert((qlonglong)win, pix.copy(0, 22, pix.width(), height));
    s_block = false;
//    qDebug() << "took" << t.elapsed() << "ms to gen gfx";
}

QPixmap
ScrollWatcher::bg(qlonglong win)
{
    return s_winBg.value(win, QPixmap());
}

bool
ScrollWatcher::eventFilter(QObject *o, QEvent *e)
{
    if (s_block)
        return false;
    if (e->type() == QEvent::Resize && qobject_cast<QMainWindow *>(o))
    {
        QMainWindow *win(static_cast<QMainWindow *>(o));
        if (!m_wins.contains(win))
            m_wins << win;
        if (s_handles.contains(win))
            XHandler::freePix(s_handles.take(win));
        if (!m_timer->isActive())
            m_timer->start(50);
    }
    if (e->type() == QEvent::Paint && !qobject_cast<QMainWindow *>(o))
    {

        QMainWindow *win(qobject_cast<QMainWindow *>(static_cast<QWidget *>(o)->window()));
        if (!m_wins.contains(win))
            m_wins << win;
        if (!m_timer->isActive())
            m_timer->start(50);
    }
    return false;
}

QRegion
ScrollWatcher::paintRegion(QMainWindow *win)
{
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
        if (bar->isVisible())
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

using namespace UNO;

Q_DECL_EXPORT Handler *Handler::s_instance = 0;
Q_DECL_EXPORT QMap<int, QVector<QPixmap> > Handler::s_pix;
Q_DECL_EXPORT QMap<QWidget *, UNO::Data> Handler::s_unoData;

Handler::Handler(QObject *parent)
    : QObject(parent)
{
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

Handler::~Handler()
{
}

Handler
*Handler::instance()
{
    if (!s_instance)
        s_instance = new Handler();
    return s_instance;
}

void
Handler::cleanUp()
{
    QMapIterator<int, QVector<QPixmap> > i(s_pix);
    while (i.hasNext())
    {
        QVector<QPixmap> pix(i.next().value());
        XHandler::freePix(pix[1]);
    }
    s_pix.clear();
}

void
Handler::manage(QWidget *mw)
{
    mw->removeEventFilter(instance());
    mw->installEventFilter(instance());
    QList<QToolBar *> toolBars = mw->findChildren<QToolBar *>();
    for (int i = 0; i < toolBars.count(); ++i)
    {
        QToolBar *bar(toolBars.at(i));
        bar->removeEventFilter(instance());
        bar->installEventFilter(instance());
    }
    fixTitleLater(mw);
}

void
Handler::release(QWidget *mw)
{
    mw->removeEventFilter(instance());
    QList<QToolBar *> toolBars = mw->findChildren<QToolBar *>();
    for (int i = 0; i < toolBars.count(); ++i)
        toolBars.at(i)->removeEventFilter(instance());
}

#define HASCHECK "DSP_hascheck"

static void applyMask(QWidget *widget)
{
    if (XHandler::opacity() < 1.0f)
    {
        widget->clearMask();
        if (!widget->windowFlags().testFlag(Qt::FramelessWindowHint) && Settings::conf.removeTitleBars)
        {
            widget->setWindowFlags(widget->windowFlags()|Qt::FramelessWindowHint);
            widget->show();
            ShadowHandler::manage(widget);
        }
        unsigned int d(0);
        XHandler::setXProperty<unsigned int>(widget->winId(), XHandler::KwinBlur, XHandler::Long, &d);
    }
    else
    {
        const int w(widget->width()), h(widget->height());
        QRegion r(0, 2, w, h-4);
        r += QRegion(1, 1, w-2, h-2);
        r += QRegion(2, 0, w-4, h);
        widget->setMask(r);
    }
    widget->setProperty(HASCHECK, true);
}

#define HASSTRETCH "DSP_hasstretch"
#define HASBUTTONS "DSP_hasbuttons"

static TitleWidget *canAddTitle(QToolBar *toolBar, bool &canhas)
{
    canhas = false;
    const QList<QAction *> actions(toolBar->actions());
    if (toolBar->toolButtonStyle() == Qt::ToolButtonIconOnly)
    {
        canhas = true;
        for (int i = 0; i < actions.count(); ++i)
        {
            QAction *a(actions.at(i));
            if (a->isSeparator())
                continue;
            QWidget *w(toolBar->widgetForAction(actions.at(i)));
            if (!w || qobject_cast<Buttons *>(w) || qobject_cast<TitleWidget *>(w))
                continue;
            if (!qobject_cast<QToolButton *>(w))
            {
                canhas = false;
                break;
            }
        }
    }
    TitleWidget *tw(toolBar->findChild<TitleWidget *>());
    if (!canhas && tw)
    {
        toolBar->removeAction(toolBar->actionAt(tw->geometry().topLeft()));
        tw->deleteLater();
        toolBar->setProperty(HASSTRETCH, false);
    }
    return tw;
}

void
Handler::setupNoTitleBarWindow(QToolBar *toolBar)
{
    if (!toolBar
            || toolBar->actions().isEmpty()
            || !qobject_cast<QMainWindow *>(toolBar->parentWidget())
            || toolBar->geometry().topLeft() != QPoint(0, 0)
            || toolBar->orientation() != Qt::Horizontal)
        return;
    if (toolBar->parentWidget()->parentWidget())
        return;

    Buttons *b(toolBar->findChild<Buttons *>());
    if (!b)
    {
        toolBar->insertWidget(toolBar->actions().first(), new Buttons(toolBar));
        toolBar->window()->installEventFilter(instance());
        toolBar->window()->setProperty(HASBUTTONS, true);
    }
    else
    {
        QAction *a(toolBar->actionAt(b->geometry().topLeft()));
        if (a)
        {
            toolBar->removeAction(a);
            toolBar->insertAction(toolBar->actions().first(), a);
        }
    }
    applyMask(toolBar->window());

    bool cantitle;
    TitleWidget *t = canAddTitle(toolBar, cantitle);
    if (cantitle)
    {
        QAction *ta(0);
        if (t)
        {
            ta = toolBar->actionAt(t->geometry().topLeft());
            toolBar->removeAction(ta);
        }
        TitleWidget *title(t?t:new TitleWidget(toolBar));
        title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QAction *a(0);
        if (Settings::conf.titlePos == TitleWidget::Center)
        {
            int at(qFloor((float)toolBar->actions().count()/2.0f));
            int i(at);
            const QList<QAction *> actions(toolBar->actions());
            a = actions.at(i);
            while (a && i > (at-3))
            {
                if (!a)
                {
                    toolBar->removeAction(toolBar->actionAt(title->geometry().topLeft()));
                    title->deleteLater();
                    return;
                }
                if (a->isSeparator())
                    break;
                else
                    a = actions.at(--i);
            }
        }
        else if (Settings::conf.titlePos == TitleWidget::Left)
            a = toolBar->actions().at(1);
        if (ta)
            toolBar->insertAction(a, ta);
        else
            toolBar->insertWidget(a, title);
        toolBar->setProperty(HASSTRETCH, true);
    }
    updateToolBar(toolBar);
    fixTitleLater(toolBar->parentWidget());
}

bool
Handler::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e)
        return false;
    if (!o->isWidgetType())
        return false;
    QWidget *w = static_cast<QWidget *>(o);
    switch (e->type())
    {
    case QEvent::Resize:
    {
        if (w->isWindow())
        {
            fixWindowTitleBar(w);
            if (qobject_cast<QMainWindow *>(w) && w->property(HASBUTTONS).toBool())
                applyMask(w);
        }
        if (castObj(QToolBar *, toolBar, o))
            if (castObj(QMainWindow *, win, toolBar->parentWidget()))
                updateToolBar(toolBar);
        return false;
    }
    case QEvent::Show:
    {
        fixWindowTitleBar(w);
        if (qobject_cast<QMainWindow *>(w) && XHandler::opacity() < 1.0f && !w->parentWidget())
        {
            unsigned int d(0);
            XHandler::setXProperty<unsigned int>(w->winId(), XHandler::KwinBlur, XHandler::Long, &d);
        }
        return false;
    }
    default: break;
    }
    return false;
}

bool
Handler::drawUnoPart(QPainter *p, QRect r, const QWidget *w, const QPoint &offset, float opacity)
{
    QWidget *win(w->window());
    const int clientUno(unoHeight(win, ToolBarAndTabBar));
    if (!clientUno)
        return false;
    if (const QToolBar *tb = qobject_cast<const QToolBar *>(w))
//        if (QMainWindow *mwin = qobject_cast<QMainWindow *>(tb->parentWidget()))
//            if (mwin->toolBarArea(const_cast<QToolBar *>(tb)) != Qt::TopToolBarArea)
//                if (tb->orientation() != Qt::Horizontal)
        if (tb->geometry().top() > clientUno)
            return false;

    if (!w->isWindow() && w->height() > w->width())
        return false;

    if (qobject_cast<QMainWindow *>(w->parentWidget()) && w->parentWidget()->parentWidget())
        return false;

    const int allUno(unoHeight(win, All));
    if (s_pix.contains(allUno))
    {
        const float savedOpacity(p->opacity());
        p->setOpacity(opacity);
        p->drawTiledPixmap(r, s_pix.value(allUno).at(0), offset);
        if (Settings::conf.contAware && w->mapTo(win, QPoint(0, 0)).y() < clientUno)
        {
            p->setOpacity(0.33f);
            p->drawTiledPixmap(r, ScrollWatcher::bg((qlonglong)win), offset);
        }
        p->setOpacity(savedOpacity);
        return true;
    }
    return false;
}

static QDBusMessage methodCall(const QString &method)
{
    return QDBusMessage::createMethodCall("org.kde.kwin", "/StyleProjectFactory", "org.kde.DBus.StyleProjectFactory", method);
}

void
Handler::updateWindow(WId window)
{
    QDBusMessage msg = methodCall("update");
    msg << QVariant::fromValue((unsigned int)window);
    QDBusConnection::sessionBus().send(msg);
}


unsigned int
Handler::getHeadHeight(QWidget *win, unsigned int &needSeparator)
{
    unsigned char *h = XHandler::getXProperty<unsigned char>(win->winId(), XHandler::DecoData);
    if (!h && !(Settings::conf.removeTitleBars && win->property("DSP_hasbuttons").toBool()))
        return 0;

    int hd[HeightCount];
    hd[TitleBar] = (h?*h:0);
    hd[All] = hd[TitleBar];
    hd[ToolBars] = hd[ToolBarAndTabBar] = 0;
    if (castObj(QMainWindow *, mw, win))
    {
        QMenuBar *menuBar = mw->findChild<QMenuBar *>();
        if (menuBar && menuBar->isVisible())
            hd[All] += menuBar->height();

        QList<QToolBar *> tbs(mw->findChildren<QToolBar *>());
        for (int i = 0; i<tbs.count(); ++i)
        {
            QToolBar *tb(tbs.at(i));
            if (tb->isVisible())
                if (!tb->findChild<QTabBar *>()) //ah eiskalt...
                    if (mw->toolBarArea(tb) == Qt::TopToolBarArea)
                    {
                        if (tb->geometry().bottom() > hd[ToolBars])
                            hd[ToolBars] = tb->geometry().bottom()+1;
                        needSeparator = 0;
                    }
        }
    }
    hd[ToolBarAndTabBar] = hd[ToolBars];
    QList<QTabBar *> tabbars = win->findChildren<QTabBar *>();
    QTabBar *possible(0);
    for (int i = 0; i < tabbars.count(); ++i)
    {
        const QTabBar *tb(tabbars.at(i));
        if (tb && tb->isVisible() && Ops::isSafariTabBar(tb))
        {
            possible = const_cast<QTabBar *>(tb);
            needSeparator = 0;
            const int y(tb->mapTo(win, tb->rect().bottomLeft()).y());
            if (y > hd[ToolBarAndTabBar])
                hd[ToolBarAndTabBar] = y;
        }
    }
    hd[All] += hd[ToolBarAndTabBar];
    s_unoData.insert(win, Data(hd, possible));
    return hd[All];
}

static QVector<QPixmap> unoParts(QWidget *win, int h)
{
    QLinearGradient lg(0, 0, 0, h);
    QColor bc(win->palette().color(win->backgroundRole()));
    bc = Color::mid(bc, Settings::conf.uno.tint.first, 100-Settings::conf.uno.tint.second, Settings::conf.uno.tint.second);
    lg.setStops(Settings::gradientStops(Settings::conf.uno.gradient, bc));

    const unsigned int n(Settings::conf.uno.noise);
    const int w(n?Render::noise().width():1);
    QPixmap p(w, h);
    p.fill(Qt::transparent);
    QPainter pt(&p);
    pt.fillRect(p.rect(), lg);
    pt.end();
    if (n)
        p = Render::mid(p, Render::noise(), 100-n, n);
    QVector<QPixmap> values;
    values << p.copy(0, unoHeight(win, TitleBar), w, unoHeight(win, ToolBarAndTabBar)) << XHandler::x11Pix(p.copy(0, 0, w, unoHeight(win, TitleBar)));
    return values;
}

void
Handler::fixWindowTitleBar(QWidget *win)
{
    if (!win || !win->isWindow())
        return;
    unsigned int ns(1);
    WindowData wd;
    wd.height = getHeadHeight(win, ns);
    wd.separator = ns;
    wd.opacity = win->testAttribute(Qt::WA_TranslucentBackground)?(unsigned int)(Settings::conf.opacity*100.0f):100;
    wd.text = win->palette().color(win->foregroundRole()).rgba();
    if (!wd.height)
        return;
    if (!s_pix.contains(wd.height))
        s_pix.insert(wd.height, unoParts(win, wd.height));
    unsigned long handle(s_pix.value(wd.height).at(1).handle());
    const WId id(win->winId());
    unsigned long *h = XHandler::getXProperty<unsigned long>(id, XHandler::DecoBgPix);
    if (!h || (h && *h != handle))
        XHandler::setXProperty<unsigned long>(id, XHandler::DecoBgPix, XHandler::Long, reinterpret_cast<unsigned long *>(&handle));
    XHandler::setXProperty<unsigned int>(id, XHandler::WindowData, XHandler::Short, reinterpret_cast<unsigned int *>(&wd), 4);
    updateWindow(win->winId());
//    qDebug() << ((c & 0xff000000) >> 24) << ((c & 0xff0000) >> 16) << ((c & 0xff00) >> 8) << (c & 0xff);
//    qDebug() << QColor(c).alpha() << QColor(c).red() << QColor(c).green() << QColor(c).blue();

    const QList<QToolBar *> toolBars = win->findChildren<QToolBar *>();
    for (int i = 0; i < toolBars.size(); ++i)
        toolBars.at(i)->update();
    if (QMenuBar *mb = win->findChild<QMenuBar *>())
        mb->update();
    if (QTabBar *tb = win->findChild<QTabBar *>())
        tb->update();
    if (QStatusBar *sb = win->findChild<QStatusBar *>())
        sb->update();
}

void
Handler::updateToolBar(QToolBar *toolBar)
{
    QMainWindow *win = static_cast<QMainWindow *>(toolBar->parentWidget());
    if (toolBar->isWindow() && toolBar->isFloating())
    {
        toolBar->setContentsMargins(0, 0, 0, 0);
        if (toolBar->layout())
            toolBar->layout()->setContentsMargins(4, 4, 4, 4);
    }
    else if (win->toolBarArea(toolBar) == Qt::TopToolBarArea && toolBar->layout())
    {
        if (toolBar->geometry().top() <= win->rect().top() && !toolBar->isWindow())
        {
            if (win->windowFlags() & Qt::FramelessWindowHint || !win->mask().isEmpty())
            {
                toolBar->setMovable(false);
                const int pm(6/*toolBar->style()->pixelMetric(QStyle::PM_ToolBarFrameWidth)*/);
                toolBar->layout()->setContentsMargins(pm, pm, pm, pm);
                toolBar->setContentsMargins(0, 0, 0, 0);
            }
            else
            {
                toolBar->setMovable(true);
                toolBar->layout()->setContentsMargins(0, 0, 0, 0);
                toolBar->setContentsMargins(0, 0, toolBar->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent), 6);
            }
        }
        else if (toolBar->findChild<QTabBar *>()) //sick, put a tabbar in a toolbar... eiskaltdcpp does this :)
        {
            toolBar->layout()->setContentsMargins(0, 0, 0, 0);
            toolBar->setMovable(false);
        }
        else
            toolBar->layout()->setContentsMargins(2, 2, 2, 2);
    }
}

void
Handler::fixTitleLater(QWidget *win)
{
    if (!win)
        return;
    QTimer *t = new QTimer(win);
    connect(t, SIGNAL(timeout()), instance(), SLOT(fixTitle()));
    t->start(250);
}

void
Handler::fixTitle()
{
    if (QTimer *t = qobject_cast<QTimer *>(sender()))
    if (t->parent()->isWidgetType())
    if (QWidget *w = static_cast<QWidget *>(t->parent()))
    if (w->isWindow())
    {
        fixWindowTitleBar(w);
        t->stop();
        t->deleteLater();
    }
}

