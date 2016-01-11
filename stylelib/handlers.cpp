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
#if HASDBUS
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include "stylelib/macmenu.h"
#endif

using namespace DSP;

static const char *s_active("DSP_activeWindow");

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
    bl->addWidget(new WidgetButton(ButtonBase::Close, this));
    bl->addWidget(new WidgetButton(ButtonBase::Minimize, this));
    bl->addWidget(new WidgetButton(ButtonBase::Maximize, this));
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

static const char *s_spacerName("DSP_TOOLBARSPACER");

TitleWidget::TitleWidget(QWidget *parent)
    : QWidget(parent)
{
    setContentsMargins(8, 0, 0, 0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_NoMousePropagation);
}

void
TitleWidget::paintEvent(QPaintEvent *)
{
    QString title(window()->windowTitle());
    const QRect mappedRect(mapFrom(parentWidget(), QPoint()), parentWidget()->contentsRect().size());
    QPainter p(this);
    QFont f(p.font());
    const bool active(Handlers::Window::isActiveWindow(this));
    QPalette pal(palette());
    pal.setCurrentColorGroup(active?QPalette::Active:QPalette::Disabled);
    f.setBold(active);
//    f.setPointSize(f.pointSize()-1.5f);
    p.setFont(f);
    int align(Qt::AlignVCenter);
    switch (dConf.titlePos)
    {
    case Left: align |= Qt::AlignLeft; break;
    case Center: align |= Qt::AlignHCenter; break;
    case Right: align |= Qt::AlignRight; break;
    default: break;
    }
//    Render::renderShadow(dConf.toolbtn.shadow, rect(), &p, dConf.toolbtn.rnd, All, dConf.shadows.opacity);
#if QT_VERSION > 0x050000
//    align = Qt::AlignBottom|Qt::AlignHCenter;
//    QFont fb(p.font());
//    fb.setPointSize(fb.pointSize()-3);
//    fb.setBold(false);
//    QColor c(pal.color(foregroundRole()));
//    c.setAlpha(127);
//    p.setPen(c);
//    p.setFont(fb);
//    p.drawText(rect, Qt::AlignTop|Qt::AlignHCenter, qApp->applicationDisplayName());
//    p.setFont(f);
    if (!title.isEmpty())
        title.append(QString(" - "));
    title.append(qApp->applicationDisplayName());
#endif
    const QString &elidedTitle = p.fontMetrics().elidedText(title, Qt::ElideRight, rect().width()-20);
    QRect tr(p.fontMetrics().boundingRect(mappedRect, align, elidedTitle));
    if (tr.x() < 20)
        tr.moveLeft(20);
    style()->drawItemText(&p, tr, align, pal, true, elidedTitle, foregroundRole());
    if (dConf.deco.icon && !title.isEmpty())
    {
        const QPixmap icon(window()->windowIcon().pixmap(16, active?QIcon::Normal:QIcon::Disabled));
        QRect ir(QPoint(0, mappedRect.height()/2-qMax(1, icon.height())/2), icon.size());
        ir.moveRight(tr.left()-4);
        if (icon.width() && ir.right() >= icon.width())
            style()->drawItemPixmap(&p, ir, Qt::AlignVCenter|Qt::AlignRight, icon);
    }
    p.end();
}

void
TitleWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    e->accept();
    if (WindowData *data = WindowData::memory(window()->winId(), window()))
    if (uint deco = data->decoId())
        XHandler::doubleClickEvent(e->globalPos(), deco, e->button());
}

void
TitleWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        QWidget::mousePressEvent(e);
        XHandler::mwRes(e->pos(), e->globalPos(), window()->winId());
        return;
    }
    if (WindowData *data = WindowData::memory(window()->winId(), window()))
    if (uint deco = data->decoId())
        XHandler::pressEvent(e->globalPos(), deco, e->button());
}

void
TitleWidget::wheelEvent(QWheelEvent *e)
{
    e->accept();
    if (WindowData *data = WindowData::memory(window()->winId(), window()))
    if (uint deco = data->decoId())
        XHandler::wheelEvent(deco, e->delta()>0);
}

