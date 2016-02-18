#include <QTableWidget>
#include <QPainter>
#include <QStyleOptionTabWidgetFrameV2>
#include <QStyleOptionToolButton>
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
#include <QPushButton>
#include <QDebug>
#include <QScrollBar>

#include "dsp.h"
#include "stylelib/xhandler.h"
#include "stylelib/ops.h"
#include "stylelib/handlers.h"
#include "config/settings.h"
#include "overlay.h"
#include "stylelib/animhandler.h"
#include "stylelib/shadowhandler.h"

#include "defines.h"
#if HASDBUS
#include "stylelib/macmenu.h"
#endif

using namespace DSP;

bool
Style::eventFilter(QObject *o, QEvent *e)
{
    if (!e || !o || !o->isWidgetType())
        return false;
    if (e->type() < EVSize && m_ev[e->type()])
        return (this->*m_ev[e->type()])(o, e);

    QWidget *w(static_cast<QWidget *>(o));
    switch (e->type())
    {
#if DEBUG
//    case QEvent::Show:
//    case QEvent::Leave:
//    case QEvent::HoverLeave:
    case QEvent::Enter:
//    case QEvent::HoverEnter:
    {
        qDebug() << w << w->parentWidget();
    }
#endif
    case QEvent::Hide:
    {
        if (dConf.uno.enabled && (qobject_cast<QTabBar *>(w) || qobject_cast<QMenuBar*>(o)))
            Handlers::Window::updateWindowDataLater(w->window());
        break;
    }
    case QEvent::LayoutRequest:
    {
        if (w->inherits("KTitleWidget") && !dConf.animateStack)
        {
//            QList<QLabel *> lbls = w->findChildren<QLabel *>();
//            for (int i = 0; i < lbls.count(); ++i)
//                lbls.at(i)->setAlignment(Qt::AlignCenter);
        }
        else if (qobject_cast<QMenuBar *>(w))
        {
            if (dConf.uno.enabled)
                Handlers::Window::updateWindowDataLater(w->window());
//#if HASDBUS
//            if (BE::MacMenu::isActive() && BE::MacMenu::manages(static_cast<QMenuBar *>(w)))
//            {
//                /* Sometimes the menubar shows itself as a glitghy
//                 * square painting some undefined data in the topleft
//                 * corner for some reason I've yet to understand...
//                 *
//                 * NOTE: *only* qt5 apps are affected
//                 */
//                w->setFixedSize(1,1);
//                w->setFixedSize(0,0);
//                w->move(-QPoint(w->width(), w->height()));
//            }
//#endif
        }
        break;
    }
//    case QEvent::MetaCall:
//    {
//        if (w->property("DSP_KTitleLabel").toBool() && !dConf.animateStack)
//            static_cast<QLabel *>(w)->setAlignment(Qt::AlignCenter);
//        break;
//    }
#if 0
    case QEvent::HoverEnter:
    {
        qDebug() << w << w->parentWidget();
        break;
    }
#endif
    default: break;
    }
    return QCommonStyle::eventFilter(o, e);
}

