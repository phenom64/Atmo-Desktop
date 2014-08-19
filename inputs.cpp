
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
#include "stylelib/animhandler.h"

static void drawSafariLineEdit(const QRect &r, QPainter *p, const QBrush &b, const QStyleOption *opt = 0)
{
    Render::renderMask(r.adjusted(1, 1, -1, -2), p, b);
    QBrush *c = 0;
    bool focus(false);
    if (opt && opt->state & QStyle::State_HasFocus)
    {
        c = new QBrush(opt->palette.color(QPalette::Highlight));
        focus = true;
    }
    Render::renderShadow(Render::Etched, r, p, 32, Render::All, 0.1f, c);
    Render::renderShadow(Render::Sunken, r, p, 32, Render::All, focus?0.6f:0.2f, c);
}

bool
StyleProject::drawLineEdit(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (qobject_cast<const QComboBox *>(widget?widget->parentWidget():0))
        return true;
    if (widget && widget->objectName() == "qt_spinbox_lineedit")
        return true;

    bool inToolBar(false);
    if (const QLineEdit *lineEdit = qobject_cast<const QLineEdit *>(widget))
        if (qobject_cast<const QToolBar *>(lineEdit->parentWidget()))
            inToolBar = true;

    QLinearGradient shadow(0, 0, 0, option->rect.height());
    shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
    shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
    shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
    QBrush b(shadow);
    Render::renderShadow(Render::Simple, option->rect, painter, m_s.input.rnd, Render::All, 1.0f, &b);
    Render::renderMask(option->rect.adjusted(!inToolBar, !inToolBar, -!inToolBar, -1), painter, option->palette.brush(QPalette::Base), m_s.input.rnd-1+inToolBar);

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

    QPalette::ColorRole bg(Ops::bgRole(widget, QPalette::Button))
            , fg(Ops::fgRole(widget, QPalette::ButtonText))
            , ar(QPalette::HighlightedText);

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
//        Render::renderShadow(Render::Raised, frameRect, painter, 5);

        QColor bgc(opt->palette.color(bg));
        QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
        if (option->ENABLED && !(option->state & State_On))
        {
            int hl(Anim::Basic::level(widget));
            bgc = Color::mid(bgc, sc, STEPS-hl, hl);
        }

        QLinearGradient lg(opt->rect.topLeft(), opt->rect.bottomLeft());

        lg.setColorAt(0.0f, Color::mid(bgc, Qt::white, 5, 1));
        lg.setColorAt(1.0f, bgc/*Color::mid(bc, Qt::black, 7, 1)*/);

        Render::renderMask(frameRect, painter, lg, 5);
        QLinearGradient shadow(0, 0, 0, frameRect.height());
        shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
        shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
        shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
        QBrush b(shadow);


        const QColor hc(opt->palette.color(QPalette::Highlight));
        lg.setColorAt(0.0f, Color::mid(hc, Qt::white, 5, 1));
        lg.setColorAt(1.0f, Color::mid(hc, Qt::black, 7, 1));
//        arrowRect.adjust(!ltr?m:0, m, -(ltr?m:0), -m);
        Render::renderMask(arrowRect, painter, lg, 5, Render::All & ~(ltr?Render::Left:Render::Right));
        Render::renderShadow(Render::Simple, frameRect, painter, 5, Render::All, 1.0f, &b);
        painter->setPen(QColor(0, 0, 0, 32));
        const float o(painter->opacity());
        painter->setOpacity(0.5f);
        if (ltr)
            painter->drawLine(arrowRect.topLeft()/*-QPoint(1, 0)*/, arrowRect.bottomLeft()-QPoint(1, 0));
        else
            painter->drawLine(arrowRect.topRight()/*+QPoint(1, 0)*/, arrowRect.bottomRight()+QPoint(1, 0));
        painter->setOpacity(o);
    }
    else
    {
        QBrush brush = opt->palette.brush(bg);
        if (widget)
            if (QLineEdit *l = widget->findChild<QLineEdit *>())
                brush = l->palette().brush(l->backgroundRole());
//        drawSafariLineEdit(opt->rect, painter, brush);
        Render::renderMask(option->rect.adjusted(1, 1, -1, -1), painter, brush, 4);
        QLinearGradient shadow(0, 0, 0, option->rect.height());
        shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
        shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
        shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
        QBrush b(shadow);
        Render::renderShadow(Render::Simple, option->rect, painter, 5, Render::All, 1.0f, &b);
    }

    drawItemPixmap(painter, iconRect, Qt::AlignCenter, opt->currentIcon.pixmap(opt->iconSize));
    m*=2;
    Ops::drawArrow(painter, opt->palette.color(opt->editable?QPalette::Text:QPalette::HighlightedText), arrowRect.adjusted(m*1.25f, m, -m, -m), Ops::Down, Qt::AlignCenter, 7);
    return true;
}

bool
StyleProject::drawComboBoxLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(ComboBox, opt, option);
    if (!opt || opt->editable)
        return true;
    QPalette::ColorRole /*bg(Ops::bgRole(widget, QPalette::Button)),*/ fg(Ops::fgRole(widget, QPalette::ButtonText));
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
    QRect up(subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, widget).shrinked(2));
//    up.setBottom(up.bottom()-1);
    QRect down(subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, widget).shrinked(2));
//    down.setTop(down.top()+1);
//    QRect edit(subControlRect(CC_SpinBox, opt, SC_SpinBoxEditField, widget));

    Render::renderMask(option->rect, painter, option->palette.brush(QPalette::Base), m_s.input.rnd);
    QLinearGradient shadow(0, 0, 0, option->rect.height());
    shadow.setColorAt(0.0f, QColor(0, 0, 0, 32));
    shadow.setColorAt(0.8f, QColor(0, 0, 0, 32));
    shadow.setColorAt(1.0f, QColor(0, 0, 0, 92));
    QBrush b(shadow);
    Render::renderShadow(Render::Simple, option->rect, painter, m_s.input.rnd, Render::All, 1.0f, &b);

    Ops::drawArrow(painter, opt->palette.color(QPalette::Text), up, Ops::Up);
    Ops::drawArrow(painter, opt->palette.color(QPalette::Text), down, Ops::Down);
    return true;
}
