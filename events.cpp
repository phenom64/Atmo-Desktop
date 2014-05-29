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
#include "styleproject.h"
#include "stylelib/xhandler.h"
#include "stylelib/ops.h"

bool
StyleProject::eventFilter(QObject *o, QEvent *e)
{
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
    const int ol(style->pixelMetric(QStyle::PM_TabBarTabOverlap));
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
    return QCommonStyle::eventFilter(o, e);
}

bool
StyleProject::resizeEvent(QObject *o, QEvent *e)
{
//    QResizeEvent *re = static_cast<QResizeEvent *>(e);
    if (castObj(QWidget *, w, o))
        if (w->isWindow())
            Ops::fixWindowTitleBar(w);
//            fixTitleLater(w);
    if (castObj(QToolBar *, toolBar, o))
        if (castObj(QMainWindow *, win, toolBar->parentWidget()))
            updateToolBar(toolBar);
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
    castObj(QWidget *, w, o);
    if (!w)
        return QCommonStyle::eventFilter(o, e);
    if (w->isWindow())
        fixTitleLater(w);
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
//    if (castObj(QTabBar *, bar, o))
//    {
//        if (Ops::isSafariTabBar(bar))
//            bar->setStyleSheet("QTabBar::tab::first{ margin-left: 16px; }");
//    }
    return QCommonStyle::eventFilter(o, e);
}
