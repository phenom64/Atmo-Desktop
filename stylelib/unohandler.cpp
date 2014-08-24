#include "unohandler.h"
#include "xhandler.h"
#include "macros.h"
#include "color.h"
#include "render.h"
#include "ops.h"

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

Q_DECL_EXPORT UNOHandler *UNOHandler::s_instance = 0;
Q_DECL_EXPORT QMap<int, QPixmap> UNOHandler::s_pix;

UNOHandler::UNOHandler(QObject *parent)
    : QObject(parent)
{
}

UNOHandler
*UNOHandler::instance()
{
    if (!s_instance)
        s_instance = new UNOHandler();
    return s_instance;
}

void
UNOHandler::manage(QWidget *mw)
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
UNOHandler::release(QWidget *mw)
{
    mw->removeEventFilter(instance());
    QList<QToolBar *> toolBars = mw->findChildren<QToolBar *>();
    for (int i = 0; i < toolBars.count(); ++i)
        toolBars.at(i)->removeEventFilter(instance());
}

bool
UNOHandler::eventFilter(QObject *o, QEvent *e)
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
            fixWindowTitleBar(w);
        if (castObj(QToolBar *, toolBar, o))
            if (castObj(QMainWindow *, win, toolBar->parentWidget()))
                updateToolBar(toolBar);
        return false;
    }
    case QEvent::Show:
    {
        fixWindowTitleBar(w);
        return false;
    }
    default: break;
    }
    return false;
}

bool
UNOHandler::drawUnoPart(QPainter *p, QRect r, const QWidget *w, int offset)
{
    if (!w->isWindow())
        w = w->window();

    QVariant var(w->property("DSP_UNOheight"));
    if (var.isValid())
    {
        p->drawTiledPixmap(r, s_pix.value(var.toInt()), QPoint(0, offset));
        return true;
    }
    return false;
}

static QDBusMessage methodCall(const QString &method)
{
    return QDBusMessage::createMethodCall("org.kde.kwin", "/StyleProjectFactory", "org.kde.DBus.StyleProjectFactory", method);
}

void
UNOHandler::updateWindow(WId window)
{
    QDBusMessage msg = methodCall("update");
    msg << QVariant::fromValue((unsigned int)window);
    QDBusConnection::sessionBus().send(msg);
}


static unsigned int getHeadHeight(QWidget *win, unsigned int &needSeparator)
{
    unsigned char *h = XHandler::getXProperty<unsigned char>(win->winId(), XHandler::DecoData);
    if (!h)
        return 0;
    unsigned int totheight(*h);
    win->setProperty("titleHeight", totheight);
    castObj(QMainWindow *, mw, win);
    if (!mw)
        return totheight;
    int tbheight(0);
    if (mw->menuBar() && mw->menuBar()->isVisible())
        totheight += mw->menuBar()->height();
    QList<QToolBar *> tbs(mw->findChildren<QToolBar *>());
    for (int i = 0; i<tbs.count(); ++i)
    {
        QToolBar *tb(tbs.at(i));
        if (tb->isVisible())
        if (!tb->findChild<QTabBar *>())
        if (mw->toolBarArea(tb) == Qt::TopToolBarArea)
        {
            if (tb->geometry().bottom() > tbheight)
                tbheight = tb->height();
            needSeparator = 0;
        }
    }
    totheight += tbheight;
    if (Ops::isSafariTabBar(mw->findChild<QTabBar *>()))
        needSeparator = 0;
    win->setProperty("DSP_headHeight", tbheight);
    win->setProperty("DSP_UNOheight", totheight);
    return totheight;
}

void
UNOHandler::fixWindowTitleBar(QWidget *win)
{
    if (!win || !win->isWindow())
        return;
    unsigned int ns(1);
    WindowData wd;
    wd.height = getHeadHeight(win, ns);
    wd.separator = ns;
    wd.top = Color::titleBarColors[0].rgba();
    wd.bottom = Color::titleBarColors[1].rgba();
    wd.text = win->palette().color(win->foregroundRole()).rgba();
    if (!wd.height)
        return;
    XHandler::setXProperty<unsigned int>(win->winId(), XHandler::WindowData, XHandler::Short, reinterpret_cast<unsigned int *>(&wd), 5);
    updateWindow(win->winId());
//    qDebug() << ((c & 0xff000000) >> 24) << ((c & 0xff0000) >> 16) << ((c & 0xff00) >> 8) << (c & 0xff);
//    qDebug() << QColor(c).alpha() << QColor(c).red() << QColor(c).green() << QColor(c).blue();
    if (!s_pix.contains(wd.height))
    {
        QLinearGradient lg(0, 0, 0, wd.height);
        lg.setColorAt(0.0f, Color::titleBarColors[0]);
        lg.setColorAt(1.0f, Color::titleBarColors[1]);
        QPixmap p(Render::noise().width(), wd.height);
        p.fill(Qt::transparent);
        QPainter pt(&p);
        pt.fillRect(p.rect(), lg);
        pt.end();
        s_pix.insert(wd.height, Render::mid(p, Render::noise(), 40, 1));
    }

    const QList<QToolBar *> toolBars = win->findChildren<QToolBar *>();
    for (int i = 0; i < toolBars.size(); ++i)
        toolBars.at(i)->update();
    if (QMenuBar *mb = win->findChild<QMenuBar *>())
        mb->update();
}

void
UNOHandler::updateToolBar(QToolBar *toolBar)
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
            toolBar->setMovable(true);
            toolBar->layout()->setContentsMargins(0, 0, 0, 0);
            toolBar->setContentsMargins(0, 0, toolBar->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent), 6);
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
UNOHandler::fixTitleLater(QWidget *win)
{
    if (!win)
        return;
    QTimer *t = new QTimer(win);
    connect(t, SIGNAL(timeout()), instance(), SLOT(fixTitle()));
    t->start(250);
}

void
UNOHandler::fixTitle()
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
