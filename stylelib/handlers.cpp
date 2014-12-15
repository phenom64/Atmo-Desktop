#include "handlers.h"
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
#include <QAbstractItemView>
#include <QScrollBar>
#include <QElapsedTimer>
#include <QBuffer>
#include <QHeaderView>

static QRegion paintRegion(QMainWindow *win)
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
    switch (dConf.titlePos)
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

//-------------------------------------------------------------------------------------------------

using namespace Handlers;

Q_DECL_EXPORT ToolBar ToolBar::s_instance;

ToolBar
*ToolBar::instance()
{
    return &s_instance;
}

void
ToolBar::manage(QToolButton *tb)
{
    if (!qobject_cast<QToolBar *>(tb->parentWidget()))
        return;
    tb->removeEventFilter(instance());
    tb->installEventFilter(instance());
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
    }
    if (tb->arrowType() != Qt::NoArrow)
        option.features |= QStyleOptionToolButton::Arrow;
    if (tb->popupMode() == QToolButton::DelayedPopup)
        option.features |= QStyleOptionToolButton::PopupDelay;
    if (tb->menu())
        option.features |= QStyleOptionToolButton::HasMenu;
    QRect r = tb->style()->subControlRect(QStyle::CC_ToolButton, &option, QStyle::SC_ToolButtonMenu, tb);
    if (r.contains(pos))
        tb->setProperty("DSP_menupress", true);
    else
        tb->setProperty("DSP_menupress", false);
}

bool
ToolBar::isArrowPressed(const QToolButton *tb)
{
    return tb&&tb->property("DSP_menupress").toBool();
}

void
ToolBar::updateToolBarLater(QToolBar *bar, const int time)
{
    QTimer *t(bar->findChild<QTimer *>("DSP_toolbartimer"));
    if (!t)
    {
        t = new QTimer(bar);
        t->setObjectName("DSP_toolbartimer");
        connect(t, SIGNAL(timeout()), instance(), SLOT(updateToolBar()));
    }
    t->start(time);
}

void
ToolBar::forceButtonSizeReRead(QToolBar *bar)
{
    const QSize &iconSize(bar->iconSize());
    bar->setIconSize(iconSize - QSize(1, 1));
    bar->setIconSize(iconSize);
#if 0
    for (int i = 0; i < bar->actions().count(); ++i)
    {
        QAction *a(bar->actions().at(i));
        if (QWidget *w = bar->widgetForAction(a))
        {
            bool prevIsToolBtn(false);
            QAction *prev(0);
            if (i)
                prev = bar->actions().at(i-1);
            if (!prev)
                continue;
            prevIsToolBtn = qobject_cast<QToolButton *>(bar->widgetForAction(prev));
            if (!prevIsToolBtn && !prev->isSeparator() && !a->isSeparator())
                bar->insertSeparator(a);
        }
    }
#endif

    if (dConf.removeTitleBars
            && bar->geometry().topLeft() == QPoint(0, 0)
            && bar->orientation() == Qt::Horizontal
            && qobject_cast<QMainWindow *>(bar->parentWidget()))
        setupNoTitleBarWindow(bar);
}

void
ToolBar::adjustMargins(QToolBar *toolBar)
{
    QMainWindow *win = qobject_cast<QMainWindow *>(toolBar->parentWidget());
    if (!win || !toolBar || toolBar->actions().isEmpty())
      return;
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
                const int pm(6/*toolBar->style()->pixelMetric(QStyle::PM_ToolBarFrameWidth)*/);
                toolBar->layout()->setContentsMargins(pm, pm, pm, pm);
                toolBar->setContentsMargins(0, 0, 0, 0);
            }
            else
            {
                toolBar->layout()->setContentsMargins(0, 0, 0, 0);
                toolBar->QWidget::setContentsMargins(0, 0, dConf.uno.enabled*toolBar->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent), 6);
                if (!win->property(CSDBUTTONS).toBool() && !toolBar->property(TOOLPADDING).toBool())
                {
                    QWidget *w(new QWidget(toolBar));
                    const int sz(dConf.uno.enabled?8:4);
                    w->setFixedSize(sz, sz);
                    toolBar->insertWidget(toolBar->actions().first(), w);
                    toolBar->setProperty(TOOLPADDING, true);
                }
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
    Window::updateWindowDataLater(toolBar->window());
}

