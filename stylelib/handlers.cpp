#include "handlers.h"
#include "xhandler.h"
#include "macros.h"
#include "color.h"
#include "render.h"
#include "ops.h"
#include "../config/settings.h"
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
#include <QToolTip>
#include <QDesktopWidget>
#include <QGroupBox>

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

Buttons::Buttons(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *bl(new QHBoxLayout(this));
    bl->setContentsMargins(4, 4, 4, 4);
    bl->setSpacing(4);
    bl->addWidget(new Button(Button::Close, this));
    bl->addWidget(new Button(Button::Min, this));
    bl->addWidget(new Button(Button::Max, this));
    setLayout(bl);
    if (parent)
        parent->window()->installEventFilter(this);
}

bool
Buttons::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType() || !static_cast<QWidget *>(o)->testAttribute(Qt::WA_WState_Created))
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

ToolBar ToolBar::s_instance;
QMap<QToolButton *, Render::Sides> ToolBar::s_sides;
QMap<QToolBar *, bool> ToolBar::s_dirty;
QMap<QToolBar *, QAction *> ToolBar::s_spacers;

ToolBar
*ToolBar::instance()
{
    return &s_instance;
}

void
ToolBar::manage(QWidget *child)
{
    if (!qobject_cast<QToolBar *>(child->parentWidget()))
        return;
    child->removeEventFilter(instance());
    child->installEventFilter(instance());
    if (qobject_cast<QToolButton *>(child))
        connect(child, SIGNAL(destroyed(QObject*)), instance(), SLOT(toolBtnDeleted(QObject*)));
}

void
ToolBar::manageToolBar(QToolBar *tb)
{
    if (!tb)
        return;
    tb->removeEventFilter(instance());
    tb->installEventFilter(instance());
    if (qobject_cast<QMainWindow *>(tb->parentWidget()))
        connect(tb, SIGNAL(topLevelChanged(bool)), ToolBar::instance(), SLOT(adjustMarginsSlot()));
    connect(tb, SIGNAL(orientationChanged(Qt::Orientation)), ToolBar::instance(), SLOT(adjustMarginsSlot()));
    connect(tb, SIGNAL(destroyed(QObject*)), instance(), SLOT(toolBarDeleted(QObject*)));
}

void
ToolBar::toolBarDeleted(QObject *toolBar)
{
    QToolBar *bar(static_cast<QToolBar *>(toolBar));
    if (s_dirty.contains(bar))
        s_dirty.remove(bar);
    if (s_spacers.contains(bar))
        s_spacers.remove(bar);
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
    }
    if (tb->arrowType() != Qt::NoArrow)
        option.features |= QStyleOptionToolButton::Arrow;
    if (tb->popupMode() == QToolButton::DelayedPopup)
        option.features |= QStyleOptionToolButton::PopupDelay;
    if (tb->menu())
        option.features |= QStyleOptionToolButton::HasMenu;
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
ToolBar::adjustMarginsSlot()
{
    QToolBar *toolBar = static_cast<QToolBar *>(sender());
    adjustMargins(toolBar);
    processToolBar(toolBar, true);
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
                const int pm(6);
                toolBar->layout()->setContentsMargins(pm, pm, pm, pm);
                toolBar->setContentsMargins(0, 0, 0, 0);
            }
            else
            {
                toolBar->layout()->setContentsMargins(0, 0, 0, 0);
                toolBar->QWidget::setContentsMargins(0, 0, toolBar->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent), 6);
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

bool
ToolBar::isDirty(QToolBar *bar)
{
    return bar&&s_dirty.value(bar, true);
}

void
ToolBar::processToolBar(QToolBar *bar, bool forceSizeUpdate)
{
    instance()->queryToolBar((qulonglong)bar, forceSizeUpdate);
}

void
ToolBar::separateToolButtons(QWidget *toolbar, int *actions, int n)
{
//    if (!actions)
//        return;
//    QToolBar *bar = static_cast<QToolBar *>(toolbar);
//    int a[n];
//    for (int i = 0; i < n; ++i)
//        a[i] = actions[i];
//    for (int i = bar->actions().count(); i > 0; --i)
}

