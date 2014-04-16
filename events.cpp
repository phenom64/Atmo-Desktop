#include <QTableWidget>
#include <QPainter>
#include <QStyleOptionTabWidgetFrameV2>
#include <QMainWindow>
#include <QToolBar>
#include <QLayout>
#include <QAction>
#include <QResizeEvent>

#include "styleproject.h"
#include "stylelib/xhandler.h"

bool
StyleProject::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() < EVSize && m_ev[e->type()])
        return (this->*m_ev[e->type()])(o, e);

    switch (e->type())
    {
    case QEvent::Show:
    {
        if (castObj(QMainWindow *, win, o))
        {
            unsigned int d(1);
            XHandler::setXProperty<unsigned int>(win->winId(), XHandler::MainWindow, &d);
            unsigned int c(m_specialColor[0].rgba());
//            qDebug() << ((c & 0xff000000) >> 24) << ((c & 0xff0000) >> 16) << ((c & 0xff00) >> 8) << (c & 0xff);
//            qDebug() << QColor(c).alpha() << QColor(c).red() << QColor(c).green() << QColor(c).blue();
            XHandler::setXProperty<unsigned int>(win->winId(), XHandler::HeadColor, &c);
        }
    }
    case QEvent::Leave:
    case QEvent::HoverLeave:
    case QEvent::Enter:
    case QEvent::HoverEnter:
    {
        if (castObj(QAbstractItemView *, v, o))
            v->viewport()->update();
        break;
    }
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
    QResizeEvent *re = static_cast<QResizeEvent *>(e);
    if (castObj(QToolBar *, toolBar, o))
        if (castObj(QMainWindow *, win, toolBar->parentWidget()))
            if (win->toolBarArea(toolBar) == Qt::TopToolBarArea)
    {
        QPoint topLeft = toolBar->mapTo(win, toolBar->rect().topLeft());
        QRect winRect = win->rect();
        QRect widgetRect = QRect(topLeft, re->size());
        if (winRect.top() <= widgetRect.top())
            toolBar->setContentsMargins(0, 0, 0, 5);
    }
    return QCommonStyle::eventFilter(o, e);
}