void
ToolBar::updateToolBar()
{
    QTimer *t = qobject_cast<QTimer *>(sender());
    if (!t)
        return;
    QToolBar *bar = qobject_cast<QToolBar *>(t->parent());
    if (!bar)
        return;

    forceButtonSizeReRead(bar);
    adjustMargins(bar);

    t->stop();
    t->deleteLater();
}

void
ToolBar::manage(QToolBar *tb)
{
    if (!tb)
        return;
    tb->removeEventFilter(instance());
    tb->installEventFilter(instance());
    if (qobject_cast<QMainWindow *>(tb->parentWidget()))
        connect(tb, SIGNAL(topLevelChanged(bool)), ToolBar::instance(), SLOT(adjustMarginsSlot()));
}

void
ToolBar::adjustMarginsSlot()
{
    adjustMargins(static_cast<QToolBar *>(sender()));
}

bool
ToolBar::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;
    QToolBar *tb = qobject_cast<QToolBar *>(o);
    QToolButton *tbn = qobject_cast<QToolButton *>(o);
    switch (e->type())
    {
    case QEvent::Show:
    {
        if (tbn)
        {
            updateToolBarLater(static_cast<QToolBar *>(tbn->parentWidget()));
//            forceButtonSizeReRead(static_cast<QToolBar *>(tbn->parentWidget()));
//            if (dConf.removeTitleBars)
//                setupNoTitleBarWindow(static_cast<QToolBar *>(tbn->parentWidget()));
        }
        else if (tb && dConf.uno.enabled)
            Window::updateWindowData(tb->window());
        return false;
    }
    case QEvent::Hide:
    {
        if (tb && dConf.uno.enabled)
            Window::updateWindowDataLater(tb->window());
        return false;
    }
    case QEvent::MouseButtonPress:
    {
        if (tbn && Ops::hasMenu(tbn))
            checkForArrowPress(tbn, static_cast<QMouseEvent *>(e)->pos());
        return false;
    }
    case QEvent::ActionChanged:
    {
        if (tb)
            tb->update();
        return false;
    }
    default: return false;
    }
    return false;
}

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

static void applyMask(QWidget *widget)
{
    if (XHandler::opacity() < 1.0f)
    {
        widget->clearMask();
        if (!widget->windowFlags().testFlag(Qt::FramelessWindowHint) && widget->property(CSDBUTTONS).toBool())
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
}

void
ToolBar::setupNoTitleBarWindow(QToolBar *toolBar)
{
    if (!toolBar
            || !toolBar->parentWidget()
            || toolBar->parentWidget()->parentWidget()
            || toolBar->actions().isEmpty()
            || !qobject_cast<QMainWindow *>(toolBar->parentWidget())
            || toolBar->geometry().topLeft() != QPoint(0, 0)
            || toolBar->orientation() != Qt::Horizontal)
        return;

    Buttons *b(toolBar->findChild<Buttons *>());
    if (!b)
    {
        toolBar->insertWidget(toolBar->actions().first(), new Buttons(toolBar));
        toolBar->window()->installEventFilter(instance());
        toolBar->window()->setProperty(CSDBUTTONS, true);
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
        if (dConf.titlePos == TitleWidget::Center)
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
        else if (dConf.titlePos == TitleWidget::Left)
            a = toolBar->actions().at(1);
        if (ta)
            toolBar->insertAction(a, ta);
        else
            toolBar->insertWidget(a, title);
        toolBar->setProperty(HASSTRETCH, true);
    }
//    if (TitleWidget *tw = toolBar->findChild<TitleWidget *>())
//    {

//    }
    adjustMargins(toolBar);
//    fixTitleLater(toolBar->parentWidget());
}

//-------------------------------------------------------------------------------------------------

Window Window::s_instance;
QMap<uint, QVector<QPixmap> > Window::s_unoPix;
QMap<QWidget *, Handlers::Data> Window::s_unoData;
QPixmap Window::s_bgPix;

Window::Window(QObject *parent)
    : QObject(parent)
{
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanUp()));
}

