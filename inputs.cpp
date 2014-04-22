
#include <QStyleOption>
#include <QStyleOptionFrame>
#include <QStyleOptionComboBox>
#include <QDebug>
#include <QLineEdit>
#include <QToolBar>
#include <QGroupBox>
#include <QComboBox>
#include <QAbstractItemView>

#include "styleproject.h"
#include "stylelib/render.h"
#include "overlay.h"
#include "stylelib/ops.h"
#include "stylelib/color.h"

static void drawSafariLineEdit(const QRect &r, QPainter *p, const QBrush &b)
{
    Render::renderMask(r.adjusted(1, 1, -1, -2), p, b);
    const int o(p->opacity());
    p->setOpacity(0.25f);
    Render::renderShadow(Render::Etched, r, p);
    Render::renderShadow(Render::Sunken, r, p);
    p->setOpacity(o);
}

bool
StyleProject::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (qobject_cast<const QComboBox *>(widget?widget->parentWidget():0))
        return true;
    if (widget && widget->objectName() == "qt_spinbox_lineedit")
        return true;

    int roundNess = 2;
    if (const QLineEdit *lineEdit = qobject_cast<const QLineEdit *>(widget))
        if (qobject_cast<const QToolBar *>(lineEdit->parentWidget()))
            roundNess = MAXRND;

    if (roundNess == MAXRND)
        drawSafariLineEdit(option->rect, painter, option->palette.color(QPalette::Base));
    else
    {
        Render::renderMask(option->rect.adjusted(1, 1, -1, -2), painter, option->palette.color(QPalette::Base), roundNess);
        const int o(painter->opacity());
        painter->setOpacity(0.5f);
        Render::renderShadow(Render::Sunken, option->rect, painter, roundNess);
        painter->setOpacity(o);
    }
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

    QPalette::ColorRole bg(QPalette::Button), fg(QPalette::ButtonText), ar(QPalette::HighlightedText);
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
    }

    QRect arrow(subControlRect(CC_ComboBox, opt, SC_ComboBoxArrow, widget));
    QRect frame(subControlRect(CC_ComboBox, opt, SC_ComboBoxFrame, widget));
    int m(3);

    if (!opt->editable)
    {
        const float o(painter->opacity());
        painter->setOpacity(0.5f*o);
        Render::renderShadow(Render::Raised, frame, painter, 5);
        painter->setOpacity(o);


        const QColor bgc(opt->palette.color(bg));
        QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());

        lg.setColorAt(0.0f, Color::mid(bgc, Qt::white, 5, 1));
        lg.setColorAt(1.0f, Color::mid(bgc, Qt::black, 7, 1));

        frame.adjust(m, m, -m, -m);
        Render::renderMask(frame, painter, lg, 2);

        const QColor hc(opt->palette.color(QPalette::Highlight));
        lg.setColorAt(0.0f, Color::mid(hc, Qt::white, 5, 1));
        lg.setColorAt(1.0f, Color::mid(hc, Qt::black, 7, 1));
        arrow.adjust(0, m, -m, -m);
        Render::renderMask(arrow, painter, lg, 2, Render::All & ~Render::Left);
        painter->setPen(opt->palette.color(fg));
        painter->setOpacity(0.5f);
        painter->drawLine(arrow.topLeft()-QPoint(1, 0), arrow.bottomLeft()-QPoint(1, 0));
        painter->setOpacity(o);
    }
    else
    {
        drawSafariLineEdit(opt->rect, painter, opt->palette.color(QPalette::Base));
    }
    QRect iconRect(0, 0, frame.height(), frame.height());

    drawItemPixmap(painter, iconRect, Qt::AlignCenter, opt->currentIcon.pixmap(opt->iconSize));
    m*=2;
    Ops::drawArrow(painter, opt->palette.color(opt->editable?QPalette::Text:QPalette::HighlightedText), arrow.adjusted(m*1.25f, m, -m, -m), Ops::Down);
    return true;
}

bool
StyleProject::drawComboBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ComboBox, opt, option);
    if (!opt || opt->editable)
        return true;
    QPalette::ColorRole bg(QPalette::Button), fg(QPalette::ButtonText);
    if (widget)
    {
        bg = widget->backgroundRole();
        fg = widget->foregroundRole();
    }
    const int m(6);
    QRect frameRect(subControlRect(CC_ComboBox, opt, SC_ComboBoxFrame, widget).adjusted(m, 0, -m, 0));
    drawItemText(painter, frameRect, Qt::AlignLeft|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->currentText, fg);
    return true;
}