void
ToolBar::queryToolBar(qulonglong toolbar, bool forceSizeUpdate)
{
    QToolBar *bar = getChild<QToolBar *>(toolbar);
    if (!bar)
        return;
    if (forceSizeUpdate)
    {
        const QSize is(bar->iconSize());
        bar->setIconSize(is - QSize(1, 1));
        bar->setIconSize(is);
        s_dirty.insert(bar, true);
        return;
    }
    bar->removeEventFilter(this);
    if (!bar->isVisible())
    {
        const QList<QAction *> actions(bar->actions());
        for (int i = 0; i < actions.count(); ++i)
        {
            QToolButton *btn = qobject_cast<QToolButton *>(bar->widgetForAction(actions.at(i)));
            if (btn)
            {
                Render::Sides sides(Render::All);
                if (i && qobject_cast<QToolButton *>(bar->widgetForAction(actions.at(i-1))))
                    sides &= ~(bar->orientation() == Qt::Horizontal?Render::Left:Render::Top);
                if (i+1 < actions.count())
                {
                    QAction *right(actions.at(i+1));
                    if (qobject_cast<QToolButton *>(bar->widgetForAction(right)))
                        sides &= ~(bar->orientation() == Qt::Horizontal?Render::Right:Render::Bottom);
                    //                    else if (!right->isSeparator() && bar->widgetForAction(right) && bar->isVisible())
                    //                    {
                    //                        qDebug() << "inserting separator before" << right << bar->widgetForAction(right);
                    //                        bar->insertSeparator(right);
                    //                    }
                }
//                else if (i == actions.count()-1 && bar->isVisible() && !actions.at(i)->isSeparator())
//                {
//                    qDebug() << "adding separator";
//                    bar->addSeparator();
//                }

                s_sides.insert(btn, sides);
            }
        }
    }
    else
    {
        const QList<QToolButton *> buttons(bar->findChildren<QToolButton *>());
        for (int i = 0; i < buttons.count(); ++i)
        {
            QToolButton *btn(buttons.at(i));
            Render::Sides sides(Render::All);
            if (bar->orientation() == Qt::Horizontal)
            {
                if (qobject_cast<QToolButton *>(bar->childAt(btn->geometry().topLeft()-QPoint(1, 0))))
                    sides&=~Render::Left;
                if (qobject_cast<QToolButton *>(bar->childAt(btn->geometry().topRight()+QPoint(2, 0))))
                    sides&=~Render::Right;
            }
            else
            {
                if (qobject_cast<QToolButton *>(bar->childAt(btn->geometry().topLeft()-QPoint(0, 1))))
                    sides&=~Render::Top;
                if (qobject_cast<QToolButton *>(bar->childAt(btn->geometry().bottomLeft()+QPoint(0, 2))))
                    sides&=~Render::Bottom;
            }
            s_sides.insert(btn, sides);
        }
    }
    s_dirty.insert(bar, false);
    bar->installEventFilter(this);
}

Render::Sides
ToolBar::sides(const QToolButton *btn)
{
    if (QToolBar *bar = qobject_cast<QToolBar *>(btn->parentWidget()))
        if (isDirty(bar))
            processToolBar(bar);
    return s_sides.value(const_cast<QToolButton *>(btn), Render::All);
}

void
ToolBar::fixSpacer(qulonglong toolbar)
{
    QToolBar *tb = getChild<QToolBar *>(toolbar);
    if (!tb
            || tb->window()->property(CSDBUTTONS).toBool()
            || !qobject_cast<QMainWindow *>(tb->parentWidget())
            || tb->findChild<QTabBar *>()
            || !tb->styleSheet().isEmpty())
        return;

    tb->removeEventFilter(this);
    if (!s_spacers.contains(tb))
    {
        QWidget *w(new QWidget(tb));
        w->setFixedSize(8, 8);
        s_spacers.insert(tb, tb->insertWidget(tb->actions().first(), w));
    }
    tb->removeAction(s_spacers.value(tb));
    tb->insertAction(tb->actions().first(), s_spacers.value(tb));
    s_spacers.value(tb)->setVisible(!tb->isMovable());
    tb->installEventFilter(this);
}

