
#include <QStyleOption>
#include <QStyleOptionFrame>
#include <QStyleOptionComboBox>
#include <QStyleOptionSpinBox>
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
    Render::renderShadow(Render::Etched, r, p, 32, Render::All, 0.25f);
    Render::renderShadow(Render::Sunken, r, p, 32, Render::All, 0.25f);
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
        Render::renderShadow(Render::Sunken, option->rect, painter, roundNess, Render::All, 0.33f);
    }
    return true;
}

/* not exactly a 'input' widget in all cases but here now, deal w/ it */

bool
StyleProject::drawFrame(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (!widget)
        return true;

    castOpt(FrameV3, opt, option);
    if (!opt)
        return true;

    castObj(const QFrame *, frame, widget);

    int roundNess(2);
    QRect r(option->rect);

    if ((frame && frame->frameShadow() == QFrame::Sunken) || opt->state & State_Sunken)
    if (!frame->findChild<OverLay *>())
        Render::renderShadow(Render::Sunken, r.adjusted(1, 1, -1, 0), painter, roundNess, Render::All, 0.25f);

    if (opt->state & State_Raised)
    {
        QPixmap pix(frame->rect().size());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        Render::renderShadow(Render::Raised, pix.rect(), &p, 8);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Render::renderMask(pix.rect().adjusted(2, 2, -2, -2), &p, Qt::black, 6);
        p.end();
        painter->drawTiledPixmap(frame->rect(), pix);
    }
    if (frame && frame->frameShadow() == QFrame::Plain)
        Render::renderShadow(Render::Etched, r, painter, 6, Render::All, 0.25f);
    return true;
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

    const bool ltr(opt->direction == Qt::LeftToRight);

    QRect arrowRect(subControlRect(CC_ComboBox, opt, SC_ComboBoxArrow, widget));
    QRect frameRect(subControlRect(CC_ComboBox, opt, SC_ComboBoxFrame, widget));
    QRect iconRect; //there is no SC_ rect function for this?
    QRect textRect(frameRect);

    if (!opt->currentIcon.isNull())
    {
        const int h(frameRect.height());
        iconRect = QRect(0, 0, h, h);
    }
    if (opt->direction == Qt::LeftToRight)
    {
        iconRect.moveLeft(frameRect.left());
        textRect.setLeft(iconRect.right());
        textRect.setRight(arrowRect.left());
    }
    else if (opt->direction == Qt::RightToLeft)
    {
        iconRect.moveRight(frameRect.right());
        textRect.setRight(iconRect.left());
        textRect.setLeft(arrowRect.right());
    }
    int m(2);

    if (!opt->editable)
    {
        Render::renderShadow(Render::Raised, frameRect, painter, 5);

        const QColor bgc(opt->palette.color(bg));
        QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());

        lg.setColorAt(0.0f, Color::mid(bgc, Qt::white, 5, 1));
        lg.setColorAt(1.0f, Color::mid(bgc, Qt::black, 7, 1));

        frameRect.adjust(m, m, -m, -m);
        Render::renderMask(frameRect, painter, lg, 2);

        const QColor hc(opt->palette.color(QPalette::Highlight));
        lg.setColorAt(0.0f, Color::mid(hc, Qt::white, 5, 1));
        lg.setColorAt(1.0f, Color::mid(hc, Qt::black, 7, 1));
        arrowRect.adjust(!ltr?m:0, m, -(ltr?m:0), -m);
        Render::renderMask(arrowRect, painter, lg, 2, Render::All & ~(ltr?Render::Left:Render::Right));
        painter->setPen(opt->palette.color(fg));
        const float o(painter->opacity());
        painter->setOpacity(0.5f);
        if (ltr)
            painter->drawLine(arrowRect.topLeft()-QPoint(1, 0), arrowRect.bottomLeft()-QPoint(1, 0));
        else
            painter->drawLine(arrowRect.topRight()+QPoint(1, 0), arrowRect.bottomRight()+QPoint(1, 0));
        painter->setOpacity(o);
    }
    else
    {
        drawSafariLineEdit(opt->rect, painter, opt->palette.color(QPalette::Base));
    }

    drawItemPixmap(painter, iconRect, Qt::AlignCenter, opt->currentIcon.pixmap(opt->iconSize));
    m*=2;
    Ops::drawArrow(painter, opt->palette.color(opt->editable?QPalette::Text:QPalette::HighlightedText), arrowRect.adjusted(m*1.25f, m, -m, -m), Ops::Down);
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
    const bool ltr(opt->direction==Qt::LeftToRight);
    QRect rect(subControlRect(CC_ComboBox, opt, SC_ComboBoxEditField, widget).adjusted(m, 0, -m, 0));
    const int hor(ltr?Qt::AlignLeft:Qt::AlignRight);
    if (!opt->currentIcon.isNull())
    {
        if (ltr)
            rect.setLeft(rect.left()+opt->iconSize.width());
        else
            rect.setRight(rect.right()-opt->iconSize.width());
    }
    drawItemText(painter, rect, hor|Qt::AlignVCenter, opt->palette, opt->ENABLED, opt->currentText, fg);
    return true;
}

bool
StyleProject::drawSpinBox(const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(SpinBox, opt, option);
    if (!opt)
        return true;

    QRect frame(subControlRect(CC_SpinBox, opt, SC_SpinBoxFrame, widget));
    QRect up(subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, widget));
    up.setBottom(up.bottom()-1);
    QRect down(subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, widget));
    down.setTop(down.top()+1);
//    QRect edit(subControlRect(CC_SpinBox, opt, SC_SpinBoxEditField, widget));

    Render::renderMask(frame.adjusted(1, 1, -1, -1), painter, opt->palette.color(QPalette::Base), 3);
    Render::renderShadow(Render::Sunken, frame, painter, 2, Render::All, 0.33f);
    Ops::drawArrow(painter, opt->palette.color(QPalette::Text), up, Ops::Up);
    Ops::drawArrow(painter, opt->palette.color(QPalette::Text), down, Ops::Down);
    return true;
}
