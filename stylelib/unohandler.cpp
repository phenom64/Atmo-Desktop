#include "unohandler.h"
#include "xhandler.h"
#include "macros.h"
#include "color.h"
#include "render.h"
#include "ops.h"
#include "settings.h"

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

Q_DECL_EXPORT UNOHandler UNOHandler::s_instance;
Q_DECL_EXPORT QMap<int, QPixmap> UNOHandler::s_pix;

Button::Button(Type type, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
    , m_hasPress(false)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_Hover);
    setFixedSize(16, 16);
    for (int i = 0; i < TypeCount; ++i)
        m_paintEvent[i] = 0;

    m_paintEvent[Close] = &Button::paintClose;
    m_paintEvent[Min] = &Button::paintMin;
    m_paintEvent[Max] = &Button::paintMax;
}

Button::~Button()
{

}

void
Button::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (m_type < TypeCount && m_paintEvent[m_type] && (this->*m_paintEvent[m_type])(p))
        return;
    p.end();
}

void
Button::mousePressEvent(QMouseEvent *e)
{
//    QWidget::mousePressEvent(e);
    e->accept();
    m_hasPress = true;
}

void
Button::mouseReleaseEvent(QMouseEvent *e)
{
//    QWidget::mouseReleaseEvent(e);
    if (m_hasPress)
    {
        e->accept();
        m_hasPress = false;
        if (rect().contains(e->pos()))
        {
            emit clicked();
            switch (m_type)
            {
            case Close: window()->close(); break;
            case Min: window()->showMinimized(); break;
            case Max: window()->isMaximized()?window()->showNormal():window()->showMaximized(); break;
            default: break;
            }
        }
    }
}

/**
 * @brief Button::drawBase
 * @param c
 * @param p
 * @param r
 * draw a maclike decobuttonbase using color c
 */

void
Button::drawBase(QColor c, QPainter &p, QRect &r) const
{
    switch (Settings::conf.titleButtons)
    {
    case 0:
    {
        p.save();
        p.setPen(Qt::NoPen);
        r.adjust(2, 2, -2, -2);
        p.setBrush(Color::mid(c, Qt::black, 4, 1));
        p.drawEllipse(r);
        p.setBrush(c);
        r.adjust(1, 1, -1, -1);
        p.drawEllipse(r);
        p.restore();
        break;
    }
    case 1:
    {
        c.setHsv(c.hue(), qMax(164, c.saturation()), c.value(), c.alpha());
        const QColor low(Color::mid(c, Qt::black, 5, 3));
        const QColor high(QColor(255, 255, 255, 127));
        r.adjust(2, 2, -2, -2);
        p.setBrush(high);
        p.drawEllipse(r.translated(0, 1));
        p.setBrush(low);
        p.drawEllipse(r);
        r.adjust(1, 1, -1, -1);

        QRadialGradient rg(r.center()+QPoint(1, r.height()/2-1), r.height()-1);
        rg.setColorAt(0.0f, Color::mid(c, Qt::white));
        rg.setColorAt(0.5f, c);

        rg.setColorAt(1.0f, Color::mid(c, Qt::black));
        p.setBrush(rg);
        p.drawEllipse(r);

        QRect rr(r);
        rr.setWidth(6);
        rr.setHeight(3);
        rr.moveCenter(r.center());
        rr.moveTop(r.top()+1);
        QLinearGradient lg(rr.topLeft(), rr.bottomLeft());
        lg.setColorAt(0.0f, QColor(255, 255, 255, 192));
        lg.setColorAt(1.0f, QColor(255, 255, 255, 64));
        p.setBrush(lg);
        p.drawEllipse(rr);
        break;
    }
    default: break;
    }
}

/** Stolen colors from bespin....
 *  none of them are perfect, but
 *  the uncommented ones are 'good enough'
 *  for me, search the web/pick better
 *  ones yourself if not happy
 */

// static uint fcolors[3] = {0x9C3A3A/*0xFFBF0303*/, 0xFFEB55/*0xFFF3C300*/, 0x77B753/*0xFF00892B*/};
// Font
//static uint fcolors[3] = {0xFFBF0303, 0xFFF3C300, 0xFF00892B};
// Aqua
// static uint fcolors[3] = { 0xFFD86F6B, 0xFFD8CA6B, 0xFF76D86B };

// static uint fcolors[3] = { 0xFFFF7E71, 0xFFFBD185, 0xFFB1DE96 }; <<<-best from bespin
// Aqua2
// static uint fcolors[3] = { 0xFFBF2929, 0xFF29BF29, 0xFFBFBF29 };

static uint fcolors[3] = { 0xFFFF7E71, 0xFFFBD185, 0xFF37CC40 };