bool
ToolBar::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;
    QToolBar *tb = qobject_cast<QToolBar *>(o);
    QToolButton *tbn = qobject_cast<QToolButton *>(o);
    QToolBar *childParent(0);
    if (!tbn && !tb)
        childParent = qobject_cast<QToolBar *>(o->parent());

    switch (e->type())
    {
    case QEvent::ChildAdded:
    case QEvent::ChildRemoved:
    {
//        QChildEvent *ce = static_cast<QChildEvent *>(e);
//        if (tb
//                && ce->child()->isWidgetType()
//                && !static_cast<QWidget *>(ce->child())->isWindow())
//        {
//            QWidget *child = static_cast<QWidget *>(ce->child());
//            static int i(0);
//            qDebug() << child << ++i;
//            QMetaObject::invokeMethod(this, "fixSpacer", Qt::QueuedConnection, Q_ARG(qulonglong,(qulonglong)tb));

//        }
        if (tb)
            s_dirty.insert(tb, true);
        return false;
    }
    case QEvent::Show:
    case QEvent::Hide:
    {
        if (tb)
        {
            s_dirty.insert(tb, true);
            if (dConf.uno.enabled && dConf.removeTitleBars)
                setupNoTitleBarWindowLater(tb);

            adjustMargins(tb);
            Window::updateWindowDataLater(tb->window());
        }
        else if (childParent) //non-toolbutton child shown/hidden in toolbar, this does apparently not trigger a size-from-content reload.
            QMetaObject::invokeMethod(this, "queryToolBar", Qt::QueuedConnection, Q_ARG(qulonglong,(qulonglong)childParent), Q_ARG(bool,true));
        return false;
    }
    case QEvent::HideToParent:
    case QEvent::ShowToParent:
    {
        if (tb)
        {
            if (dConf.uno.enabled && dConf.removeTitleBars)
                setupNoTitleBarWindowLater(tb);
            else
            {
                s_dirty.insert(tb, true);
                QMetaObject::invokeMethod(this, "fixSpacer", Qt::QueuedConnection, Q_ARG(qulonglong,(qulonglong)tb));
            }
        }
        return false;
    }
    case QEvent::ActionAdded:
    case QEvent::ActionRemoved:
    case QEvent::ActionChanged:
    {
        QActionEvent *ae = static_cast<QActionEvent *>(e);
        if (tb)
        {
            if (e->type() == QEvent::ActionChanged)
            {
                tb->update();
                return false;
            }
//            if (dConf.uno.enabled)
                s_dirty.insert(tb, true);
            if (dConf.removeTitleBars)
            {
                QWidget *w(tb->widgetForAction(ae->action()));
                if (qobject_cast<QToolButton *>(w))
                    setupNoTitleBarWindowLater(tb);
            }
            else if (!tb->actions().isEmpty())
            {
                QMetaObject::invokeMethod(this, "fixSpacer", Qt::QueuedConnection, Q_ARG(qulonglong,(qulonglong)tb));
            }
        }
        return false;
    }
    case QEvent::MouseButtonPress:
    {
        if (tbn && Ops::hasMenu(tbn))
            checkForArrowPress(tbn, static_cast<QMouseEvent *>(e)->pos());
        return false;
    }
    case QEvent::Resize:
    {
        QResizeEvent *re(static_cast<QResizeEvent *>(e));
        if (tb && dConf.uno.enabled && re->oldSize().height() != re->size().height())
            Window::updateWindowDataLater(tb->window());
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
//    if (toolBar->toolButtonStyle() == Qt::ToolButtonIconOnly)
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

void
ToolBar::setupNoTitleBarWindowLater(QToolBar *toolBar)
{
    QMetaObject::invokeMethod(instance(), "setupNoTitleBarWindow", Qt::QueuedConnection, Q_ARG(qulonglong, (qulonglong)toolBar));
}

void
ToolBar::setupNoTitleBarWindow(qulonglong bar)
{
    QToolBar *toolBar(0);
    const QList<QWidget *> widgets = qApp->allWidgets();
    for (int i = 0; i < widgets.count(); ++i)
    {
        QWidget *w(widgets.at(i));
        if ((qulonglong)w == bar)
        {
            toolBar = static_cast<QToolBar *>(w);
            break;
        }
    }
    if (!toolBar
            || !toolBar->parentWidget()
            || toolBar->parentWidget()->parentWidget()
            || toolBar->actions().isEmpty()
            || !qobject_cast<QMainWindow *>(toolBar->parentWidget())
            || toolBar->geometry().topLeft() != QPoint(0, 0)
            || toolBar->orientation() != Qt::Horizontal)
        return;

    toolBar->removeEventFilter(instance());
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
        title->show();
    }
    processToolBar(toolBar, true);
    adjustMargins(toolBar);
    toolBar->installEventFilter(instance());
    Window::updateWindowDataLater(toolBar->parentWidget());
}

//-------------------------------------------------------------------------------------------------

Window Window::s_instance;
QMap<QWidget *, Handlers::Data> Window::s_unoData;

Window::Window(QObject *parent)
    : QObject(parent)
{
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
//            return false;
            unsigned char *addV(XHandler::getXProperty<unsigned char>(w->winId(), XHandler::DecoTitleHeight));
            const QColor bgColor(w->palette().color(w->backgroundRole()));
            QPainter p(w);
            Render::Sides sides(Render::All);
            if (!w->property(CSDBUTTONS).toBool())
                sides &= ~Render::Top;
            if (dConf.uno.enabled)
            {
                if (XHandler::opacity() < 1.0f
                        && qobject_cast<QMainWindow *>(w))
                    p.setClipRegion(paintRegion(static_cast<QMainWindow *>(w)));
                p.fillRect(w->rect(), bgColor);
            }
            else if (unsigned long *bgPix = XHandler::getXProperty<unsigned long>(w->winId(), XHandler::DecoBgPix))
            {
                p.drawTiledPixmap(w->rect(), QPixmap::fromX11Pixmap(*bgPix), QPoint(0, addV?*addV:0));
                XHandler::freeData(bgPix);
            }
            if (!w->isModal())
                Render::shapeCorners(&p, sides);
            p.setPen(QColor(0, 0, 0, dConf.shadows.opacity*255.0f));
            if (dConf.uno.enabled)
                if (int th = Handlers::unoHeight(w, ToolBars))
                    p.drawLine(0, th, w->width(), th);
            p.end();
            if (addV)
                XHandler::freeData(addV);
        }
        return false;
    }
    case QEvent::Show:
    {
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
        if (w->isWindow())
            updateWindowDataLater(w);
        if (!dConf.uno.enabled)
            QMetaObject::invokeMethod(this,
                                      "updateDecoBg",
                                      Qt::QueuedConnection,
                                      Q_ARG(QWidget*,w));
        return false;
    }
    case QEvent::Resize:
    {
        if (w->isWindow() && w->property(CSDBUTTONS).toBool())
            applyMask(w);
        if (!dConf.uno.enabled && !dConf.windows.gradient.isEmpty() && w->isWindow())
        {
            QResizeEvent *re = static_cast<QResizeEvent *>(e);
            const bool horChanged = dConf.windows.hor && re->oldSize().width() != re->size().width();
            const bool verChanged = !dConf.windows.hor && re->oldSize().height() != re->size().height();
            QSize sz(re->size());
            if (verChanged)
                if (unsigned char *th = XHandler::getXProperty<unsigned char>(w->winId(), XHandler::DecoTitleHeight))
                {
                    sz.rheight() += *th;
                    XHandler::freeData(th);
                }
            if (horChanged  || verChanged)
                updateDecoBg(w);
        }
        return false;
    }
    case QEvent::Close:
    case QEvent::PaletteChange:
    {
        if (w->isWindow())
        {
            if (unsigned long *bgPx = XHandler::getXProperty<unsigned long>(w->winId(), XHandler::DecoBgPix))
            {
                XHandler::deleteXProperty(w->winId(), XHandler::DecoBgPix);
                XHandler::freePix(*bgPx);
                XHandler::freeData(bgPx); //superfluous?
            }
            if (WindowData *wd = XHandler::getXProperty<WindowData>(w->winId(), XHandler::WindowData))
            {
                XHandler::deleteXProperty(w->winId(), XHandler::WindowData);
                XHandler::freeData(wd);
            }
            if (e->type() == QEvent::PaletteChange)
            {
                updateWindowDataLater(w);
            }
        }
        break;
    }
    default: break;
    }
    return false;
}

