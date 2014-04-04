
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
    QPlastiqueStyle::polish(widget);
}
