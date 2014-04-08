
#include <QStyleOption>

#include "styleproject.h"
#include "render.h"

bool
StyleProject::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    Render::renderMask(option->rect, painter, option->palette.color(QPalette::Base));
    return true;
}