bool
Button::paintClose(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(rect());
    drawBase(window()->isActiveWindow()?fcolors[m_type]:QColor(127, 127, 127, 255), p, r);
    if (underMouse())
    {
        p.setBrush(palette().color(foregroundRole()));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
}

bool
Button::paintMax(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(rect());
    drawBase(window()->isActiveWindow()?fcolors[m_type]:QColor(127, 127, 127, 255), p, r);
    if (underMouse())
    {
        p.setBrush(palette().color(foregroundRole()));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
}

bool
Button::paintMin(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(rect());
    drawBase(window()->isActiveWindow()?fcolors[m_type]:QColor(127, 127, 127, 255), p, r);
    if (underMouse())
    {
        p.setBrush(palette().color(foregroundRole()));
        p.drawEllipse(r.adjusted(3, 3, -3, -3));
    }
    p.end();
    return true;
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
    style()->drawItemText(&p, rect(), Qt::AlignCenter, palette(), true, p.fontMetrics().elidedText(title, Qt::ElideRight, rect().width()), foregroundRole());
    p.end();
}

UNOHandler::UNOHandler(QObject *parent)
    : QObject(parent)
{
}

UNOHandler
*UNOHandler::instance()
{
    return &s_instance;
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

#define HASCHECK "DSP_hascheck"

static void applyMask(QWidget *widget)
{
    if (XHandler::opacity() < 1.0f)
    {
        widget->setWindowFlags(widget->windowFlags()|Qt::FramelessWindowHint);
        widget->show();
        unsigned int d(1);
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
UNOHandler::setupNoTitleBarWindow(QToolBar *toolBar)
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
        int at(qFloor((float)toolBar->actions().count()/2.0f));
        int i(at);
        const QList<QAction *> actions(toolBar->actions());
        QAction *a = actions.at(i);
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
        return false;
    }
    default: break;
    }
    return false;
}

bool
UNOHandler::drawUnoPart(QPainter *p, QRect r, const QWidget *w, int offset, float opacity)
{
    if (const QToolBar *tb = qobject_cast<const QToolBar *>(w))
        if (QMainWindow *mwin = qobject_cast<QMainWindow *>(tb->parentWidget()))
            if (mwin->toolBarArea(const_cast<QToolBar *>(tb)) != Qt::TopToolBarArea)
                if (tb->orientation() != Qt::Horizontal)
                    return false;

    if (!w->isWindow() && w->height() > w->width())
        return false;

    if (!w->isWindow())
        w = w->window();

    QVariant var(w->property("DSP_UNOheight"));
    if (var.isValid())
    {
        p->save();
        p->setOpacity(opacity);
        p->drawTiledPixmap(r, s_pix.value(var.toInt()), QPoint(0, offset));
        p->restore();
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
    if (!h && !(Settings::conf.removeTitleBars && win->property("DSP_hasbuttons").toBool()))
        return 0;

    unsigned int totheight(h?*h:0);
    win->setProperty("titleHeight", totheight);
    int tbheight(0);
    if (castObj(QMainWindow *, mw, win))
    {
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
    }
    totheight += tbheight;
    if (Ops::isSafariTabBar(win->findChild<QTabBar *>()))
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
    wd.opacity = win->testAttribute(Qt::WA_TranslucentBackground)?(unsigned int)(Settings::conf.opacity*100.0f):100;
    wd.top = Color::titleBarColors[0].rgba();
    wd.bottom = Color::titleBarColors[1].rgba();
    wd.text = win->palette().color(win->foregroundRole()).rgba();
    if (!wd.height)
        return;
    XHandler::setXProperty<unsigned int>(win->winId(), XHandler::WindowData, XHandler::Short, reinterpret_cast<unsigned int *>(&wd), 6);
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
            if (win->windowFlags() & Qt::FramelessWindowHint)
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

//-------------------------------------------------------------------------------------------------

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
        if (w->isWindow())
            return false;
        QMouseEvent *me(static_cast<QMouseEvent *>(e));
        if (castObj(QTabBar *, tb, w))
            if (tb->tabAt(me->pos()) != -1)
                return false;
        if (w->cursor().shape() != Qt::ArrowCursor
                || QApplication::overrideCursor()
                || w->mouseGrabber())
            return false;
        XHandler::mwRes(me->globalPos(), w->window()->winId());
        return false;
    }
    case QEvent::Show:
    {
        if (Settings::conf.hackDialogs)
        if (qobject_cast<QDialog *>(w))
        {
            QWidget *p = w->parentWidget();
            if  (!p)
                p = qApp->activeWindow();
            if (!p || p == w)
                return false;
            w->setWindowFlags(w->windowFlags() | Qt::FramelessWindowHint);
            w->setVisible(true);
            int y(p->mapToGlobal(p->rect().topLeft()).y());
            if (p->windowFlags() & Qt::FramelessWindowHint
                    && qobject_cast<QMainWindow *>(p))
                if (QToolBar *bar = p->findChild<QToolBar *>())
                    if (bar->orientation() == Qt::Horizontal
                            && bar->geometry().topLeft() == QPoint(0, 0))
                        y+=bar->height();
            int x(p->mapToGlobal(p->rect().center()).x()-(w->width()/2));
            w->move(x, y);
            return false;
        }
    }
    default: break;
    }
    return false;
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
