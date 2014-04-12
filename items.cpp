#include <QMenuBar>
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionMenuItem>

#include "styleproject.h"

bool
StyleProject::drawMenuItem(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!painter->isActive())
        return false;
    castOpt(MenuItem, opt, option);
    drawItemText(painter, option->rect, Qt::AlignLeft|Qt::AlignVCenter, option->palette, option->state & State_Enabled, opt->text);
    return true;
}