bool
TitleWidget::supported(const QToolBar *toolBar)
{
    if (!dConf.deco.embed||!dConf.uno.enabled||!toolBar)
        return false;
    bool hasMacMenu(false);
#if HASDBUS//        if (!qobject_cast<QToolButton *>(w) && w->isVisible())
    //            return false;
        hasMacMenu = BE::MacMenu::isActive();
#endif
    if (!hasMacMenu
            || dConf.app == Settings::SystemSettings
            || toolBar->isFloating()
            || !toolBar->isMovable()
            || toolBar->orientation() == Qt::Vertical
            || !qobject_cast<QMainWindow *>(toolBar->parentWidget())
            || toolBar->width() != toolBar->parentWidget()->width()
            || toolBar->toolButtonStyle() != Qt::ToolButtonIconOnly
            || toolBar->iconSize().height() != 16
            || (toolBar->parentWidget() && toolBar->parentWidget()->parentWidget()))
        return false;

    const QList<QWidget *> kids(toolBar->findChildren<QWidget *>());
    for (int i = 0; i < kids.count(); ++i)
    {
        QWidget *w(kids.at(i));
//        qDebug() << w;
        if (/*!w->inherits("QToolBarSeparator") || w->inherits("QToolBarExtension") ||*/ qobject_cast<TitleWidget *>(w) || w->width() < 128)
            continue;
        if (!qobject_cast<QToolButton *>(w) && w->isVisible())
            return false;
    }

//    const QList<QAction *> actions(toolBar->actions());
//    for (int i = 0; i < actions.count(); ++i)
//    {
//        QAction *a(actions.at(i));
//        if (a->isSeparator() || a->objectName() == s_spacerName)
//            continue;
//        QWidget *w(toolBar->widgetForAction(actions.at(i)));
//        qDebug() << w;
//        if (!w || qobject_cast<TitleWidget *>(w) || w->width() < 192)
//            continue;
//        if (!qobject_cast<QToolButton *>(w) && w->isVisible())
//            return false;
//    }
    return true;
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
    {
        s_instance = new ToolBar();
#if HASDBUS
        if (dConf.deco.embed && BE::MacMenu::instance())
            connect(BE::MacMenu::instance(), SIGNAL(activeChanged()), s_instance, SLOT(macMenuChanged()));
#endif
    }
    return s_instance;
}

