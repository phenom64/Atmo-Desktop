#include <QDebug>
#include <QTabBar>
#include <QTabWidget>
#include <QStyleOption>
#include <QStyleOptionTab>
#include <QStyleOptionTabBarBase>
#include <QStyleOptionTabBarBaseV2>
#include <QStyleOptionTabV2>
#include <QStyleOptionTabV3>
#include <QStyleOptionTabWidgetFrame>
#include <QStyleOptionTabWidgetFrameV2>
#include <QMainWindow>
#include <QToolBar>
#include <QPaintDevice>
#include <QPainter>

#include "styleproject.h"
#include "stylelib/ops.h"
#include "stylelib/render.h"
#include "stylelib/color.h"

bool
StyleProject::drawTab(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    return false;
//    drawTabShape(option, painter, widget);
//    drawTabLabel(option, painter, widget);
//    return true;
}

bool
StyleProject::drawTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(Tab, opt, option);
    if (!opt)
        return true;
    castObj(const QTabBar *, bar, widget);
    const bool isFirst(opt->position == QStyleOptionTab::Beginning);
    const bool isOnly(opt->position == QStyleOptionTab::OnlyOneTab);
    const bool isLast(opt->position == QStyleOptionTab::End);
    const bool isRtl(opt->direction == Qt::RightToLeft);
    const bool isSelected(opt->state & State_Selected || opt->position == QStyleOptionTab::OnlyOneTab);
    if (Ops::isSafariTabBar(bar))
    {
        const bool isLeftOf(bar->tabAt(opt->rect.center()) < bar->currentIndex());
        int o(pixelMetric(PM_TabBarTabOverlap));
        QRect r(opt->rect.adjusted(0, 0, 0, !isSelected));
        if (!isFirst && !isOnly)
            r.setLeft(r.left()-((isFirst||isOnly)?o/2:o));
        if (!isLast && !isOnly)
            r.setRight(r.right()+((painter->device() == widget)?o:o/2));

        QPainterPath p;
        Render::renderTab(r, painter, isLeftOf ? Render::BeforeSelected : isSelected ? Render::Selected : Render::AfterSelected, &p, 0.25f);
        if (isSelected)
        {
            static QMap<int, QPixmap> s_pix;
            if (!s_pix.contains(r.height()))
            {
                QPixmap pix(Render::noise().width(), r.height());
                pix.fill(Qt::transparent);
                QPainter pt(&pix);
//                pt.fillRect(pix.rect(), Color::titleBarColors[1]);
                QLinearGradient lg(pix.rect().topLeft(), pix.rect().bottomLeft());
                lg.setColorAt(0.0f, Color::titleBarColors[1]);
                lg.setColorAt(1.0f, Color::mid(Color::titleBarColors[1], Qt::black, 8, 1));
                pt.fillRect(pix.rect(), lg);
                pt.end();
                s_pix.insert(r.height(), Render::mid(pix, Render::noise(), 40, 1));
            }
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setBrush(s_pix.value(r.height()));
            painter->setPen(Qt::NoPen);
            painter->drawPath(p);
            painter->restore();
        }
        return true;
    }
    Render::Sides sides = Render::All;
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::ButtonText)), bg(Ops::bgRole(widget, QPalette::Button));
    if (castObj(const QTabWidget *, tw, widget))
        if (const QTabBar *bar = tw->findChild<QTabBar *>())
        {
            fg = bar->foregroundRole();
            bg = bar->backgroundRole();
        }
    QColor bgc(opt->palette.color(bg));
    QColor fgc(opt->palette.color(fg));
    QLinearGradient lg;
    QRect r(opt->rect);
    bool vert(false);
    switch (opt->shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        lg = QLinearGradient(0, 0, 0, r.height());
        if (isOnly)
            break;
        if (isRtl)
            sides &= ~((!isFirst*Render::Right)|(!isLast*Render::Left));
        else
            sides &= ~((!isFirst*Render::Left)|(!isLast*Render::Right));
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        lg = QLinearGradient(0, 0, r.width(), 0);
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        if (opt->shape != QTabBar::RoundedWest && opt->shape != QTabBar::TriangularWest)
            lg = QLinearGradient(r.width(), 0, 0, 0);
        vert = true;
        if (isOnly)
            break;
        sides &= ~((!isLast*Render::Bottom)|(!isFirst*Render::Top));
        break;
    default: break;
    }
    lg.setColorAt(0.0f, Color::mid(bgc, Qt::white, 2, 1));
    lg.setColorAt(1.0f, Color::mid(bgc, Qt::black, 5, 1));
    const QRect mask(r.sAdjusted(1, 1, -1, -2));
    Render::renderMask(mask, painter, lg, 32, sides);
    Render::renderShadow(isSelected?Render::Sunken:Render::Etched, r, painter, 32, sides, 0.66f);
    if (isSelected)
    {
        QPixmap shadow(mask.size());
        shadow.fill(Qt::transparent);
        QPainter p(&shadow);
        Render::renderShadow(Render::Sunken, shadow.rect(), &p, 32, Render::All-sides, 0.33);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Render::renderShadow(Render::Sunken, shadow.rect(), &p, 32, sides);
        p.end();
        painter->drawTiledPixmap(mask, shadow);
    }
    else if (opt->selectedPosition != QStyleOptionTab::NextIsSelected && opt->position != QStyleOptionTab::End)
    {
        const QPen pen(painter->pen());
        painter->setPen(Color::mid(bgc, fgc));
        painter->drawLine(vert?mask.bottomLeft():isRtl?mask.topLeft():mask.topRight(), vert?mask.bottomRight():isRtl?mask.bottomLeft():mask.bottomRight());
        painter->setPen(pen);
    }
    return true;
}