void
Window::updateDecoBg(QWidget *w)
{
    QSize sz(w->size());
    if (unsigned char *th = XHandler::getXProperty<unsigned char>(w->winId(), XHandler::DecoTitleHeight))
    {
        sz.rheight() += *th;
        XHandler::freeData(th);
    }
    QPixmap pix(bgPix(sz, w->palette().color(QPalette::Window)));
    Qt::HANDLE h(0);
    XHandler::x11Pix(pix, h);

    unsigned long *bgPx = XHandler::getXProperty<unsigned long>(w->winId(), XHandler::DecoBgPix);

    if (h)
        XHandler::setXProperty<unsigned long>(w->winId(), XHandler::DecoBgPix, XHandler::Long, &h);
    else
        XHandler::deleteXProperty(w->winId(), XHandler::DecoBgPix);

    if (bgPx)
    {
        XHandler::freePix(*bgPx);
        XHandler::freeData(bgPx); //superfluous?
    }
}

bool
Window::drawUnoPart(QPainter *p, QRect r, const QWidget *w, QPoint offset)
{
    QWidget *win(w->window());
    const int clientUno(unoHeight(win, ToolBarAndTabBar));
    if (!clientUno)
        return false;
    if (const QToolBar *tb = qobject_cast<const QToolBar *>(w))
        if (tb->geometry().top() > clientUno)
            return false;

    if (!w->isWindow() && w->height() > w->width())
        return false;

    if (qobject_cast<QMainWindow *>(w->parentWidget()) && w->parentWidget()->parentWidget()) //weird case where mainwindow is embedded in another window
        return false;

    const bool csd(/*(win->windowFlags() & Qt::FramelessWindowHint) && */win->property(CSDBUTTONS).toBool());
    if (!csd)
    if (unsigned char *titleHeight = XHandler::getXProperty<unsigned char>(win->winId(), XHandler::DecoTitleHeight))
    {
        offset.ry()+=*titleHeight;
        XHandler::freeData(titleHeight);
    }
    if (unsigned long *unoBg = XHandler::getXProperty<unsigned long>(win->winId(), XHandler::DecoBgPix))
    {
        p->drawTiledPixmap(r, QPixmap::fromX11Pixmap(*unoBg), offset);
        XHandler::freeData(unoBg);
        if (dConf.uno.contAware && w->mapTo(win, QPoint(0, 0)).y() < clientUno)
            ScrollWatcher::drawContBg(p, win, r, offset);

        if (csd)
        {
            QIcon icon(win->windowIcon());
            if (!icon.isNull() && !qobject_cast<const QStatusBar *>(w))
            {
                QPixmap pix(icon.pixmap(128));
//                QTransform tf;
//                tf.translate(pix.width()/2, pix.height()/2);
//                tf.rotate(-60.0f, Qt::YAxis);
//                tf.translate(-pix.width()/2, -pix.height()/2);
//                pix = pix.transformed(tf, Qt::SmoothTransformation);
                offset.ry()+=(pix.height()-clientUno)/2;
//                offset.rx()-=(win->findChild<Buttons *>()->width());
                QPainter pt(&pix);
                pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
                pt.fillRect(pix.rect(), QColor(0, 0, 0, 170));
                pt.end();
                p->drawPixmap(pix.rect().translated(-offset), pix);
            }
        }
        return true;
    }
    return false;
}