void
ToolBar::manage(QWidget *child)
{
    if (!qobject_cast<QToolBar *>(child->parentWidget()))
        return;
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

    if (qobject_cast<QMainWindow *>(tb->parentWidget()))
    {
        connect(tb, SIGNAL(topLevelChanged(bool)), instance(), SLOT(toolBarFloatingChagned(bool)));
        connect(tb, SIGNAL(orientationChanged(Qt::Orientation)), instance(), SLOT(toolBarOrientationChagned(Qt::Orientation)));
        connect(tb, SIGNAL(movableChanged(bool)), instance(), SLOT(toolBarMovableChanged(bool)));
        connect(tb, SIGNAL(visibilityChanged(bool)), instance(), SLOT(toolBarVisibilityChanged(bool)));
        connect(tb, SIGNAL(destroyed(QObject*)), instance(), SLOT(toolBarDeleted(QObject*)));
    }
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
ToolBar::macMenuChanged()
{
#if HASDBUS
    const QList<QWidget *> allWidgets(qApp->allWidgets());
    for (int i = 0; i < allWidgets.count(); ++i)
        if (QMainWindow *win = qobject_cast<QMainWindow *>(allWidgets.at(i)))
        {
            const QList<QToolBar *> toolBars = win->findChildren<QToolBar *>();
            for (int t = 0; t < toolBars.count(); ++t)
            {
                if (BE::MacMenu::isActive() && dConf.deco.embed)
                    embedTitleWidgetLater(toolBars.at(t));
                else
                    unembed(toolBars.at(t));
            }
        }
#endif
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
ToolBar::toolBarOrientationChagned(const Qt::Orientation o)
{
    QToolBar *toolBar = static_cast<QToolBar *>(sender());
    adjustMargins(toolBar);
}

void
ToolBar::toolBarFloatingChagned(const bool floating)
{
    QToolBar *toolBar = static_cast<QToolBar *>(sender());
    if (floating)
        unembed(toolBar);
    adjustMargins(toolBar);
}

void
ToolBar::toolBarMovableChanged(const bool movable)
{
    QToolBar *toolBar = static_cast<QToolBar *>(sender());
    if (!movable)
        unembed(toolBar);
    else
        embedTitleWidgetLater(toolBar);
    adjustMargins(toolBar);
    Window::updateWindowDataLater(toolBar->window());
}

void
ToolBar::toolBarVisibilityChanged(const bool visible)
{
    QToolBar *toolBar = static_cast<QToolBar *>(sender());
    if (visible)
    {
        adjustMargins(toolBar);
        s_dirty.insert(toolBar, true);
        if (dConf.deco.embed)
            embedTitleWidgetLater(toolBar);
    }
    else if (!toolBar->isVisibleTo(toolBar->parentWidget()))
        unembed(toolBar);
    Window::updateWindowDataLater(toolBar->window());
}

void
ToolBar::adjustMargins(QToolBar *toolBar)
{
    if (!toolBar)
        return;

    if (toolBar->isFloating())
    {
        toolBar->setMovable(true);
        toolBar->setContentsMargins(0, 0, 0, 0);
        toolBar->layout()->setContentsMargins(0, 0, 0, 0);
        return;
    }

    QMainWindow *win = qobject_cast<QMainWindow *>(toolBar->parentWidget());
    if (!win || !toolBar->layout() || toolBar->actions().isEmpty() || win->toolBarArea(toolBar) != Qt::TopToolBarArea)
        return;

    if (toolBar->geometry().top() <= win->rect().top())
    {
        toolBar->layout()->setContentsMargins(0, 0, 0, 0);
        WindowData *d = WindowData::memory(win->winId(), win);
        int m(0);
        if (d)
            m = d->value<uint>(WindowData::RightEmbedSize, 0);
        toolBar->QWidget::setContentsMargins(0, 0, toolBar->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent)+m, 6);
    }
    else if (toolBar->findChild<QTabBar *>()) //sick, put a tabbar in a toolbar... eiskaltdcpp does this :)
    {
        toolBar->layout()->setContentsMargins(0, 0, 0, 0);
        toolBar->setMovable(false);
    }
    else
        toolBar->layout()->setContentsMargins(2, 2, 2, 2);
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

static QList<qulonglong> s_toolBarQuery[2];

void
ToolBar::queryToolBarLater(QToolBar *bar, bool forceSizeUpdate)
{
    const qulonglong tb = (qulonglong)bar;
    if (!s_toolBarQuery[forceSizeUpdate].contains(tb))
    {
        s_toolBarQuery[forceSizeUpdate] << tb;
        QMetaObject::invokeMethod(instance(), "queryToolBar", Qt::QueuedConnection, Q_ARG(qulonglong,tb), Q_ARG(bool,forceSizeUpdate));
    }
}

void
ToolBar::queryToolBar(qulonglong toolbar, bool forceSizeUpdate)
{
    s_toolBarQuery[forceSizeUpdate].removeOne(toolbar);
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
                Sides sides(All);
                if (i && qobject_cast<QToolButton *>(bar->widgetForAction(actions.at(i-1))))
                    sides &= ~(bar->orientation() == Qt::Horizontal?Left:Top);
                if (i+1 < actions.count())
                {
                    QAction *right(actions.at(i+1));
                    if (qobject_cast<QToolButton *>(bar->widgetForAction(right)))
                        sides &= ~(bar->orientation() == Qt::Horizontal?Right:Bottom);
                }
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
    }
    s_dirty.insert(bar, false);
    bar->installEventFilter(this);
}

Sides
ToolBar::sides(const QToolButton *btn)
{
    if (btn)
    if (QToolBar *bar = qobject_cast<QToolBar *>(btn->parentWidget()))
        if (isDirty(bar))
                queryToolBarLater(bar, true);
    return s_sides.value(const_cast<QToolButton *>(btn), All);
}

class RedWidget : public QWidget
{
public:
    RedWidget(QWidget *parent) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent *e)
    {
        QPainter p(this);
        p.fillRect(rect(), Qt::red);
        p.end();
    }
};

//static QList<QPair<qulonglong, int> > s_spacerQueue;

void
ToolBar::fixSpacerLater(QToolBar *toolbar, int width)
{
    const qulonglong tb = (qulonglong)toolbar;
//    if (!s_spacerQueue.contains(QPair<qulonglong, int>(tb, width)))
//    {
//        s_spacerQueue << QPair<qulonglong, int>(tb, width);
        QMetaObject::invokeMethod(instance(), "fixSpacer", Qt::QueuedConnection, Q_ARG(qulonglong, tb), Q_ARG(int, width));
//    }
}

