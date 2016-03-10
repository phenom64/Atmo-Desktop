#include "toolbarhelpers.h"
#include "ops.h"
#include <QToolBar>
#include <QAction>
#include <QLayout>
#include <QMainWindow>
#include "windowdata.h"
#include <QWidgetAction>

using namespace DSP;

ToolbarHelpers *ToolbarHelpers::s_instance(0);

static const char *s_spacerName("DSP_TOOLBARSPACER");

ToolbarHelpers
*ToolbarHelpers::instance()
{
    if (!s_instance)
        s_instance = new ToolbarHelpers();
    return s_instance;
}

ToolbarHelpers::ToolbarHelpers(QObject *parent)
    : QObject(parent)
{

}

static QList<QPair<qulonglong, int> > s_spacerQueue;

void
ToolbarHelpers::fixSpacerLater(QToolBar *toolbar, int width)
{
    const qulonglong tb = (qulonglong)toolbar;
    if (!s_spacerQueue.contains(QPair<qulonglong, int>(tb, width)))
    {
        s_spacerQueue << QPair<qulonglong, int>(tb, width);
        QMetaObject::invokeMethod(ToolbarHelpers::instance(), "fixSpacer", Qt::QueuedConnection, Q_ARG(qulonglong, tb), Q_ARG(int, width));
    }
}

void
ToolbarHelpers::fixSpacer(qulonglong toolbar, int width)
{
    s_spacerQueue.removeOne(QPair<qulonglong, int>(toolbar, width));
    QToolBar *tb = Ops::getChild<QToolBar *>(toolbar);
    if (!tb
            || !qobject_cast<QMainWindow *>(tb->parentWidget())
            || tb->findChild<QTabBar *>()
            || !tb->styleSheet().isEmpty())
        return;

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
}

static QList<qulonglong> s_marginsQueue;

void
ToolbarHelpers::adjustMarginsLater(QToolBar *toolBar)
{
    const qulonglong tb = (qulonglong)toolBar;
    if (!s_marginsQueue.contains(tb))
    {
        s_marginsQueue << tb;
        QMetaObject::invokeMethod(ToolbarHelpers::instance(), "adjustMargins", Qt::QueuedConnection, Q_ARG(qulonglong, tb));
    }
}
void
ToolbarHelpers::adjustMargins(qulonglong toolbar)
{
    s_marginsQueue.removeOne(toolbar);
    QToolBar *tb = Ops::getChild<QToolBar *>(toolbar);
    if (!tb)
        return;

    if (tb->isFloating())
    {
        tb->setMovable(true);
        tb->setContentsMargins(0, 0, 0, 0);
        tb->layout()->setContentsMargins(0, 0, 0, 0);
        return;
    }

    QMainWindow *win = qobject_cast<QMainWindow *>(tb->parentWidget());
//    if (!win || !toolBar->layout() || toolBar->actions().isEmpty() || win->toolBarArea(toolBar) != Qt::TopToolBarArea)
//        return;

    if (win
            && tb->geometry().top() <= win->rect().top()
            && win->toolBarArea(tb) == Qt::TopToolBarArea
            && !win->parentWidget())
    {
        tb->layout()->setContentsMargins(0, 0, 0, 0);
        WindowData *d = WindowData::memory(win->winId(), win);
        int m(0);
        if (d)
            m = d->value<uint>(WindowData::RightEmbedSize, 0);
        tb->QWidget::setContentsMargins(0, 0, tb->style()->pixelMetric(QStyle::PM_ToolBarHandleExtent)+m, 6);
    }
    else if (tb->findChild<QTabBar *>()) //sick, put a tabbar in a toolbar... eiskaltdcpp does this :)
    {
        tb->layout()->setContentsMargins(0, 0, 0, 0);
        tb->setMovable(false);
    }
    else
    {
        QEvent e(QEvent::StyleChange);
        QApplication::sendEvent(tb, &e);
    }
}