void
Window::manage(QWidget *w)
{
    if (!w)
        return;
    w->removeEventFilter(instance());
    w->installEventFilter(instance());
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
    return &s_instance;
}

void
Window::addCompactMenu(QWidget *w)
{
    if (instance()->m_menuWins.contains(w))
        return;
    instance()->m_menuWins << w;
    w->installEventFilter(instance());
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
        else if (w->isWindow())
        {
            const QColor bgColor(w->palette().color(w->backgroundRole()));
            QPainter p(w);
            p.setOpacity(XHandler::opacity());
            Render::Sides sides(Render::All);
            if (!w->property(CSDBUTTONS).toBool())
                sides &= ~Render::Top;
            QBrush b;
            if (dConf.uno.enabled)
            {
                if (XHandler::opacity() < 1.0f && qobject_cast<QMainWindow *>(w))
                {
                    p.setOpacity(1.0f);
                    p.setClipRegion(paintRegion(static_cast<QMainWindow *>(w)));
                }
                b = bgColor;
            }
            else
            {
                unsigned char *addV(XHandler::getXProperty<unsigned char>(w->winId(), XHandler::DecoData));
                if (addV)
                    b = s_bgPix.copy(0, *addV, s_bgPix.width(), s_bgPix.height()-*addV);
            }
            Render::renderMask(w->rect(), &p, b, 4, sides);
            p.setPen(Qt::black);
            p.setOpacity(dConf.shadows.opacity);
            if (dConf.uno.enabled)
            if (int th = Handlers::unoHeight(w, Handlers::ToolBars))
                p.drawLine(0, th, w->width(), th);
            p.end();
            return false;
        }
        return false;
    }
    case QEvent::Show:
    {
        if (dConf.hackDialogs && qobject_cast<QDialog *>(w) && w->isModal())
        {
            QWidget *p = w->parentWidget();
            if  (!p)
                p = qApp->activeWindow();
            if (!p || !qobject_cast<QMainWindow *>(p) || p == w || p->objectName() == "BE::Desk")
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
        else
        {
            updateWindowDataLater(w);
            if (qobject_cast<QMainWindow *>(w) && XHandler::opacity() < 1.0f && !w->parentWidget())
            {
                unsigned int d(0);
                XHandler::setXProperty<unsigned int>(w->winId(), XHandler::KwinBlur, XHandler::Long, &d);
                QTimer::singleShot(500, w, SLOT(update()));
            }
            if (dConf.compactMenu && w->testAttribute(Qt::WA_WState_Created) && qobject_cast<QMainWindow *>(w) && m_menuWins.contains(w))
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
        }
        return false;
    }
    case QEvent::Resize:
    {
        if (w->isWindow())
        {
            updateWindowData(w);
            if (qobject_cast<QMainWindow *>(w) && w->property(CSDBUTTONS).toBool())
                applyMask(w);
        }
        return false;
    }
    default: break;
    }
    return false;
}

unsigned int
Window::getHeadHeight(QWidget *win, unsigned int &needSeparator)
{
    unsigned char *h = XHandler::getXProperty<unsigned char>(win->winId(), XHandler::DecoData);
    if (!h && !(dConf.removeTitleBars && win->property("DSP_hasbuttons").toBool()))
        return 0;
    if (!dConf.uno.enabled)
    {
        needSeparator = 0;
        if (win->property("DSP_hasbuttons").toBool())
            return 0;
        return *h;
    }
    const int oldHead(unoHeight(win, All));
    int hd[HeightCount];
    hd[TitleBar] = (h?*h:0);
    hd[All] = hd[TitleBar];
    hd[ToolBars] = hd[ToolBarAndTabBar] = 0;
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(win))
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
    if (oldHead != hd[All] && qobject_cast<QMainWindow *>(win))
        ScrollWatcher::detachMem(static_cast<QMainWindow *>(win));
    s_unoData.insert(win, Data(hd, possible));
    return hd[All];
}