bool
StyleProject::drawTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(TabV3, opt, option);
    castObj(const QTabBar *, bar, widget);
    if (!opt)
        return false;
    painter->save();
//    const bool isFirst(opt->position == QStyleOptionTab::Beginning);
    const bool isOnly(opt->position == QStyleOptionTab::OnlyOneTab);
    const bool isSelected(opt->state & State_Selected || isOnly);
//    const QRect rect(opt->rect);
    QRect tr(subElementRect(SE_TabBarTabText, opt, widget));
    int d(pixelMetric(PM_TabBarTabOverlap));
    if ((isOnly || opt->position == QStyleOptionTab::End) && styleHint(SH_TabBar_CloseButtonPosition, opt, widget) == QTabBar::LeftSide)
        d *= 2;
    QRect ir(tr.adjusted(d, 0, -d, 0));

    if (!opt->icon.isNull())
    {
        if (styleHint(SH_TabBar_CloseButtonPosition, opt, widget) == QTabBar::LeftSide) //icon on right...
        {
            tr.setRight(tr.right()-(opt->iconSize.width()+d));
            ir.setLeft(tr.right());
        }
        else
        {
            tr.setLeft(tr.left()+(opt->iconSize.width()+d));
            ir.setRight(tr.left());
        }
    }
//    QRect lr(subElementRect(SE_TabBarTabLeftButton, opt, widget));
//    QRect rr(subElementRect(SE_TabBarTabRightButton, opt, widget));
    int trans(0);
    switch (opt->shape)
    {
//    case QTabBar::RoundedNorth:
//    case QTabBar::TriangularNorth:
//    case QTabBar::RoundedSouth:
//    case QTabBar::TriangularSouth:
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        trans = -90;
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        trans = 90;
        break;
    default: break;
    }
    if (qAbs(trans))
    {
        painter->translate(tr.center());
        painter->rotate(trans);
        painter->translate(-tr.center());
        const QRect tmp(tr);
        tr.setHeight(tmp.width());
        tr.setWidth(tmp.height());
        tr.moveCenter(tmp.center());
    }

    QFont f(painter->font());
    if (isSelected)
    {
        f.setBold(true);
        painter->setFont(f);
    }
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::WindowText));
    if (opt->state & State_MouseOver && !isSelected)
        fg = QPalette::Highlight;

    Qt::Alignment align = Qt::AlignCenter;
    QFontMetrics fm(f);
    QString text(opt->text);
    if (fm.boundingRect(text).width() > tr.width())
        text = fm.elidedText(text, bar?bar->elideMode():Qt::ElideRight, tr.width());
//        align &= ~Qt::AlignHCenter;
    drawItemText(painter, tr, align, option->palette, option->ENABLED, text, fg);
    if (!opt->icon.isNull())
        drawItemPixmap(painter, ir, Qt::AlignCenter, opt->icon.pixmap(opt->iconSize));
    painter->restore();
    return true;
}

static void renderSafariBar(QPainter *p, const QTabBar *bar, const QColor &c, Render::Sides sides, QRect rect = QRect())
{
    QRect r(rect.isValid()?rect:bar->rect());
    static QMap<int, QPixmap> s_pix;
    if (!s_pix.contains(r.height()))
    {
        QPixmap pix(Render::noise().width(), r.height());
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        pt.fillRect(pix.rect(), c);
        pt.end();
        s_pix.insert(r.height(), Render::mid(pix, Render::noise(), 40, 1));
    }
    p->fillRect(r, s_pix.value(r.height()));
//    r.setBottom(r.bottom()+1);
//    Render::renderShadow(Render::Etched, r, p, 16, Render::Top|Render::Bottom, 0.33f);
    const float o(p->opacity());
    p->setPen(Qt::black);
    p->setOpacity(0.2f);
    p->drawLine(r.topLeft(), r.topRight());
    p->drawLine(r.bottomLeft(), r.bottomRight());
    p->setOpacity(o);
    Render::renderShadow(Render::Sunken, r, p, 16, Render::Top, 0.1f);
}