void
ToolBar::fixSpacer(qulonglong toolbar, int width)
{
//    s_spacerQueue.removeOne(QPair<qulonglong, int>(toolbar, width));
    QToolBar *tb = getChild<QToolBar *>(toolbar);
    if (!tb
            || !qobject_cast<QMainWindow *>(tb->parentWidget())
            || tb->findChild<QTabBar *>()
            || !tb->styleSheet().isEmpty())
        return;

//    QToolBar *sp = tb->parentWidget()->findChild<QToolBar *>("DSP_SPACERBAR");
//    if (!sp)
//    {
//        sp = new QToolBar(tb->parentWidget());
//        sp->setObjectName("DSP_SPACERBAR");
//        sp->setMovable(false);
//    }
//    sp->setFixedWidth(80);
//    static_cast<QMainWindow *>(tb->parentWidget())->insertToolBarBreak(tb);
//    return;
    tb->removeEventFilter(this);
    if (tb->isMovable() && width == 7)
    {
        if (QAction *spacer = tb->findChild<QAction *>(s_spacerName))
            spacer->deleteLater();
        return;
    }
    QAction *spacer = tb->findChild<QAction *>(s_spacerName);
    if (!spacer)
    {
        QWidget *w = new QWidget(tb);
        spacer = tb->insertWidget(tb->actions().first(), w);
        spacer->setObjectName(s_spacerName);
    }
    tb->widgetForAction(spacer)->setFixedSize(width, 7);
    tb->removeAction(spacer);
    tb->insertAction(tb->actions().first(), spacer);
    spacer->setVisible(!tb->isMovable()||width>7);
    tb->installEventFilter(this);
}