unsigned int
Window::getHeadHeight(QWidget *win, unsigned int &needSeparator)
{
    unsigned char *h = XHandler::getXProperty<unsigned char>(win->winId(), XHandler::DecoTitleHeight);
    const bool csd(dConf.removeTitleBars && win->property(CSDBUTTONS).toBool());
    if (!h && !csd)
    {
        needSeparator = false;
        return 0;
    }
    if (!dConf.uno.enabled)
    {
        needSeparator = 0;
        if (csd)
            return 0;
        if (h)
            return *h;
        return 0;
    }
//    const int oldHead(unoHeight(win, All));
    int hd[HeightCount];
    hd[TitleBar] = (h?*h:0);
    XHandler::freeData(h);
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
//    if (oldHead != hd[All] && qobject_cast<QMainWindow *>(win))
//        ScrollWatcher::detachMem(static_cast<QMainWindow *>(win));
    s_unoData.insert(win, Data(hd, possible));
    return hd[All];
}

QPixmap
Window::unoBgPix(QWidget *win, int h)
{
    const bool hor(dConf.uno.hor);
    QLinearGradient lg(0, 0, hor?win->width():0, hor?0:h);
    QPalette pal(win->palette());
    if (!pal.color(win->backgroundRole()).alpha() < 0xff)
        pal = QApplication::palette();
    QColor bc(pal.color(win->backgroundRole()));
    bc = Color::mid(bc, dConf.uno.tint.first, 100-dConf.uno.tint.second, dConf.uno.tint.second);
    lg.setStops(Settings::gradientStops(dConf.uno.gradient, bc));

    const unsigned int n(dConf.uno.noise);
    const int w(hor?win->width():(n?Render::noise().width():1));
    QPixmap p(w, h);
    p.fill(Qt::transparent);
    QPainter pt(&p);
    pt.fillRect(p.rect(), lg);
    if (n)
    {
//        p = Render::mid(p, QBrush(Render::noise()), 100-n, n);
        QPixmap noise(Render::noise().size());
        noise.fill(Qt::transparent);
        QPainter ptt(&noise);
        ptt.drawTiledPixmap(noise.rect(), Render::noise());
        ptt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        ptt.fillRect(noise.rect(), QColor(0, 0, 0, n*2.55f));
        ptt.end();
        pt.setCompositionMode(QPainter::CompositionMode_Overlay);
        pt.drawTiledPixmap(p.rect(), noise);
    }
    pt.end();

    if (XHandler::opacity() < 1.0f)
    {
        QPixmap copy(p);
        p.fill(Qt::transparent);
        pt.begin(&p);
        pt.drawPixmap(p.rect(), copy);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pt.fillRect(p.rect(), QColor(0, 0, 0, XHandler::opacity()*255.0f));
        pt.end();
    }
    return p;
}

