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
    case QEvent::Show:
        if ((qobject_cast<QMenuBar*>(o)||qobject_cast<QMenu *>(o)))
        {
            static_cast<QWidget *>(o)->setMouseTracking(true);
            static_cast<QWidget *>(o)->setAttribute(Qt::WA_Hover);
        }
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
    return QCommonStyle::eventFilter(o, e);
}

bool
StyleProject::resizeEvent(QObject *o, QEvent *e)
{
//    QResizeEvent *re = static_cast<QResizeEvent *>(e);
    if (castObj(QWidget *, w, o))
        if (w->isWindow())
//            Ops::fixWindowTitleBar(w);
            fixTitleLater(w);
    if (castObj(QToolBar *, toolBar, o))
        if (castObj(QMainWindow *, win, toolBar->parentWidget()))
            updateToolBar(toolBar);
    return QCommonStyle::eventFilter(o, e);
}
