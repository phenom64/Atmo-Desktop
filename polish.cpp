#include <QToolBar>
#include <QFrame>
#include <QMainWindow>
#include <QLayout>
#include <QMenuBar>
#include <QToolButton>
#include <QAbstractItemView>
#include <QGroupBox>
#include <QCheckBox>


#include "styleproject.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/shadowhandler.h"

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

    if (castObj(QMainWindow *, win, widget))
    {
        installFilter(win);
    }

    /** TODO:
     *  Now the shadowhandler handles *ALL*
     *  windows... that we dont want, we
     *  dont want desktop panels etc to have a
     *  shadow... fix.
     */
    if (widget->isWindow())
    {
        installFilter(widget);
        ShadowHandler::manage(widget);
    }
    if (castObj(QScrollBar *, sb, widget))
        sb->setAttribute(Qt::WA_Hover);

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
