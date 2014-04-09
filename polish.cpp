#include <QToolBar>

#include "styleproject.h"


/* a non-const environment that is called
 * for for every widget directly after its
 * creation, one can manipulate them in
 * pretty much any way here.
 *
 * For now this function does nothing but
 * call the base implementation.
 */

void
StyleProject::polish(QWidget *widget)
{
    if (!widget)
        return;

    /* needed for mac os x lion like toolbuttons,
     * we need to repaint the toolbar when a action
     * has been triggered, otherwise we are painting
     * errors */
    if (QToolBar *bar = qobject_cast<QToolBar *>(widget))
        connect(bar, SIGNAL(actionTriggered(QAction*)), bar, SLOT(update()));
    QPlastiqueStyle::polish(widget);
}
