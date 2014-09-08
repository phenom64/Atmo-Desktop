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
#include "styleproject.h"
#include "stylelib/xhandler.h"
#include "stylelib/ops.h"
#include "stylelib/unohandler.h"
#include "stylelib/settings.h"

bool
StyleProject::eventFilter(QObject *o, QEvent *e)
{
    if (!e || !o || !o->isWidgetType())
        return false;
    if (e->type() < EVSize && m_ev[e->type()])
        return (this->*m_ev[e->type()])(o, e);

    QWidget *w(static_cast<QWidget *>(o));
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
    case QEvent::Close:
        if (w->testAttribute(Qt::WA_TranslucentBackground) && w->isWindow())
            XHandler::deleteXProperty(w->winId(), XHandler::KwinBlur);
    case QEvent::MouseButtonPress:
    {
        if (QToolButton *tb = qobject_cast<QToolButton *>(w))
            if (Ops::hasMenu(tb))
        {
            QStyleOptionToolButton option;
            option.initFrom(tb);
            option.features = QStyleOptionToolButton::None;
            if (tb->popupMode() == QToolButton::MenuButtonPopup) {
                option.subControls |= QStyle::SC_ToolButtonMenu;
                option.features |= QStyleOptionToolButton::MenuButtonPopup;
            }
            if (tb->arrowType() != Qt::NoArrow)
                option.features |= QStyleOptionToolButton::Arrow;
            if (tb->popupMode() == QToolButton::DelayedPopup)
                option.features |= QStyleOptionToolButton::PopupDelay;
            if (tb->menu())
                option.features |= QStyleOptionToolButton::HasMenu;
            QRect r = subControlRect(QStyle::CC_ToolButton, &option, QStyle::SC_ToolButtonMenu, tb);
            if (r.contains(static_cast<QMouseEvent *>(e)->pos()))
                tb->setProperty("DSP_menupress", true);
            else
                tb->setProperty("DSP_menupress", false);
        }
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
        if (!Ops::isSafariTabBar(tb))
            return false;
        opt.rect.setHeight(tb->height());
        QPainter p(w);
        QRect geo(tb->mapTo(w, tb->rect().topLeft()), tb->size());
        p.setClipRegion(QRegion(w->rect())-QRegion(geo));
        drawTabBar(&opt, &p, tb);
        p.end();
        return true;
    }
    else if (QMainWindow *win = qobject_cast<QMainWindow *>(o))
    {
        bool ok;
        int th(win->property("DSP_headHeight").toInt(&ok));
        if (ok)
        {
            bool needRound(true);
            const QColor bgColor(win->palette().color(win->backgroundRole()));
            QPainter p(win);
            if (XHandler::opacity() < 1.0f)
            {
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
                        if (bar->mapTo(win, bar->rect().bottomLeft()).y() >= win->rect().bottom())
                            needRound = false;
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
                p.setClipRegion(r);
                if (needRound)
                    Render::renderMask(win->rect(), &p, bgColor, 4, Render::Bottom|Render::Left|Render::Right);
                else
                    p.fillRect(win->rect(), bgColor);
            }
            else
                p.fillRect(win->rect(), bgColor);
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
    QWidget *w(static_cast<QWidget *>(o));
    if ((qobject_cast<QMenuBar*>(o)||qobject_cast<QMenu *>(o)))
    {
        static_cast<QWidget *>(o)->setMouseTracking(true);
        static_cast<QWidget *>(o)->setAttribute(Qt::WA_Hover);
        if (qobject_cast<QMenu *>(o))
        {
            unsigned int d(0);
            XHandler::setXProperty<unsigned int>(static_cast<QMenu *>(o)->winId(), XHandler::KwinBlur, XHandler::Long, &d);
        }
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
        if (!toolBar)
            return false;
        //below simply to trigger an event that forces the toolbar to call sizeFromContents again
        Ops::updateToolBarLater(toolBar, 50);
    }
    if (w->testAttribute(Qt::WA_TranslucentBackground) && w->isWindow())
    {
//        Ops::callLater(static_cast<QWidget *>(o), &QWidget::update);
        QTimer::singleShot(500, w, SLOT(update()));
    }
    return QCommonStyle::eventFilter(o, e);
}
