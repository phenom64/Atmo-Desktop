
#include <QStyleOption>
#include <QStyleOptionFrame>
#include <QDebug>
#include <QLineEdit>
#include <QToolBar>
#include <QGroupBox>
#include <QAbstractItemView>

#include "styleproject.h"
#include "render.h"

bool
StyleProject::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && widget->objectName() == "qt_spinbox_lineedit")
        return true;

    int roundNess = 6;
    if (const QLineEdit *lineEdit = qobject_cast<const QLineEdit *>(widget))
        if (qobject_cast<const QToolBar *>(lineEdit->parentWidget()))
            roundNess = MAXRND;

    painter->save();
    Render::renderMask(option->rect.adjusted(1, 1, -1, -2), painter, option->palette.color(QPalette::Base), roundNess);
    painter->setOpacity(0.5f);
    Render::renderShadow(Render::Sunken, option->rect, painter, roundNess);
    painter->restore();
    return true;
}

/* not exactly a 'input' widget in all cases but here now, deal w/ it */

bool
StyleProject::drawFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!widget)
        return true;

    if (const QGroupBox *box = qobject_cast<const QGroupBox *>(widget))
    {
        return false;
    }
    if (const QAbstractScrollArea *v = qobject_cast<const QAbstractScrollArea *>(widget))
    {
        painter->save();
        Render::renderMask(option->rect.adjusted(0, 0, 0, -1), painter, option->palette.color(v->viewport()->backgroundRole()), 4);
        painter->setOpacity(0.5f);
        Render::renderShadow(Render::Etched, option->rect, painter, 4);
        painter->restore();
        return true;
    }
    return false;
}
