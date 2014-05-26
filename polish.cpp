#include <QToolBar>
#include <QFrame>
#include <QMainWindow>
#include <QLayout>
#include <QMenuBar>
#include <QToolButton>
#include <QAbstractItemView>
#include <QGroupBox>
#include <QCheckBox>
#include <QDockWidget>
#include <QDesktopWidget>
#include <QLabel>
#include <QProgressBar>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>

#include "styleproject.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/shadowhandler.h"
#include "stylelib/progresshandler.h"

/* Thomas gave me his blessing to use
 * his macmenu! yeah! so now we get
 * macmenues in styleproject!
 */
#if !defined(QT_NO_DBUS)
#include "macmenu.h"
#endif


/* a non-const environment that is called
 * for for every widget directly after its
 * creation, one can manipulate them in
 * pretty much any way here.
 */

#define installFilter(_VAR_) _VAR_->removeEventFilter(this); _VAR_->installEventFilter(this)

void
StyleProject::polish(QWidget *widget)
{
    if (!widget)
        return;

    /* needed for mac os x lion like toolbuttons,
     * we need to repaint the toolbar when a action
     * has been triggered, otherwise we are painting
     * errors
     */
    if (castObj(QToolBar *, bar, widget))
    {
        if (qobject_cast<QMainWindow *>(widget->parentWidget()))
            connect(bar, SIGNAL(topLevelChanged(bool)), this, SLOT(fixMainWindowToolbar()));
        installFilter(bar);
    }

    if (castObj(QToolButton *, btn, widget))
    {
        installFilter(btn);
    }

    if (castObj(QMainWindow *, win, widget))
    {
        installFilter(win);
    }

    if (castObj(QProgressBar *, pBar, widget))
        ProgressHandler::manage(pBar);

//    if (castObj(QMainWindow *, mwin, widget))
//    {
//        if (QWidget *c = mwin->centralWidget())
//        {
//            c->setContentsMargins(0, 0, 0, 0);
//            if (QLayout *l = c->layout())
//            {
//                l->setContentsMargins(0, 0, 0, 0);
//                l->setSpacing(0);
//            }
//        }
//    }

    /** TODO:
     *  Now the shadowhandler handles *ALL*
     *  windows... that we dont want, we
     *  dont want desktop panels etc to have a
     *  shadow... fix.
     *
     * EDIT: started working on it
     */
    if (widget->isWindow())
    {
        installFilter(widget);
        bool needShadows(false);
        if (widget->windowType() == Qt::ToolTip && widget->inherits("QTipLabel"))
        {
            widget->setAttribute(Qt::WA_TranslucentBackground);
            needShadows = true;
        }
        if (qobject_cast<QMenu *>(widget)
                || qobject_cast<QDockWidget *>(widget)
                || qobject_cast<QToolBar *>(widget))
            needShadows = true;
        if (needShadows)
            ShadowHandler::manage(widget);
    }
    if (castObj(QScrollBar *, sb, widget))
        sb->setAttribute(Qt::WA_Hover);

    if (widget->inherits("KTitleWidget"))
    {
        installFilter(widget);
        QList<QWidget *> children = widget->findChildren<QWidget *>();
        for (int i = 0; i < children.size(); ++i)
        {
            children.at(i)->setAutoFillBackground(false);
            if (castObj(QFrame *, f, children.at(i)))
                f->setFrameStyle(0);
            if (castObj(QLabel *, l, children.at(i)))
                l->setAlignment(Qt::AlignCenter);
        }
    }

    if (castObj(QFrame *, frame, widget))
        if (frame->frameShadow() == QFrame::Sunken
                && qobject_cast<QMainWindow *>(frame->window()))
            OverLay::manage(frame, 100);

    if (castObj(QAbstractItemView *, view, widget))
    {
        view->viewport()->setAttribute(Qt::WA_Hover);
        view->setAttribute(Qt::WA_MouseTracking);
        installFilter(view);
    }
    if (castObj(QComboBox *, box, widget))
        if (QLineEdit *l = box->findChild<QLineEdit *>())
            connect(l, SIGNAL(textChanged(QString)), box, SLOT(update()));
    if (qobject_cast<QMenu *>(widget)||qobject_cast<QMenuBar *>(widget))
    {
        widget->setAttribute(Qt::WA_TranslucentBackground);
        widget->setMouseTracking(true);
        widget->setAttribute(Qt::WA_Hover);
        if (qobject_cast<QMenu *>(widget))
        {
            widget->setForegroundRole(QPalette::Text);
            widget->setBackgroundRole(QPalette::Base);
        }
        /**
         * Apparently some menues fight back and dont
         * want mousetracking, we filter and set mousetracking
         * when the menu is shown.
         */
        installFilter(widget);
    }
    if (widget->inherits("KTabWidget"))
    {
        installFilter(widget);
    }
    if (castObj(QTabBar *, tabBar, widget))
    {
//        tabBar->setTabsClosable(true);
//        QFont f(tabBar->font());
//        f.setBold(true);
//        tabBar->setFont(f);
//        installFilter(tabBar);
//        if (tabBar->documentMode())
//            tabBar->setExpanding(true);
        tabBar->setAttribute(Qt::WA_Hover);
        tabBar->setAttribute(Qt::WA_MouseTracking);
        if (!qApp->applicationName().compare("konsole", Qt::CaseInsensitive))
        {
            QWidget *p(tabBar->parentWidget());
            if (p)
            {
                p->setObjectName("konsole_tabbar_parent");
                installFilter(p);
            }
        }
    }

#if !defined(QT_NO_DBUS)
    if (castObj(QMenuBar *, menuBar, widget))
        Bespin::MacMenu::manage(menuBar);
#endif
    QCommonStyle::polish(widget);
}

void
StyleProject::unpolish(QWidget *widget)
{
#if !defined(QT_NO_DBUS)
    if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(widget))
        Bespin::MacMenu::release(menuBar);
#endif
    QCommonStyle::unpolish(widget);
}