QPixmap
Window::bgPix(const QSize &sz, const QColor &bgColor)
{
    int w = dConf.windows.hor&&!dConf.windows.gradient.isEmpty()?sz.width() : dConf.windows.noise?Render::noise().width() : 1;
    int h = !dConf.windows.hor&&!dConf.windows.gradient.isEmpty()?sz.height() : dConf.windows.noise?Render::noise().height() : 1;
    QPixmap pix(w, h);
    if (XHandler::opacity() < 1.0f)
        pix.fill(Qt::transparent);

    QPainter pt(&pix);
    if (!dConf.windows.gradient.isEmpty())
    {
        QLinearGradient lg(0, 0, dConf.windows.hor*pix.width(), !dConf.windows.hor*pix.height());
        lg.setStops(Settings::gradientStops(dConf.windows.gradient, bgColor));
        pt.fillRect(pix.rect(), lg);
    }
    else
        pt.fillRect(pix.rect(), bgColor);

    if (dConf.windows.noise)
    {
        static QPixmap noisePix;
        if (noisePix.isNull())
        {
            noisePix = QPixmap(Render::noise().size());
            noisePix.fill(Qt::transparent);
            QPainter p(&noisePix);
            p.drawPixmap(noisePix.rect(), Render::noise());
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.fillRect(noisePix.rect(), QColor(0, 0, 0, dConf.windows.noise*2.55f));
            p.end();
        }
        pt.setCompositionMode(QPainter::CompositionMode_Overlay);
        pt.drawTiledPixmap(pix.rect(), noisePix);
    }

    if (XHandler::opacity() < 1.0f)
    {
        pt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pt.fillRect(pix.rect(), QColor(0, 0, 0, dConf.opacity*255.0f));
        pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
    pt.end();
    return pix;
}

void
Window::updateWindowDataLater(QWidget *win)
{
    QMetaObject::invokeMethod(instance(), "updateWindowData", Qt::QueuedConnection, Q_ARG(qulonglong, (qulonglong)win));
}

void
Window::updateWindowData(qulonglong window)
{
    QWidget *win = getChild<QWidget *>(window);
    if (!win || !win->isWindow() || !win->testAttribute(Qt::WA_WState_Created))
        return;

    unsigned int ns(1);
    const unsigned int height(getHeadHeight(win, ns));

    WindowData wd;
    wd.setValue<bool>(WindowData::Separator, ns);
    wd.setValue<bool>(WindowData::ContAware, dConf.uno.enabled&&dConf.uno.contAware);
    wd.setValue<bool>(WindowData::Uno, dConf.uno.enabled);
    wd.setValue<bool>(WindowData::Horizontal, dConf.uno.enabled?dConf.uno.hor:dConf.windows.hor);
    wd.setValue<int>(WindowData::Opacity, XHandler::opacity()*255.0f);
    wd.setValue<int>(WindowData::UnoHeight, height);
    wd.setValue<int>(WindowData::Buttons, dConf.deco.buttons);
    wd.setValue<int>(WindowData::Frame, dConf.deco.frameSize);

    QPalette pal(win->palette());
    if (!pal.color(win->backgroundRole()).alpha() < 0xff) //im looking at you spotify you faggot!!!!!!!!!!!111111
        pal = QApplication::palette();

    wd.fg = pal.color(win->foregroundRole()).rgba();
    wd.bg = pal.color(win->backgroundRole()).rgba();

    WindowData *oldWd = XHandler::getXProperty<WindowData>(win->winId(), XHandler::WindowData);
    const bool needUpdate(!oldWd || *oldWd != wd);
    if (oldWd)
        XHandler::freeData(oldWd);

    if ((!height && dConf.uno.enabled) || !needUpdate)
        return;

    const WId id(win->winId());
    if (dConf.uno.enabled)
    {
        unsigned long handle(0);
        unsigned long *x11pixmap = XHandler::getXProperty<unsigned long>(id, XHandler::DecoBgPix);

        if (x11pixmap)
            handle = *x11pixmap;

        XHandler::x11Pix(unoBgPix(win, height), handle, win);

        if ((handle && !x11pixmap) || (x11pixmap && *x11pixmap != handle))
            XHandler::setXProperty<unsigned long>(id, XHandler::DecoBgPix, XHandler::Long, &handle);

        if (x11pixmap)
            XHandler::freeData(x11pixmap);
    }

    win->repaint();
    XHandler::setXProperty<WindowData>(id, XHandler::WindowData, XHandler::Byte, &wd);
    emit instance()->windowDataChanged(win);
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
    if (!w->rect().contains(me->pos()))
        return false;
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
        XHandler::updateDeco(win->winId());
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
        if (m->lock())
        {
            uchar *data = reinterpret_cast<uchar *>(m->data());
            QImage img(data, 2048, 128, QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::transparent);
            m->unlock();
        }
    }
    return m;
}

