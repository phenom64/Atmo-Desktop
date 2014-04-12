
#include <QStyleOption>
#include <QStyleOptionFrame>
#include <QStyleOptionComboBox>
#include <QDebug>
#include <QLineEdit>
#include <QToolBar>
#include <QGroupBox>
#include <QAbstractItemView>

#include "styleproject.h"
#include "render.h"
#include "overlay.h"
#include "ops.h"

bool
StyleProject::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!painter->isActive())
        return false;
    if (widget && widget->objectName() == "qt_spinbox_lineedit")
        return true;

    int roundNess = 2;
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
    if (!widget || !painter->isActive())
        return true;

    int roundNess(2);
    bool isGroupBox(false);
    QRect r(option->rect);
    if (qobject_cast<const QGroupBox *>(widget))
    {
        isGroupBox = true;
        r.setTop(r.top()+8);
        roundNess = 8;
    }
    QPalette::ColorRole base(widget->backgroundRole());
    bool needFill(widget->autoFillBackground());
    if (const QAbstractScrollArea *a = qobject_cast<const QAbstractScrollArea *>(widget))
        if (a->viewport()->autoFillBackground())
            base = a->viewport()->backgroundRole();
        else
            needFill = false;

    if (isGroupBox || !findChild<OverLay *>())
    {
        painter->save();
        painter->fillRect(r, option->palette.color(widget->backgroundRole()));
        painter->setOpacity(0.25f);
        Render::renderShadow(Render::Sunken, r.adjusted(1, 1, -1, 0), painter, roundNess);
        painter->restore();
        return true;
    }
    return false;
}

bool
StyleProject::drawComboBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ComboBox, opt, option);
    if (!opt)
        return true;

    QPalette::ColorRole fg(QPalette::ButtonText), bg(QPalette::Button);
//    if (widget)
//    {
//        fg = widget->foregroundRole();
//        bg = widget->backgroundRole();
//    }

    QRect arrow(subControlRect(CC_ComboBox, opt, SC_ComboBoxArrow, widget));
    QRect frame(subControlRect(CC_ComboBox, opt, SC_ComboBoxFrame, widget));

    painter->fillRect(frame, opt->palette.color(bg));
    painter->fillRect(arrow, Ops::mid(opt->palette.color(bg), opt->palette.color(fg)));
    if (opt->editable)
    {
        QRect edit(subControlRect(CC_ComboBox, opt, SC_ComboBoxEditField, widget));
        painter->fillRect(edit, opt->palette.color(QPalette::Base));
    }
    return true;
}