static QVector<QPixmap> unoParts(QWidget *win, int h)
{
    const bool hor(dConf.uno.hor);
    QLinearGradient lg(0, 0, hor?win->width():0, hor?0:h);
    QColor bc(win->palette().color(win->backgroundRole()));
    bc = Color::mid(bc, dConf.uno.tint.first, 100-dConf.uno.tint.second, dConf.uno.tint.second);
    lg.setStops(Settings::gradientStops(dConf.uno.gradient, bc));

    const unsigned int n(dConf.uno.noise);
    const int w(hor?win->width():(n?Render::noise().width():1));
    QPixmap p(w, h);
    p.fill(Qt::transparent);
    QPainter pt(&p);
    pt.fillRect(p.rect(), lg);
    pt.end();
    if (n)
        p = Render::mid(p, QBrush(Render::noise()), 100-n, n);
    QVector<QPixmap> values;
    values << p.copy(0, unoHeight(win, TitleBar), w, unoHeight(win, ToolBarAndTabBar)) << XHandler::x11Pix(p.copy(0, 0, w, unoHeight(win, TitleBar)));
    return values;
}

static QPixmap bgPix(QWidget *win)
{
    const bool hor(dConf.windows.hor);
    QLinearGradient lg(0, 0, hor*win->width(), !hor*win->height());
    QColor bc(win->palette().color(win->backgroundRole()));
    lg.setStops(Settings::gradientStops(dConf.windows.gradient, bc));

    unsigned char *addV(XHandler::getXProperty<unsigned char>(win->winId(), XHandler::DecoData));
    if (!addV)
        return QPixmap();
    const unsigned int n(dConf.windows.noise);
    const int w(hor?win->width():(n?Render::noise().width():1));
    const int h(!hor?win->height()+*addV:(n?Render::noise().height():1));
    QPixmap p(w, h);
    p.fill(Qt::transparent);
    QPainter pt(&p);
    pt.fillRect(p.rect(), lg);

    if (n)
//        p = Render::mid(p, QBrush(Render::noise()), 100-n, n);
    {
        pt.setOpacity(n*0.01f);
        pt.drawTiledPixmap(p.rect(), Render::noise());
    }
    pt.end();
    if (Window::s_bgPix.handle())
        Window::s_bgPix.detach();
    Window::s_bgPix = XHandler::x11Pix(p);
    return Window::s_bgPix;
}

template<typename T> static void updateChildren(QWidget *win)
{
    const QList<T> kids = win->findChildren<T>();
    const int size(kids.size());
    for (int i = 0; i < size; ++i)
        kids.at(i)->update();
}

void
Window::updateWindowData(QWidget *win)
{
    if (!win || !win->isWindow())
        return;
    unsigned int ns(1);
    WindowData wd;
    wd.height = getHeadHeight(win, ns);
    wd.separator = ns;
    wd.opacity = win->testAttribute(Qt::WA_TranslucentBackground)?(unsigned int)(XHandler::opacity()*100.0f):100;
    wd.text = win->palette().color(win->foregroundRole()).rgba();
    wd.bg = win->palette().color(win->backgroundRole()).rgba();
    wd.uno = dConf.uno.enabled;
    wd.contAware = dConf.uno.enabled&&dConf.uno.contAware;

    if (!wd.height && dConf.uno.enabled)
        return;
    const WId id(win->winId());
    unsigned long handle(0);
    if (dConf.uno.enabled)
    {
        uint check = wd.height;
        if (dConf.uno.hor)
            check = check<<16|win->width();
        if (!s_unoPix.contains(check))
            s_unoPix.insert(check, unoParts(win, wd.height));
        handle = s_unoPix.value(check).at(1).handle();
    }
    else
        handle = bgPix(win).handle();

    unsigned long *h = XHandler::getXProperty<unsigned long>(id, XHandler::DecoBgPix);
    if (!h || (h && *h != handle))
        XHandler::setXProperty<unsigned long>(id, XHandler::DecoBgPix, XHandler::Long, reinterpret_cast<unsigned long *>(&handle));

    if (dConf.uno.enabled)
    {
        updateChildren<QToolBar *>(win);
        updateChildren<QMenuBar *>(win);
        updateChildren<QTabBar *>(win);
        updateChildren<QStatusBar *>(win);
    }
    else
        win->update();
    //    qDebug() << ((c & 0xff000000) >> 24) << ((c & 0xff0000) >> 16) << ((c & 0xff00) >> 8) << (c & 0xff);
    //    qDebug() << QColor(c).alpha() << QColor(c).red() << QColor(c).green() << QColor(c).blue();
    XHandler::setXProperty<unsigned int>(id, XHandler::WindowData, XHandler::Short, reinterpret_cast<unsigned int *>(&wd), 7);
    updateWindow(win->winId());
}

