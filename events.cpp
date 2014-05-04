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
    else if (castObj(QTabBar *, tb, o))
    {
#if 0
        if (!Ops::isSafariTabBar(tb))
            return false;

        QPainter p(tb);
        QStyleOptionTabBarBaseV2 optTabBase;
        optTabBase.initFrom(tb);
        optTabBase.tabBarRect = tb->rect();
        if (QTabWidget *tbw = qobject_cast<QTabWidget *>(tb->parentWidget()))
        {
            optTabBase.tabBarRect.setWidth(tbw->width());
            optTabBase.tabBarRect.moveTopLeft(tbw->rect().topLeft());
        }
        optTabBase.selectedTabRect = tb->tabRect(tb->currentIndex());
        if (tb->drawBase())
            drawPrimitive(PE_FrameTabBarBase, &optTabBase, &p, tb);
        for (int i = 0; i < tb->currentIndex(); ++i)
            paintTab(tb, i, this, p);
        for (int i = tb->count(); i >= tb->currentIndex(); --i)
            paintTab(tb, i, this, p);
        p.end();
        return true;
#endif
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
    return QCommonStyle::eventFilter(o, e);
}
