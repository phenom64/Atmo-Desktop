/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**************************************************************************
*   Based on styleproject, Copyright (C) 2013 by Robert Metsaranta        *
*   therealestrob@gmail.com                                              *
***************************************************************************/
#include <QTableWidget>
#include <QPainter>
#include <QStyleOptionTabWidgetFrame>
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
#include <QHeaderView>

#include "nse.h"
#include "atmolib/xhandler.h"
#include "atmolib/ops.h"
#include "atmolib/handlers.h"
#include "config/settings.h"
#include "overlay.h"
#include "atmolib/animhandler.h"
#include "atmolib/shadowhandler.h"
#include "atmolib/windowhelpers.h"

#include "defines.h"
#if HASDBUS
#include "atmolib/macmenu.h"
#endif

using namespace NSE;

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

        if (w->parentWidget() && w->parentWidget()->parentWidget())
            qDebug() << w << w->parentWidget() << w->parentWidget()->parentWidget() << w->parentWidget()->parentWidget()->parentWidget();
        else if (w->parentWidget())
            qDebug() << w << w->parentWidget() << w->parentWidget()->parentWidget();
        else
            qDebug() << w << w->parentWidget();
    }
#endif
    case QEvent::DynamicPropertyChange:
    {
        if (w->isWindow() && w->testAttribute(Qt::WA_TranslucentBackground))
            Handlers::Window::applyBlur(w);
        break;
    }
    case QEvent::MouseButtonPress:
    {
#if DEBUG
        qDebug() << "widget mousepressed:" << w;
#endif
        if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(w))
        {
            QMouseEvent *me = static_cast<QMouseEvent *>(e);
            if (!menuBar->actionAt(me->pos()))
                XHandler::mwRes(me->pos(), me->globalPos(), menuBar->winId());
        }
        break;
    }
    case QEvent::Hide:
    {
        if (dConf.uno.enabled && (qobject_cast<QTabBar *>(w) || qobject_cast<QMenuBar*>(w)))
            WindowHelpers::updateWindowDataLater(w->window());
        break;
    }
    case QEvent::LayoutRequest:
    {
//        if (w->inherits("KTitleWidget") && !dConf.animateStack)
//        {
//            QList<QLabel *> lbls = w->findChildren<QLabel *>();
//            for (int i = 0; i < lbls.count(); ++i)
//                lbls.at(i)->setAlignment(Qt::AlignCenter);
//        }
        /*else */if (qobject_cast<QMenuBar *>(w))
        {
            if (dConf.uno.enabled)
                WindowHelpers::updateWindowDataLater(w->window());
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
        else if (w->inherits("KMultiTabBarInternal"))
        {
            if (QLayout *l = w->layout())
            {
                if (l->count())
                {
                    QLayoutItem *item = l->itemAt(l->count()-1);
                    if (item->spacerItem())
                    {
                        l->removeItem(item);
                        delete item;
                    }
                }
            }
        }
        break;
    }
#if 0
    case QEvent::WindowStateChange:
    {
        if (qobject_cast<QMainWindow *>(w))
        {
            if (QWidget *center = static_cast<QMainWindow *>(w)->centralWidget())
                if (center->property("NSE_center").toBool())
                {
                    if (w->isFullScreen())
                        center->setContentsMargins(0,0,0,0);
                    else
                        center->setContentsMargins(8,8,8,8);
                }
        }
    }
#endif
    default: break;
    }
    return QCommonStyle::eventFilter(o, e);
}

