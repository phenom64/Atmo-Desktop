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
#include "styleproject.h"
#include "stylelib/xhandler.h"
#include "stylelib/ops.h"
#include "stylelib/unohandler.h"
#include "stylelib/settings.h"
#include "overlay.h"
#include "stylelib/animhandler.h"

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
//    {
//        qDebug() << w << w->parentWidget();
//    }
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
    case QEvent::Hide:
    {
        if (qobject_cast<QTabBar *>(w))
        {
            if (Ops::isSafariTabBar(static_cast<QTabBar *>(w)))
                UNO::Handler::fixWindowTitleBar(w->window());
        }
    }
    case QEvent::LayoutRequest:
    {
        if (w->inherits("KTitleWidget"))
        {
            QList<QLabel *> lbls = w->findChildren<QLabel *>();
            for (int i = 0; i < lbls.count(); ++i)
                lbls.at(i)->setAlignment(Qt::AlignCenter);
        }
    }
    case QEvent::MetaCall:
    {
        if (w->property("DSP_KTitleLabel").toBool())
            static_cast<QLabel *>(w)->setAlignment(Qt::AlignCenter);
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
    QWidget *w(static_cast<QWidget *>(o));
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
    else if (qobject_cast<QMainWindow *>(w))
    {
        QMainWindow *win(static_cast<QMainWindow *>(w));
        bool needRound(true);
        const QColor bgColor(win->palette().color(win->backgroundRole()));
        QPainter p(win);
        if (XHandler::opacity() < 1.0f)
        {
            if (dConf.uno.enabled)
            {
                QRegion r(ScrollWatcher::paintRegion(win));
                p.setClipRegion(r);
                if (needRound)
                    Render::renderMask(win->rect(), &p, bgColor, 4, Render::Bottom|Render::Left|Render::Right);
                else
                    p.fillRect(win->rect(), bgColor);
            }
            else
            {
                p.setOpacity(XHandler::opacity());
                if (win->property("DSP_hasbuttons").toBool())
                {
                    p.setRenderHint(QPainter::Antialiasing);
                    p.setBrush(bgColor);
                    p.setPen(Qt::NoPen);
                    p.drawRoundedRect(win->rect(), 4, 4);
                }
                else
                    Render::renderMask(win->rect(), &p, bgColor, 4, Render::Bottom|Render::Left|Render::Right);
                p.setOpacity(1.0f);
            }
        }
        else
            p.fillRect(win->rect(), bgColor);
        p.setPen(Qt::black);
        p.setOpacity(dConf.shadows.opacity);
        if (dConf.uno.enabled)
        if (int th = UNO::unoHeight(w, UNO::ToolBars))
            p.drawLine(0, th, win->width(), th);
        p.end();
        return false;
    }
    else if (w->inherits("KMultiTabBarInternal"))
    {
        QPainter p(w);
        QRect r(w->rect());
        const bool hor(w->width()>w->height());
        QLinearGradient lg(r.topLeft(), hor?r.topRight():r.bottomLeft());
        lg.setStops(Settings::gradientStops(dConf.tabs.gradient, w->palette().color(QPalette::Button)));
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
    return QCommonStyle::eventFilter(o, e);
}

bool
StyleProject::resizeEvent(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return false;
    QWidget *w(static_cast<QWidget *>(o));
    if (w->inherits("KTitleWidget"))
    {
        QList<QLabel *> lbls = w->findChildren<QLabel *>();
        for (int i = 0; i < lbls.count(); ++i)
            lbls.at(i)->setAlignment(Qt::AlignCenter);
    }
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
    else if (castObj(QToolButton *, toolButton, o))
    {
        castObj(QToolBar *, toolBar, toolButton->parentWidget());
        if (!toolBar)
            return false;
        //below simply to trigger an event that forces the toolbar to call sizeFromContents again
        Ops::updateToolBarLater(toolBar, 500);
    }
    else if (w->testAttribute(Qt::WA_TranslucentBackground) && w->isWindow())
    {
//        Ops::callLater(static_cast<QWidget *>(o), &QWidget::update);
        QTimer::singleShot(500, w, SLOT(update()));
    }
    else if (qobject_cast<QTabBar *>(w))
    {
        if (Ops::isSafariTabBar(static_cast<QTabBar *>(w)))
            UNO::Handler::fixWindowTitleBar(w->window());
    }
    else if (w->inherits("KTitleWidget"))
    {
        QList<QLabel *> lbls = w->findChildren<QLabel *>();
        for (int i = 0; i < lbls.count(); ++i)
            lbls.at(i)->setAlignment(Qt::AlignCenter);
    }
    return QCommonStyle::eventFilter(o, e);
}
