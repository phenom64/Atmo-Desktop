#include <QTableWidget>
#include <QPainter>
#include <QStyleOptionTabWidgetFrameV2>
#include <QMainWindow>
#include <QToolBar>
#include <QLayout>
#include <QAction>
#include <QResizeEvent>
#include <QMenu>
#include <QMenuBar>
#include <QCheckBox>
#include <QLabel>
#include <QApplication>
#include <QToolButton>
#include <QTimer>
#include <QStatusBar>
#include "styleproject.h"
#include "stylelib/xhandler.h"
#include "stylelib/ops.h"
#include "stylelib/unohandler.h"
#include "stylelib/settings.h"

bool
StyleProject::eventFilter(QObject *o, QEvent *e)
{
    if (!e || !o)
        return false;
    if (e->type() < EVSize && m_ev[e->type()])
        return (this->*m_ev[e->type()])(o, e);

    switch (e->type())
    {
//    case QEvent::Show:
//    case QEvent::Leave:
//    case QEvent::HoverLeave:
//    case QEvent::Enter:
//    case QEvent::HoverEnter:
    case QEvent::ActionChanged:
    {
        if (castObj(QToolBar *, toolBar, o))
            toolBar->update();
        break;
    }
    default: break;
    }
    return QCommonStyle::eventFilter(o, e);
}

static void paintTab(QTabBar *tb, const int i, QStyle *style, QPainter &p)
{
    if (!tb->tabRect(i).isValid())
        return;
    const bool isFirst(i==0), isLast(i==tb->count()-1), isSelected(i==tb->currentIndex());
    const int ol(style->pixelMetric(QStyle::PM_TabBarTabOverlap, 0, tb));
    QStyleOptionTabV3 tab;
    tab.initFrom(tb);
    if (isSelected)
        tab.state |= QStyle::State_Selected;
    tab.rect = tb->tabRect(i).adjusted(isFirst?0:-ol, 0, isLast?0:ol, 0);
    tab.text = tb->tabText(i);
    tab.icon = tb->tabIcon(i);
    tab.iconSize = tb->iconSize();
    if (QWidget *w = tb->tabButton(i, QTabBar::RightSide))
    {
        tab.rightButtonSize = w->size();
        tab.cornerWidgets |= QStyleOptionTab::RightCornerWidget;
    }
    if (QWidget *w = tb->tabButton(i, QTabBar::LeftSide))
    {
        tab.leftButtonSize = w->size();
        tab.cornerWidgets |= QStyleOptionTab::LeftCornerWidget;
    }

    if (!tb->isEnabled())
        tab.palette.setCurrentColorGroup(QPalette::Disabled);

    style->drawControl(QStyle::CE_TabBarTab, &tab, &p, tb);
}

bool
StyleProject::paintEvent(QObject *o, QEvent *e)
{
    /* for some reason KTabWidget is an idiot and
     * doesnt use the style at all for painting, only
     * for the tabBar apparently (that inside the
     * KTabWidget) so we simply override the painting
     * for the KTabWidget and paint what we want
     * anyway.
     */
    if (o->inherits("KTabWidget"))
    {
        QTabWidget *tabWidget = static_cast<QTabWidget *>(o);
        QPainter p(tabWidget);
        QStyleOptionTabWidgetFrameV2 opt;
        opt.initFrom(tabWidget);
        drawTabWidget(&opt, &p, tabWidget);
        p.end();
        return true;
    }
    else if (o->objectName() == "konsole_tabbar_parent")
    {
        QWidget *w = static_cast<QWidget *>(o);
        QStyleOptionTabBarBaseV2 opt;
        opt.rect = w->rect();
        QTabBar *tb = w->findChild<QTabBar *>();
        if (!tb)
            return false;
        opt.rect.setHeight(tb->height());
        QPainter p(w);
        drawTabBar(&opt, &p, tb);
        p.end();
        return true;
    }
    else if (QMainWindow *win = qobject_cast<QMainWindow *>(o))
    {
        if (int th = win->property("DSP_headHeight").toInt())
        {
            QPainter p(win);
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
                    if (bar->parentWidget() == win)
                        r -= QRegion(bar->geometry());
                    else
                        r -= QRegion(QRect(bar->mapTo(win, bar->rect().topLeft()), bar->size()));

//                    QList<QWidget *> kids(bar->findChildren<QWidget *>());
//                    for (int i = 0; i < kids.count(); ++i)
//                    {
//                        QWidget *kid(kids.at(i));
//                        r -= QRegion(QRect(kid->mapTo(win, kid->rect().topLeft()), kid->size()));
//                    }
                }
            p.setClipRegion(r);
            p.fillRect(win->rect(), win->palette().color(win->backgroundRole()));
            p.setPen(Qt::black);
            p.setOpacity(Settings::conf.shadows.opacity);
            p.drawLine(0, th, win->width(), th);
            p.setOpacity(1.0f);
            p.end();
            return false;
        }
    }
    return QCommonStyle::eventFilter(o, e);
}

bool
StyleProject::resizeEvent(QObject *o, QEvent *e)
{
//    QResizeEvent *re = static_cast<QResizeEvent *>(e);
//    if (castObj(QTabBar *, tb, o))
//    {
//        QToolButton *addTab = tb->findChild<QToolButton *>("tab-new");
//        QToolButton *closeTab = tb->findChild<QToolButton *>("tab-close");
//        if (!addTab || !closeTab)
//            return false;

//        int x, y, w, h;
//        tb->rect().getRect(&x, &y, &w, &h);
//        int cy = y+(h/2-addTab->height()/2);
//        addTab->move(x, cy);
//        closeTab->move(w-closeTab->width(), cy);
//    }
    return QCommonStyle::eventFilter(o, e);
}

bool
StyleProject::showEvent(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QCommonStyle::eventFilter(o, e);
    QWidget *w = static_cast<QWidget *>(o);
    if ((qobject_cast<QMenuBar*>(o)||qobject_cast<QMenu *>(o)))
    {
        static_cast<QWidget *>(o)->setMouseTracking(true);
        static_cast<QWidget *>(o)->setAttribute(Qt::WA_Hover);
    }
    if (w->inherits("KTitleWidget"))
    {
        QList<QLabel *> children = w->findChildren<QLabel *>();
        for (int i = 0; i < children.size(); ++i)
            children.at(i)->setAlignment(Qt::AlignCenter);
    }
    if (castObj(QToolButton *, toolButton, o))
    {
        castObj(QToolBar *, toolBar, toolButton->parentWidget());
        if ( !toolBar )
            return false;
        //below simply to trigger an event that forces the toolbar to call sizeFromContents again
        Ops::queToolBar(toolBar);
        QTimer::singleShot(0, Ops::instance(), SLOT(updateToolBar()));
    }
    if (qobject_cast<QMainWindow *>(o))
    {
        Ops::callLater(static_cast<QWidget *>(o), &QWidget::update);
        unsigned int d(1);
        XHandler::setXProperty<unsigned int>(static_cast<QWidget *>(o)->winId(), XHandler::KwinBlur, XHandler::Long, &d);
    }
//    if (castObj(QTabBar *, bar, o))
//    {
//        if (Ops::isSafariTabBar(bar))
//            bar->setStyleSheet("QTabBar::tab::first{ margin-left: 16px; }");
//    }
    return QCommonStyle::eventFilter(o, e);
}