void
ToolBar::unembed(QToolBar *bar)
{
    if (!bar)
        return;
    if (QAction *spacer = bar->findChild<QAction *>(s_spacerName))
    {
        static_cast<QWidgetAction *>(spacer)->defaultWidget()->setFixedSize(1, 1);
        spacer->deleteLater();
        if (WindowData *data = WindowData::memory(bar->window()->winId(), bar->window()))
        {
            data->setValue<bool>(WindowData::EmbeddedButtons, false);
            data->setValue<uint>(WindowData::TitleHeight, 25);
            data->sync();
        }
    }
    if (QWidget *w = bar->findChild<TitleWidget *>())
        w->deleteLater();
    if (!bar->isMovable())
        instance()->fixSpacer(reinterpret_cast<qulonglong>(bar));
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
            if (e->type() == QEvent::ShowToParent)
            {
                if (tb->findChild<TitleWidget *>())
                    embedTitleWidgetLater(tb);
                fixSpacerLater(tb);
            }
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
            fixSpacerLater(tb);
        }
        else if (QToolButton *tbn = qobject_cast<QToolButton *>(o))
        {
            QToolBar *tb = qobject_cast<QToolBar *>(tbn->parentWidget());
            if (tb)
                s_dirty.insert(tb, true);
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
        if (tbn && Ops::hasMenu(tbn))
            checkForArrowPress(tbn, static_cast<QMouseEvent *>(e)->pos());
        return false;
    }
    case QEvent::Resize:
    {
        QResizeEvent *re(static_cast<QResizeEvent *>(e));
        QToolBar *tb = qobject_cast<QToolBar *>(o);
        if (tb && dConf.uno.enabled && re->oldSize().height() != re->size().height())
            Window::updateWindowDataLater(tb->window());
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


static QWidget *getCenterWidget(QToolBar *bar)
{
    QLayout *l = bar->layout();
    const int center(qMin(l->count()-1, qCeil(l->count()/2.0f)+2));
    QLayoutItem *centerItem = l->itemAt(center);
    if (centerItem->spacerItem())
        centerItem = l->itemAt(center+1);
    QWidget *w = centerItem->widget();

    for (int i = center; i; --i)
    {
        QWidget *wi = l->itemAt(i)->widget();
        if (wi->isHidden())
            continue;
        if (wi->inherits("QToolBarSeparator") && qobject_cast<QToolButton *>(w))
            break;
        else if (qobject_cast<QToolButton *>(wi))
            w = wi;
    }
    return w;
}

static QList<qulonglong> s_titleQueue;

void
ToolBar::embedTitleWidgetLater(QToolBar *toolBar)
{
    if (!s_titleQueue.contains((qulonglong)toolBar))
    {
        s_titleQueue << (qulonglong)toolBar;
        QMetaObject::invokeMethod(instance(), "embedTitleWidget", Qt::QueuedConnection, Q_ARG(qulonglong, (qulonglong)toolBar));
    }
}

void
ToolBar::embedTitleWidget(qulonglong bar)
{

    s_titleQueue.removeOne(bar);
    QToolBar *toolBar = getChild<QToolBar *>(bar);
    if (toolBar && toolBar->styleSheet().isEmpty())
    {
        toolBar->blockSignals(true);
        toolBar->setMovable(true);
        toolBar->blockSignals(false);
    }

    if (!toolBar
            || !TitleWidget::supported(toolBar)
            || toolBar->layout()->count()<3)
    {
        if (toolBar)
            unembed(toolBar);
        return;
    }


    WindowData *data = WindowData::memory(toolBar->window()->winId(), toolBar->window());
    if (!data)
        return;

    data->setValue<bool>(WindowData::EmbeddedButtons, true);
    data->setValue<uint>(WindowData::TitleHeight, 6);
    toolBar->removeEventFilter(instance());
    QAction *a(0);
    if (dConf.titlePos == TitleWidget::Center)
    {
        if (QWidget *w = getCenterWidget(toolBar))
            a = toolBar->actionAt(w->geometry().center());
        else
            a = toolBar->actions().at(1);
    }
    else if (dConf.titlePos == TitleWidget::Left)
        a = toolBar->actions().at(1);

    TitleWidget *title = toolBar->findChild<TitleWidget *>();
    if (!title)
        title = new TitleWidget(toolBar);

    bool isAdded(false);
    if (QWidgetAction *action = qobject_cast<QWidgetAction *>(toolBar->actionAt(title->geometry().center())))
        if (qobject_cast<TitleWidget *>(action->defaultWidget()))
        {
            isAdded = true;
            toolBar->removeAction(toolBar->actionAt(title->geometry().center()));
            toolBar->insertAction(a, action);
        }
    if (!isAdded)
        toolBar->insertWidget(a, title);
    title->show();
    fixSpacer(bar, data->value<uint>(WindowData::LeftEmbedSize));
    data->sync();
    toolBar->installEventFilter(instance());
}

//-------------------------------------------------------------------------------------------------

Window *Window::s_instance;
QMap<QWidget *, Handlers::Data> Window::s_unoData;

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

bool
Window::isActiveWindow(const QWidget *w)
{
    if (!w)
        return true;
    QWidget *win = w->window();
    if (!win->property(s_active).isValid())
        return true;
    return (win->property(s_active).toBool());
}

void
Window::dataChanged(QDBusMessage msg)
{
    uint winId = msg.arguments().at(0).toUInt();
    QList<QWidget *> widgets = qApp->allWidgets();
    for (int i = 0; i < widgets.count(); ++i)
    {
        QWidget *w(widgets.at(i));
        if (w->isWindow() && w->winId() == winId)
        {
            WindowData *data = WindowData::memory(winId, w);
            w->setProperty(s_active, data->value<bool>(WindowData::IsActiveWindow));
            if (dConf.deco.embed)
            {
                QList<QToolBar *> toolBars(w->findChildren<QToolBar *>());
                for (int i = 0; i < toolBars.count(); ++i)
                {
                    if (TitleWidget::supported(toolBars.at(i)))
                        ToolBar::embedTitleWidgetLater(toolBars.at(i));
                    ToolBar::adjustMargins(toolBars.at(i));
                }
            }
            updateWindowDataLater(/*(qulonglong)*/w);
            break;
        }
    }
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
//            Sides sides(All);
            if (dConf.uno.enabled)
            {
                if (XHandler::opacity() < 1.0f
                        && qobject_cast<QMainWindow *>(w))
                    p.setClipRegion(paintRegion(static_cast<QMainWindow *>(w)));
//                p.fillRect(w->rect(), bgColor);
                if (dConf.windows.noise)
                    p.drawTiledPixmap(w->rect(), GFX::noise(true));
                else
                    p.fillRect(w->rect(), bgColor);
            }
            else if (dConf.windows.noise)
            {
                p.drawTiledPixmap(w->rect(), GFX::noise(true));
            }
            else
                p.fillRect(w->rect(), bgColor);
            if (WindowData *data = WindowData::memory(w->winId(), w))
                if (!data->value<bool>(WindowData::Separator, true) && data->value<bool>(WindowData::Uno))
                {
                    p.setPen(QColor(0, 0, 0, dConf.shadows.opacity));
                    p.drawLine(0, Handlers::unoHeight(w, ToolBars)+0.5f, w->width(), Handlers::unoHeight(w, ToolBars)+0.5f);
                }

            p.end();
        }
        return false;
    }
    case QEvent::Show:
    {
        if (w->isWindow())
        {
            if (!w->property(s_active).isValid())
            {
                if (WindowData *d = WindowData::memory(w->winId(), w))
                    d->setValue<bool>(WindowData::IsActiveWindow, true);
                w->setProperty(s_active, true);
            }
            updateWindowDataLater(w);
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
            updateWindowDataLater(w);
        return false;
    }
    case QEvent::WindowTitleChange:
    {
        if (TitleWidget *w = o->findChild<TitleWidget *>())
            w->update();
    }
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

    if (WindowData *data = WindowData::memory(win->winId(), win))
    {
        offset.ry()+= data->value<int>(WindowData::TitleHeight, 0);
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
    return false;
}

unsigned int
Window::getHeadHeight(QWidget *win, bool &separator)
{
    WindowData *d = WindowData::memory(win->winId(), win); //guaranteed by caller method
    int h = d->value<int>(WindowData::TitleHeight);
    if (!h)
        h = 25;
    if (!h)
    {
        separator = false;
        return 0;
    }
    if (!dConf.uno.enabled)
    {
        separator = false;
        if (h)
            return h;
        return 0;
    }
    int hd[HeightCount];
    hd[TitleBar] = h;
    hd[Head] = hd[TitleBar];
    hd[ToolBars] = hd[ToolBarAndTabBar] = 0;
    if (QMainWindow *mw = qobject_cast<QMainWindow *>(win))
    {
        if (QMenuBar *menuBar = mw->findChild<QMenuBar *>())
#if HASDBUS
            if (!BE::MacMenu::isActive() && !BE::MacMenu::manages(menuBar))
#endif
        {
            if (menuBar->isVisible())
                hd[Head] += menuBar->height();
        }

        QList<QToolBar *> tbs(mw->findChildren<QToolBar *>());
        for (int i = 0; i<tbs.count(); ++i)
        {
            QToolBar *tb(tbs.at(i));
            if (!tb->isFloating() && tb->isVisible())
                if (!tb->findChild<QTabBar *>()) //ah eiskalt...
                    if (mw->toolBarArea(tb) == Qt::TopToolBarArea)
                    {
                        if (tb->geometry().bottom() > hd[ToolBars])
                            hd[ToolBars] = tb->geometry().bottom()+1;
                        separator = false;
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
            separator = false;
            const int y(tb->mapTo(win, tb->rect().bottomLeft()).y());
            if (y > hd[ToolBarAndTabBar])
                hd[ToolBarAndTabBar] = y;
        }
    }
    hd[Head] += hd[ToolBarAndTabBar];
    s_unoData.insert(win, Data(hd, possible));
    return hd[Head];
}

void
Window::unoBg(QWidget *win, int &w, int h, const QPalette &pal, uchar *data)
{
    if (!data || !win || !h)
        return;
    const bool hor(dConf.uno.hor);
    QColor bc(pal.color(win->backgroundRole()));
    if (dConf.uno.tint.first.isValid())
        bc = Color::mid(bc, dConf.uno.tint.first, 100-dConf.uno.tint.second, dConf.uno.tint.second);
    QBrush b(bc);
    if (!dConf.uno.gradient.isEmpty())
    {
        QLinearGradient lg(0, 0, hor?win->width():0, hor?0:h);
        if (dConf.differentInactive && !isActiveWindow(win))
            lg.setStops(QGradientStops()
                        << DSP::Settings::pairToStop(DSP::GradientStop(0.0f, 0), bc)
                        << DSP::Settings::pairToStop(DSP::GradientStop(1.0f, 0), bc));
        else
            lg.setStops(DSP::Settings::gradientStops(dConf.uno.gradient, bc));
        b = QBrush(lg);
    }
    const unsigned int n(dConf.uno.noise);
    w = (hor?win->width():(n?GFX::noise().width():1));
    QImage img(data, w, h, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter pt(&img);
    pt.fillRect(img.rect(), b);
    if (n)
    {
        const QPixmap noise = FX::mid(GFX::noise(), b, n, 100-n, img.size());
//        QPixmap noise(GFX::noise().size());
//        noise.fill(Qt::transparent);
//        QPainter ptt(&noise);
//        ptt.drawTiledPixmap(noise.rect(), GFX::noise());
//        ptt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
//        ptt.fillRect(noise.rect(), QColor(0, 0, 0, n*2.55f));
//        ptt.end();
//        pt.setCompositionMode(QPainter::CompositionMode_Overlay);
        pt.drawTiledPixmap(img.rect(), noise);
    }
    pt.end();

    if (XHandler::opacity() < 1.0f)
    {
        QImage copy(img);
        img.fill(Qt::transparent);
        pt.begin(&img);
        pt.drawImage(img.rect(), copy);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pt.fillRect(img.rect(), QColor(0, 0, 0, XHandler::opacity()*255.0f));
        pt.end();
    }
}

QImage
Window::windowBg(const QSize &sz, const QColor &bgColor)
{
    int w = dConf.windows.hor&&!dConf.windows.gradient.isEmpty()?sz.width() : dConf.windows.noise?GFX::noise().width() : 1;
    int h = !dConf.windows.hor&&!dConf.windows.gradient.isEmpty()?sz.height() : dConf.windows.noise?GFX::noise().height() : 1;
    QImage img(w, h, QImage::Format_ARGB32);
    if (XHandler::opacity() < 1.0f)
        img.fill(Qt::transparent);

    QPainter pt(&img);
    if (!dConf.windows.gradient.isEmpty())
    {
        QLinearGradient lg(0, 0, dConf.windows.hor*img.width(), !dConf.windows.hor*img.height());
        lg.setStops(DSP::Settings::gradientStops(dConf.windows.gradient, bgColor));
        pt.fillRect(img.rect(), lg);
    }
    else
        pt.fillRect(img.rect(), bgColor);

    if (dConf.windows.noise)
    {
        static QPixmap noisePix;
        if (noisePix.isNull())
        {
            noisePix = QPixmap(GFX::noise().size());
            noisePix.fill(Qt::transparent);
            QPainter p(&noisePix);
            p.drawPixmap(noisePix.rect(), GFX::noise());
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.fillRect(noisePix.rect(), QColor(0, 0, 0, dConf.windows.noise*2.55f));
            p.end();
        }
        pt.setCompositionMode(QPainter::CompositionMode_Overlay);
        pt.drawTiledPixmap(img.rect(), noisePix);
    }

    if (XHandler::opacity() < 1.0f)
    {
        pt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pt.fillRect(img.rect(), QColor(0, 0, 0, dConf.opacity*255.0f));
        pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
    pt.end();
    return img;
}

static QList<qulonglong> s_scheduledWindows;

void
Window::updateWindowDataLater(QWidget *win)
{
    const qulonglong window = (qulonglong)win;
    if (!s_scheduledWindows.contains(window))
    {
        s_scheduledWindows << window;
        QMetaObject::invokeMethod(instance(), "updateWindowData", Qt::QueuedConnection, Q_ARG(qulonglong, window));
    }
}

//class Red2Widget : public QWidget
//{
//public:
//    Red2Widget(uint p, QWidget *win) : QWidget(0), parent(p)
//    {
//        setFixedSize(72, 8);
//        QWindow *window = QWindow::fromWinId(winId());
//        QWindow *deco = QWindow::fromWinId(parent);
//        QWidget *shit = QWidget::createWindowContainer(window);
//        shit->setFixedSize(72, 8);
//        shit->show();
//        show();
//    }
//protected:
//    void paintEvent(QPaintEvent *e)
//    {
//        QPainter p(this);
//        p.fillRect(rect(), Qt::red);
//        p.end();
//    }
//private:
//    uint parent;
//};

void
Window::updateWindowData(qulonglong window)
{
    s_scheduledWindows.removeOne(window);
    QWidget *win = getChild<QWidget *>(window);
    if (!win || !win->isWindow())
        return;

    WindowData *data = WindowData::memory(win->winId(), win, true);
    if (!data)
        return;

    bool separator(true);
    const unsigned int height(getHeadHeight(win, separator));
    QPalette pal(win->palette());
    if (!Color::contrast(pal.color(win->backgroundRole()), pal.color(win->foregroundRole()))) //im looking at you spotify
        pal = QApplication::palette();

    if (dConf.differentInactive)
    {
        if (isActiveWindow(win))
        {
            pal.setColor(QPalette::Active, win->backgroundRole(), Color::mid(pal.color(win->backgroundRole()), Qt::black, 10, 1));
            pal.setColor(QPalette::Active, win->foregroundRole(), Color::mid(pal.color(win->foregroundRole()), Qt::black, 10, 1));
            pal.setCurrentColorGroup(QPalette::Active);
        }
        else
        {
            pal.setColor(QPalette::Inactive, win->backgroundRole(), Color::mid(pal.color(win->backgroundRole()), Qt::white, 20, 1));
            pal.setColor(QPalette::Inactive, win->foregroundRole(), Color::mid(pal.color(win->foregroundRole()), Qt::white, 20, 1));
            pal.setCurrentColorGroup(QPalette::Inactive);
        }
//        win->setPalette(pal);
    }


//    static bool shown(false);
//    if (!shown)
//    {
//        if (uint deco = data->decoId())
//        {
//            static Red2Widget *l = 0;
//            if (!l)
//                l = new Red2Widget(deco, win);
//        }
//    }


    if (dConf.uno.enabled && height)
    {
        int width(0);
        if (data->lock())
        {

            unoBg(win, width, height, pal, data->imageData());
            data->setImageSize(width, height);
            data->unlock();
        }
    }
    data->setValue<bool>(WindowData::Separator, separator);
    data->setValue<bool>(WindowData::WindowIcon, dConf.deco.icon);
    data->setValue<bool>(WindowData::ContAware, dConf.uno.enabled&&dConf.uno.contAware);
    data->setValue<bool>(WindowData::Uno, dConf.uno.enabled);
    data->setValue<bool>(WindowData::Horizontal, dConf.uno.enabled?dConf.uno.hor:dConf.windows.hor);
    data->setValue<int>(WindowData::Opacity, XHandler::opacity()*255.0f);
    data->setValue<int>(WindowData::UnoHeight, height);
    data->setValue<int>(WindowData::Buttons, dConf.deco.buttons);
    data->setValue<int>(WindowData::Frame, dConf.deco.frameSize);
    data->setValue<int>(WindowData::ShadowOpacity, dConf.shadows.opacity);
    data->setFg(pal.color(win->foregroundRole()));
    data->setBg(pal.color(win->backgroundRole()));
    win->update();
    if (TitleWidget *w = win->findChild<TitleWidget *>())
        w->update();
    emit instance()->windowDataChanged(win);
    data->sync();
}

//-------------------------------------------------------------------------------------------------

Drag Drag::s_instance;

//QCoreApplication::EventFilter oldFilter = 0;
//static bool isFiltering = false;

//bool dragWatcher(void *message, long *result)
//{
//    if (XEvent *xe = static_cast<XEvent *>(message))
//    {
//        qDebug() << xe
//    }
//    return false;
//}

//Drag::Drag(QObject *parent) : QObject(parent)
//{
//    if (!isFiltering)
//    {
//        oldFilter = qApp->setEventFilter(dragWatcher);
//        isFiltering = true;
//    }
//}

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
    if (WindowData *wd = WindowData::memory(win->winId(), win))
        wd->sync();
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
    const int uno(unoHeight(win, Head)), titleHeight(unoHeight(win, TitleBar));;
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
            FX::expblur(img, dConf.uno.blur);

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
        p->drawImage(QPoint(0, 0), QImage(data, w->width(), unoHeight(w, Head), QImage::Format_ARGB32_Premultiplied), r.translated(offset));
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
                || pe->rect().top() > unoHeight(w->window(), Head))
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
static unsigned long pix[8];

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
    unsigned long data[12];
    genPixmaps();
    for (int i = 0; i < 8; ++i)
        data[i] = pix[i];
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
    QMetaObject::invokeMethod(instance(), "lockWindowLater", Qt::QueuedConnection, Q_ARG(QWidget *, win));
}

void
Dock::lockWindowLater(QWidget *win)
{
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