bool
Style::paintEvent(QObject *o, QEvent *e)
{
    /* for some reason KTabWidget is an idiot and
     * doesnt use the style at all for painting, only
     * for the tabBar apparently (that inside the
     * KTabWidget) so we simply override the painting
     * for the KTabWidget and paint what we want
     * anyway.
     */
    QWidget *w(static_cast<QWidget *>(o));

    if (qobject_cast<QTabWidget *>(w))
    {
        QTabBar *tb = w->findChild<QTabBar *>();
        if (!tb)
            return false;
        QStyleOptionTabBarBaseV2 opt;
        opt.rect = tb->geometry();
        opt.fontMetrics = w->fontMetrics();
        opt.documentMode = true;
        QPainter p(w);
        bool needPaint(false);
        for (int i = 0; i < 4; ++i)
            needPaint |= (bool)static_cast<QTabWidget *>(w)->cornerWidget((Qt::Corner)i);
        if (needPaint)
            drawTabBar(&opt, &p, tb);
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
        return false;
    }
    else if (o->objectName() == "konsole_tabbar_parent")
    {
        QWidget *w = static_cast<QWidget *>(o);
        QTabBar *tb = w->findChild<QTabBar *>();
        if (!tb || !tb->documentMode())
            return false;

        QStyleOptionTabBarBaseV2 opt;
        opt.rect = w->rect();
        opt.rect.setHeight(tb->height());
        QPainter p(w);
        QRect geo(tb->mapTo(w, tb->rect().topLeft()), tb->size());
        p.setClipRegion(QRegion(w->rect())-QRegion(geo));
        drawTabBar(&opt, &p, tb);
        p.end();
        return true;
    }
    else if (w->inherits("KMultiTabBarInternal"))
    {
        QPainter p(w);
        QRect r(w->rect());
        const bool hor(w->width()>w->height());
        QLinearGradient lg(r.topLeft(), !hor?r.topRight():r.bottomLeft());
        lg.setStops(DSP::Settings::gradientStops(dConf.tabs.gradient, w->palette().color(QPalette::Button)));
        p.fillRect(r, lg);
        p.end();
        return false;
    }
    else if (w->inherits("KMultiTabBarTab"))
    {
        if (w->underMouse() || static_cast<QPushButton *>(w)->isChecked())
            return false;
        QPainter p(w);
        QStyleOptionToolButton opt;
        opt.initFrom(w);
        drawToolButtonBevel(&opt, &p, w);
        p.end();

        w->removeEventFilter(this);
        QCoreApplication::sendEvent(o, e);
        w->installEventFilter(this);
        return true;
    }
    else if (QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(w->parent()))
    {
        area->verticalScrollBar()->update();
        area->horizontalScrollBar()->update();
        return false;
    }
#if 0
    else if (QTabBar *bar = qobject_cast<QTabBar *>(w))
    {
        if (!bar->documentMode() || Ops::isSafariTabBar(bar))
            return false;
        w->removeEventFilter(this);
        QCoreApplication::sendEvent(o, e);
        QStyleOptionTabBarBaseV2 opt;
        opt.initFrom(w);
        QPainter p(w);
        drawTabBar(&opt, &p, w);
        w->installEventFilter(this);
        return true;
    }
#endif
    return QCommonStyle::eventFilter(o, e);
}

bool
Style::resizeEvent(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return false;
    QWidget *w(static_cast<QWidget *>(o));
    QResizeEvent *re = static_cast<QResizeEvent *>(e);
    if (w->inherits("KTitleWidget") && !dConf.animateStack)
    {
//        QList<QLabel *> lbls = w->findChildren<QLabel *>();
//        for (int i = 0; i < lbls.count(); ++i)
//            lbls.at(i)->setAlignment(Qt::AlignCenter);
    }
    else if (dConf.uno.enabled
             && (qobject_cast<QTabBar *>(w) || qobject_cast<QMenuBar*>(o))
             && re->oldSize().height() != re->size().height())
        Handlers::Window::updateWindowDataLater(w->window());
    return QCommonStyle::eventFilter(o, e);
}

bool
Style::showEvent(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QCommonStyle::eventFilter(o, e);
    QWidget *w(static_cast<QWidget *>(o));
    if (qobject_cast<QMenuBar*>(w))
    {
        if (dConf.uno.enabled)
            Handlers::Window::updateWindowDataLater(w->window());
//#if HASDBUS
//        if (BE::MacMenu::isActive() && BE::MacMenu::manages(static_cast<QMenuBar *>(w)))
//        {
//            /* Sometimes the menubar shows itself as a glitghy
//             * square painting some undefined data in the topleft
//             * corner for some reason I've yet to understand...
//             *
//             * NOTE: *only* qt5 apps are affected
//             */
//            w->setFixedSize(1,1);
//            w->setFixedSize(0,0);
//            w->move(-QPoint(w->width(), w->height()));
//        }
//#endif
        return false;
    }
    else if (qobject_cast<QMenu *>(w))
    {
        w->setMouseTracking(true);
        w->setAttribute(Qt::WA_Hover);
        return false;
    }
    else if (qobject_cast<QTabBar *>(w))
    {
        if (dConf.uno.enabled)
            Handlers::Window::updateWindowDataLater(w->window());
        return false;
    }
    else if (w->inherits("KTitleWidget") && !dConf.animateStack)
    {
//        QList<QLabel *> lbls = w->findChildren<QLabel *>();
//        for (int i = 0; i < lbls.count(); ++i)
//            lbls.at(i)->setAlignment(Qt::AlignCenter);
//        return false;
    }
    return QCommonStyle::eventFilter(o, e);
}