bool
StyleProject::drawTabBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && widget->parentWidget() && widget->parentWidget()->inherits("KTabWidget"))
        return true;


    castOpt(TabBarBaseV2, opt, option);
    if (!opt)
        return true;

    Render::Sides sides = Render::checkedForWindowEdges(widget);
    if (const QTabBar *tabBar = qobject_cast<const QTabBar *>(widget))
    {
        if (Ops::isSafariTabBar(tabBar))
        {
            QRect r(tabBar->rect());
            if (opt->rect.width() > r.width())
                r = opt->rect;
            renderSafariBar(painter, tabBar, Color::mid(Color::titleBarColors[1], Qt::black, 15, 1), sides, r);
            return true;
        }
    }
    return true;
}

bool
StyleProject::drawTabWidget(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    Render::Sides sides = Render::checkedForWindowEdges(widget);
    castObj(const QTabWidget *, tabWidget, widget);
    castOpt(TabWidgetFrame, opt, option);

    if (!tabWidget || !opt)
        return true;
    const int m(2);
    QRect r(opt->rect.adjusted(m, m, -m, -m));
    QRect rect(opt->rect);
    if (const QTabBar *tabBar = tabWidget->findChild<const QTabBar *>()) //tabBar() method of tabwidget is protected...
    {
        QRect barRect(tabBar->geometry());
        const int h(barRect.height()/2), wh(barRect.width()/2);
        switch (opt->shape)
        {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth:
        {
            barRect.setLeft(tabWidget->rect().left());
            barRect.setRight(tabWidget->rect().right());
            const int b(barRect.bottom()+1);
            r.setTop(b);
            rect.setTop(b-h);
            break;
        }
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
        {
            const int t(barRect.top()-1);
            r.setBottom(t);
            rect.setBottom(t+h);
            break;
        }
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
        {
            const int right(barRect.right()+1);
            r.setLeft(right);
            rect.setLeft(right-wh);
            break;
        }
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
        {
            const int l(barRect.left()-1);
            r.setRight(l);
            rect.setRight(l+wh);
            break;
        }
        default: break;
        }
        if (Ops::isSafariTabBar(tabBar))
        {
            renderSafariBar(painter, tabBar, Color::mid(Color::titleBarColors[1], Qt::black, 15, 1), sides, barRect);
            return true;
        }
    }
    Render::renderShadow(Render::Sunken, rect, painter, 7, sides, 0.33f);
//    Render::renderMask(r, painter, opt->palette.color(QPalette::Window), 4, sides);
    return true;
}

static QPixmap closer[2];

bool
StyleProject::drawTabCloser(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const int size(qMin(option->rect.width(), option->rect.height())), _2_(size/2),_4_(size/4), _8_(size/8), _16_(size/16);
    const QRect line(_2_-_16_, _4_, _8_, size-(_4_*2));


    const bool hover(option->HOVER);
    if (closer[hover].isNull())
    {
        QPixmap pix = QPixmap(option->rect.size());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.setRenderHint(QPainter::Antialiasing);
        QRect l(pix.rect().adjusted(_16_, _16_, -_16_, -_16_));
        p.drawEllipse(l);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        const int r[2] = { 45, 90 };
        for (int i = 0; i < 2; ++i)
        {
            p.translate(_2_, _2_);
            p.rotate(r[i]);
            p.translate(-_2_, -_2_);
            p.drawRect(line);
        }
        p.end();

        bool isDark(false);
        if (widget)
            isDark = Color::luminosity(widget->palette().color(widget->foregroundRole())) > Color::luminosity(widget->palette().color(widget->backgroundRole()));
        int cc(isDark?0:255);
        QPixmap tmp(pix);
        Render::colorizePixmap(tmp, QColor(cc, cc, cc, 127));
        QPixmap tmp2(pix);
        QPalette::ColorRole role(widget?widget->foregroundRole():QPalette::WindowText);
        if (hover)
            role = QPalette::Highlight;
        Render::colorizePixmap(tmp2, option->palette.color(role));

        closer[hover] = QPixmap(option->rect.size());
        closer[hover].fill(Qt::transparent);
        p.begin(&closer[hover]);
        p.drawTiledPixmap(closer[hover].rect().translated(0, 1), tmp);
        p.drawTiledPixmap(closer[hover].rect(), tmp2);
        p.end();
    }
    painter->save();
    if (widget)
        if (castObj(const QTabBar *, bar, widget->parentWidget()))
            if (bar->tabAt(widget->geometry().center()) != bar->currentIndex())
                painter->setOpacity(0.75f);
    painter->drawTiledPixmap(option->rect, closer[hover]);
    painter->restore();
    return true;
}