void
ScrollWatcher::regenBg(QMainWindow *win)
{
    const int uno(unoHeight(win, All)), titleHeight(unoHeight(win, TitleBar));;
    if (uno == titleHeight || !uno)
        return;

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
            if (topLeft.y()-1 > (uno-unoHeight(win, Handlers::TitleBar)) || area->verticalScrollBar()->value() == area->verticalScrollBar()->minimum())
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
            vp->render(&img, vp->mapTo(win, QPoint(0, titleHeight-qMin(uno+offset, prevVal))), QRegion(0, 0, area->width(), uno), 0);
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
        if (dConf.uno.blur)
            Render::expblur(img, dConf.uno.blur);

        if (dConf.uno.opacity < 1.0f)
        {
            QPainter p(&img);
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.fillRect(img.rect(), QColor(0, 0, 0, dConf.uno.opacity*255.0f));
            p.end();
        }
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
        p->drawImage(QPoint(0, 0), QImage(data, w->width(), unoHeight(w, All), QImage::Format_ARGB32_Premultiplied), r.translated(offset));
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
        if (!s_watched.contains(w)
                || a->mapTo(win, QPoint()).y()-1 > unoHeight(win, ToolBarAndTabBar)
                || a->verticalScrollBar()->minimum() == a->verticalScrollBar()->maximum()
                || pe->rect().top() > unoHeight(w->window(), All))
            return false;

        o->removeEventFilter(this);
        QCoreApplication::sendEvent(o, e);
        o->installEventFilter(this);
        if (a->mapTo(win, QPoint(0, 0)).y()-1 <= unoHeight(win, ToolBarAndTabBar))
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

    Render::expblur(img, radius);
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
static QPixmap pix[8];

static void clearPix()
{
    for (int i = 0; i < 8; ++i)
        XHandler::freePix(pix[i].handle());
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

    QPixmap px(size()+QSize(s_padding*2, s_padding*2));
    px.fill(Qt::transparent);
    static const int m(8);
    const QRect r(px.rect().adjusted(m, m, -m, -m));
    QPainter p(&px);
    p.drawImage(px.rect().topLeft(), balloonTipShadow(r.translated(0, 4), 8));
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(255, 255, 255, 255), 2.0f));
    p.setBrush(palette().color(QPalette::ToolTipBase));
    p.drawPath(balloonPath(r));
    p.end();

    pix[0] = XHandler::x11Pix(px.copy(s_padding, 0, px.width()-s_padding*2, s_padding)); //top
    pix[1] = XHandler::x11Pix(px.copy(px.width()-s_padding, 0, s_padding, s_padding)); //topright
    pix[2] = XHandler::x11Pix(px.copy(px.width()-s_padding, s_padding, s_padding, px.height()-s_padding*2)); //right
    pix[3] = XHandler::x11Pix(px.copy(px.width()-s_padding, px.height()-s_padding, s_padding, s_padding)); //bottomright
    pix[4] = XHandler::x11Pix(px.copy(s_padding, px.height()-s_padding, px.width()-s_padding*2, s_padding)); //bottom
    pix[5] = XHandler::x11Pix(px.copy(0, px.height()-s_padding, s_padding, s_padding)); //bottomleft
    pix[6] = XHandler::x11Pix(px.copy(0, s_padding, s_padding, px.height()-s_padding*2)); //left
    pix[7] = XHandler::x11Pix(px.copy(0, 0, s_padding, s_padding)); //topleft
    s_isInit = true;
}

void
Balloon::updateShadow()
{
    unsigned long data[12];
    genPixmaps();
    for (int i = 0; i < 8; ++i)
        data[i] = pix[i].handle();
    for (int i = 8; i < 12; ++i)
        data[i] = s_padding;

    XHandler::setXProperty<unsigned long>(winId(), XHandler::_KDE_NET_WM_SHADOW, XHandler::Long, data, 12);
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