bool
Style::paintEvent(QObject *o, QEvent *e)
{
    /* for some reason KTabWidget
     * doesnt use the style at all for painting, only
     * for the tabBar apparently (that inside the
     * KTabWidget) so we simply override the painting
     * for the KTabWidget and paint what we want
     * anyway.
     */
    QWidget *w(static_cast<QWidget *>(o));

    if (qobject_cast<QTabWidget *>(w))
    {
        return false;
        QTabBar *tb = w->findChild<QTabBar *>();
        if (!tb)
            return false;
        QStyleOptionTabBarBase opt;
        opt.rect = tb->geometry();
        opt.fontMetrics = w->fontMetrics();
        opt.documentMode = true;
        QPainter p(w);
        bool needPaint(false);
        for (int i = 0; i < 4; ++i)
            needPaint |= (bool)static_cast<QTabWidget *>(w)->cornerWidget((Qt::Corner)i);
        if (needPaint)
            drawTabBar(&opt, &p, tb);
//        if (o->inherits("KTabWidget"))
//        {
//            QTabWidget *tabWidget = static_cast<QTabWidget *>(o);
//            QPainter p(tabWidget);
//            QStyleOptionTabWidgetFrame opt;
//            opt.initFrom(tabWidget);
//            drawTabWidget(&opt, &p, tabWidget);
//            p.end();
//            return true;
//        }
        return false;
    }
    else if (w->inherits("DolphinUrlNavigator"))
    {
//        auto inTb = [] (QWidget *wd)->bool { QWidget *w2 = wd; while(w2->parentWidget()) {if (qobject_cast<QToolBar *>(w2)) return true; w2 = w2->parentWidget();} return false;};
//        if (inTb(w))
//        {
            QPainter p(w);
            GFX::drawClickable(Sunken, w->rect(), &p, QBrush(), dConf.toolbtn.rnd);
            return false;
//        }
    }
    else if (o->property("NSE_konsoleTabBarParent").toBool())
    {
        QWidget *w = static_cast<QWidget *>(o);
        QTabBar *tb = w->findChild<QTabBar *>();
        if (!tb || !tb->documentMode())
            return false;

        QStyleOptionTabBarBase opt;
        const QPoint tl = tb->mapFrom(w, QPoint());
        opt.rect = QRect(tl, w->size()+QSize(qAbs(tl.x()), 0));
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
//        QPainter p(w);
//        QRect r(w->rect());
//        const bool hor(w->width()>w->height());
//        QLinearGradient lg(r.topLeft(), !hor?r.topRight():r.bottomLeft());
//        lg.setStops(NSE::Settings::gradientStops(dConf.tabs.gradient, w->palette().color(QPalette::Button)));
//        p.fillRect(r, lg);
//        p.end();
//        qDebug() << w->property("qproperty-position") << w->parentWidget()->property("qproperty-position");
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
    else if (w->inherits("KateTabButton")) //kate tabbutons and tabbar... sighs
    {
        QAbstractButton *btn = static_cast<QAbstractButton *>(w);
        QPainter p(w);
        QStyleOptionTab opt;
        opt.initFrom(w);
        opt.text = btn->text();
        if (btn->isChecked())
            opt.state |= State_Selected|State_Sunken|State_Active; //which one of these is it?
        const bool reg = dConf.tabs.regular;
        dConf.tabs.regular = true;
        drawTab(&opt, &p);
        dConf.tabs.regular = reg;
        return true;
    }
    else if (w->inherits("KateTabBar"))
    {
        QPainter p(w);
        QStyleOptionTabBarBase opt;
        opt.initFrom(w);
        opt.tabBarRect = w->rect();
        const bool reg = dConf.tabs.regular;
        dConf.tabs.regular = true;
        drawTabBar(&opt, &p);
        dConf.tabs.regular = reg;
        return true;
    }
    else if (w->property("NSE_center").toBool())
    {
        QPainter p(w);
        GFX::drawClickable(Raised, w->rect(), &p, w->palette().color(QPalette::Window), 6);
        return false;
    }
//    else if (w->inherits("KTextEditor::View"))
//    {
//        w->removeEventFilter(this);
//        QCoreApplication::sendEvent(w, e);
//        QPainter p(w);
//        GFX::drawShadow(Sunken, w->rect(), &p, true, 0);
//        p.end();
//        w->installEventFilter(this);
//        return true;
//    }
//    else if (QAbstractScrollArea *area = qobject_cast<QAbstractScrollArea *>(w->parent()))
//    {
//        area->verticalScrollBar()->update();
//        area->horizontalScrollBar()->update();
//        return false;
//    }
#if 0
    else if (QTabBar *bar = qobject_cast<QTabBar *>(w))
    {
        if (!bar->documentMode() || Ops::isSafariTabBar(bar))
            return false;
        w->removeEventFilter(this);
        QCoreApplication::sendEvent(o, e);
        QStyleOptionTabBarBase opt;
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
        WindowHelpers::updateWindowDataLater(w->window());
    return QCommonStyle::eventFilter(o, e);
}

bool
Style::showEvent(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QCommonStyle::eventFilter(o, e);
    QWidget *w(static_cast<QWidget *>(o));

#if DEBUG
    qDebug() << "widget shown:" << w;
#endif

    if (w->isWindow() && w->testAttribute(Qt::WA_TranslucentBackground))
        Handlers::Window::applyBlur(w);
    if (qobject_cast<QMenuBar*>(w))
    {
        if (dConf.uno.enabled)
            WindowHelpers::updateWindowDataLater(w->window());
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
            WindowHelpers::updateWindowDataLater(w->window());
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