void
Window::menuShow()
{
    QMenu *m(static_cast<QMenu *>(sender()));
    m->clear();
    QMainWindow *mw(static_cast<QMainWindow *>(m->parentWidget()->window()));
    m->addActions(mw->menuBar()->actions());
}

bool
Window::drawUnoPart(QPainter *p, QRect r, const QWidget *w, const QPoint &offset, float opacity)
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
    uint check = allUno;
    if (dConf.uno.hor)
        check = check<<16|win->width();
    if (s_unoPix.contains(check))
    {
        const float savedOpacity(p->opacity());
        p->setOpacity(opacity);
        p->drawTiledPixmap(r, s_unoPix.value(check).at(0), offset);
        if (dConf.uno.contAware && ScrollWatcher::hasBg((qlonglong)win) && w->mapTo(win, QPoint(0, 0)).y() < clientUno)
        {
            p->setOpacity(dConf.uno.opacity);
            p->drawImage(QPoint(0, 0), ScrollWatcher::bg((qlonglong)win), r.translated(offset+QPoint(0, unoHeight(win, TitleBar))));
        }
        p->setOpacity(savedOpacity);
        return true;
    }
    return false;
}

void
Window::updateWindowDataLater(QWidget *win)
{
    if (!win)
        return;
    QTimer *t = new QTimer(win);
    connect(t, SIGNAL(timeout()), instance(), SLOT(updateWindowDataSlot()));
    t->start(250);
}

void
Window::updateWindowDataSlot()
{
    if (QTimer *t = qobject_cast<QTimer *>(sender()))
    if (t->parent()->isWidgetType())
    if (QWidget *w = static_cast<QWidget *>(t->parent()))
    if (w->isWindow())
    {
        updateWindowData(w);
        t->stop();
        t->deleteLater();
    }
}

static QDBusMessage methodCall(const QString &method)
{
    return QDBusMessage::createMethodCall("org.kde.kwin", "/StyleProjectFactory", "org.kde.DBus.StyleProjectFactory", method);
}

void
Window::updateWindow(WId window, unsigned int changed)
{
    QDBusMessage msg = methodCall("update");
    msg << QVariant::fromValue((unsigned int)window) << QVariant::fromValue(changed);
    QDBusConnection::sessionBus().send(msg);
}

