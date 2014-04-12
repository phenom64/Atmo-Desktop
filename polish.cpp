#include <QToolBar>
#include <QFrame>
#include <QMainWindow>
#include <QLayout>
#include <QMenuBar>
#include <QToolButton>

#include "styleproject.h"
#include "overlay.h"
#include "ops.h"

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
    if (QToolBar *bar = qobject_cast<QToolBar *>(widget))
    {
        bar->removeEventFilter(this);
        bar->installEventFilter(this);
    }

    if (QFrame *frame = qobject_cast<QFrame *>(widget))
        if (frame->frameShadow() == QFrame::Sunken
                && qobject_cast<QMainWindow *>(frame->window()))
            OverLay::manage(frame, 100);

    if (widget->inherits("KTabWidget"))
    {
        widget->removeEventFilter(this);
        widget->installEventFilter(this);
    }

#if !defined(QT_NO_DBUS)
    if (QMenuBar *menuBar = qobject_cast<QMenuBar *>(widget))
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