void
Window::cleanUp()
{
    QMapIterator<uint, QVector<QPixmap> > i(s_unoPix);
    while (i.hasNext())
    {
        QVector<QPixmap> pix(i.next().value());
        XHandler::freePix(pix[1]);
    }
    s_unoPix.clear();
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
    if (!o || !e || !o->isWidgetType() || e->type() != QEvent::MouseButtonPress)
        return false;
    QWidget *w(static_cast<QWidget *>(o));
    QMouseEvent *me(static_cast<QMouseEvent *>(e));
    bool cd(canDrag(w));
    if (QTabBar *tabBar = qobject_cast<QTabBar *>(w))
        cd = tabBar->tabAt(me->pos()) == -1;
    if (cd)
        XHandler::mwRes(me->globalPos(), w->window()->winId());
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
QMap<qlonglong, QImage> ScrollWatcher::s_winBg;
QMap<QMainWindow *, QSharedMemory *> ScrollWatcher::s_mem;
static QList<QWidget *> s_watched;

ScrollWatcher::ScrollWatcher(QObject *parent) : QObject(parent)
{
}

void
ScrollWatcher::watch(QAbstractScrollArea *area)
{
    if (!area || s_watched.contains(area->viewport()))
        return;
    if (!dConf.uno.contAware || !qobject_cast<QMainWindow *>(area->window()))
        return;
    s_watched << area->viewport();
//    area->viewport()->removeEventFilter(instance());
    area->viewport()->installEventFilter(instance());
    area->window()->installEventFilter(instance());
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
    if (s_winBg.contains((qulonglong)win))
        s_winBg.remove((qulonglong)win);
}

static QMainWindow *s_win(0);

void
ScrollWatcher::updateLater()
{
    if (s_win)
        updateWin(s_win);
}

void
ScrollWatcher::updateWin(QMainWindow *win)
{
    blockSignals(true);
    regenBg(win);
    const QList<QToolBar *> toolBars(win->findChildren<QToolBar *>());
    const int uno(unoHeight(win, Handlers::ToolBarAndTabBar));
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
    if (!dConf.removeTitleBars)
        Handlers::Window::updateWindow(win->winId(), 64);
    blockSignals(false);
}

void
ScrollWatcher::regenBg(QMainWindow *win)
{
    const int uno(unoHeight(win, Handlers::All)), titleHeight(unoHeight(win, Handlers::TitleBar));;
    if (uno == titleHeight)
        return;
    QSharedMemory *m(0);
    if (s_mem.contains(win))
        m = s_mem.value(win);
    else
    {
        m = new QSharedMemory(QString::number(win->winId()), win);
        s_mem.insert(win, m);
    }
    const int dataSize((win->width()*uno)*4);
    if (!m->isAttached())
        if (!m->create(dataSize))
            return;
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
            if (topLeft.y()-1 > (uno-unoHeight(win, Handlers::TitleBar)) || area->verticalScrollBar()->value() == area->verticalScrollBar()->minimum())
                continue;
            QWidget *vp(area->viewport());
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
            const bool needRm(s_watched.contains(vp));
            if (needRm)
                vp->removeEventFilter(this);
            vp->render(&img, vp->mapTo(win, QPoint(0, titleHeight-qMin(uno+offset, prevVal))), QRegion(0, 0, area->width(), uno), QWidget::DrawWindowBackground);

            area->verticalScrollBar()->setValue(prevVal);
            if (needRm)
                vp->installEventFilter(this);

            if (view)
            {
                mode = view->verticalScrollMode();
                view->setVerticalScrollMode(mode);
            }

            vp->blockSignals(false);
            area->verticalScrollBar()->blockSignals(false);
            area->blockSignals(false);
        }
        if (int blurRadius = dConf.uno.blur)
            Render::expblur(img, blurRadius);
        s_winBg.insert((qlonglong)win, img);
        m->unlock();
    }
}

QImage
ScrollWatcher::bg(qlonglong win)
{
    return s_winBg.value(win, QImage());
}

bool
ScrollWatcher::eventFilter(QObject *o, QEvent *e)
{
    static bool s_block(false);
    if ((e->type() == QEvent::Resize || e->type() == QEvent::Close) && qobject_cast<QMainWindow *>(o))
        detachMem(static_cast<QMainWindow *>(o));  
    else if (e->type() == QEvent::Paint && !s_block && qobject_cast<QAbstractScrollArea *>(o->parent()))
    {
        s_win = 0;
        QWidget *w(static_cast<QWidget *>(o));
        QAbstractScrollArea *a(static_cast<QAbstractScrollArea *>(w->parentWidget()));
        if (!s_watched.contains(w)
                || a->verticalScrollBar()->minimum() == a->verticalScrollBar()->maximum())
            return false;
        QMainWindow *win(static_cast<QMainWindow *>(w->window()));
        s_block = true;
        o->removeEventFilter(this);
        QCoreApplication::sendEvent(o, e);
        s_win = win;
        if (w->parentWidget()->mapTo(win, QPoint(0, 0)).y()-1 <= unoHeight(win, Handlers::ToolBarAndTabBar))
            QTimer::singleShot(0, this, SLOT(updateLater()));
        o->installEventFilter(this);
        s_block = false;
        return true;
    }
    else if (e->type() == QEvent::Hide && qobject_cast<QAbstractScrollArea *>(o->parent()))
        updateWin(static_cast<QMainWindow *>(static_cast<QWidget *>(o)->window()));
    return false;
}
