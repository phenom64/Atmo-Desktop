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
#include <QApplication>

#include "styleproject.h"
#include "stylelib/ops.h"
#include "stylelib/render.h"
#include "stylelib/color.h"
#include "stylelib/animhandler.h"
#include "stylelib/xhandler.h"
#include "stylelib/settings.h"
#include "stylelib/unohandler.h"

bool
StyleProject::drawTab(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
//    return false;
    drawTabShape(option, painter, widget);
    drawTabLabel(option, painter, widget);
    return true;
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
        int o(pixelMetric(PM_TabBarTabOverlap, option, widget));
        QRect r(opt->rect.adjusted(0, 0, 0, !isSelected));
        int leftMargin(o), rightMargin(o);
        if (isFirst || isOnly)
            leftMargin /= 2;
        if ((isLast || isOnly) && bar->expanding())
            rightMargin /= 2;
        if (!bar->expanding())
        {
            --leftMargin;
            ++rightMargin;
        }
        r.setLeft(r.left()-leftMargin);
        r.setRight(r.right()+rightMargin);

        QPainterPath p;
        Render::renderTab(r, painter, isLeftOf ? Render::BeforeSelected : isSelected ? Render::Selected : Render::AfterSelected, &p, Settings::conf.shadows.opacity);
        if (isSelected)
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            QPixmap pix(r.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            UNO::Handler::drawUnoPart(&pt, pix.rect(), bar, bar->mapTo(bar->window(), r.topLeft()), XHandler::opacity());
            pt.end();
            if (XHandler::opacity() < 1.0f)
            {
                painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
                painter->setBrush(Qt::black);
                painter->drawPath(p);
                painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            }
            painter->setBrushOrigin(r.topLeft());
            painter->setBrush(pix);
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
    if (isSelected)
    {
        const QPalette::ColorRole tmpFg(fg);
        fg = bg;
        bg = tmpFg;
    }

    QColor bgc(opt->palette.color(bg));
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    if (option->ENABLED && !(option->state & State_On) && bar)
    {
        int hl(Anim::Tabs::level(bar, bar->tabAt(opt->rect.center())));
        bgc = Color::mid(bgc, sc, STEPS-hl, hl);
    }
    QLinearGradient lg;
    QRect r(opt->rect);
    bool vert(false);
    switch (opt->shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        lg = QLinearGradient(0, 0, 0, Render::maskHeight(Settings::conf.tabs.shadow, r.height()));
        if (isOnly)
            break;
        if (isRtl)
            sides &= ~((!isFirst*Render::Right)|(!isLast*Render::Left));
        else
            sides &= ~((!isFirst*Render::Left)|(!isLast*Render::Right));
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        lg = QLinearGradient(0, 0, Render::maskWidth(Settings::conf.tabs.shadow, r.width()), 0);
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        if (opt->shape != QTabBar::RoundedWest && opt->shape != QTabBar::TriangularWest)
        {
            lg = QLinearGradient(Render::maskWidth(Settings::conf.tabs.shadow, r.width()), 0, 0, 0);
        }
        vert = true;
        if (isOnly)
            break;
        sides &= ~((!isLast*Render::Bottom)|(!isFirst*Render::Top));
        break;
    default: break;
    }
    lg.setStops(Settings::gradientStops(Settings::conf.tabs.gradient, bgc));
    QBrush b(lg);
    Render::drawClickable(Settings::conf.tabs.shadow, r, painter, Settings::conf.tabs.rnd, Settings::conf.shadows.opacity, widget, &b, 0, sides, isSelected);
    const QRect mask(Render::maskRect(Settings::conf.tabs.shadow, r, sides));
    if (isSelected && !isOnly)
    {
        const QPen pen(painter->pen());
        painter->setPen(QColor(0, 0, 0, 32)/*Color::mid(bgc, fgc)*/);
        if (!isFirst)
            painter->drawLine(vert?mask.topLeft():isRtl?mask.topRight():mask.topLeft(), vert?mask.topRight():isRtl?mask.bottomRight():mask.bottomLeft());
        if (!isLast)
            painter->drawLine(vert?mask.bottomLeft():isRtl?mask.topLeft():mask.topRight(), vert?mask.bottomRight():isRtl?mask.bottomLeft():mask.bottomRight());
        painter->setPen(pen);
    }
    else if (opt->selectedPosition != QStyleOptionTab::NextIsSelected && opt->position != QStyleOptionTab::End && !isOnly)
    {
        const QPen pen(painter->pen());
        painter->setPen(QColor(0, 0, 0, 32)/*Color::mid(bgc, fgc)*/);
        painter->drawLine(vert?mask.bottomLeft():isRtl?mask.topLeft():mask.topRight(), vert?mask.bottomRight():isRtl?mask.bottomLeft():mask.bottomRight());
        painter->setPen(pen);
    }
    return true;
}

bool
StyleProject::isVertical(const QStyleOptionTabV3 *tab, const QTabBar *bar)
{
    if (!tab)
        return false;
    QTabBar::Shape tabShape(bar ? bar->shape() : tab ? tab->shape : QTabBar::RoundedNorth);
    switch (tabShape)
    {
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        return true;
    default:
        return false;
    }
}

bool
StyleProject::drawTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    castOpt(TabV3, opt, option);
    if (!opt)
        return true;
    castObj(const QTabBar *, bar, widget);
    painter->save();
//    const bool isFirst(opt->position == QStyleOptionTab::Beginning);
    const bool isOnly(opt->position == QStyleOptionTab::OnlyOneTab);
    const bool isSelected(opt->state & State_Selected || isOnly);
//    const bool isEnd(opt->position == QStyleOptionTab::End);
    const bool leftClose(styleHint(SH_TabBar_CloseButtonPosition, opt, widget) == QTabBar::LeftSide && bar && bar->tabsClosable());
    QRect ir(subElementRect(leftClose?SE_TabBarTabRightButton:SE_TabBarTabLeftButton, opt, widget));
    QRect tr(subElementRect(SE_TabBarTabText, opt, widget));
    int trans(0);
    switch (opt->shape)
    {
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
    if (isSelected && !Ops::isSafariTabBar(qobject_cast<const QTabBar *>(widget)))
        fg = Ops::opposingRole(fg);

    const QFontMetrics fm(painter->fontMetrics());
    Qt::TextElideMode elide((Qt::TextElideMode)styleHint(SH_TabBar_ElideMode, opt, widget));
    if (isVertical(opt, bar))
        elide = Qt::ElideRight;
    const QString text(fm.elidedText(opt->text, elide, tr.width()));
    drawItemText(painter, tr, Qt::AlignCenter, option->palette, option->ENABLED, text, fg);
    painter->restore();
    if (!opt->icon.isNull())
        drawItemPixmap(painter, ir, Qt::AlignCenter, opt->icon.pixmap(opt->iconSize));
    return true;
}

static void renderSafariBar(QPainter *p, const QTabBar *bar, const QColor &c, const Render::Sides sides, const float opacity, QRect rect = QRect())
{
    QRect r(rect.isValid()?rect:bar->rect());
    if (XHandler::opacity() < 1.0f)
    {
        p->setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p->fillRect(r, Qt::black);
        p->setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
    const float o(p->opacity());
    UNO::Handler::drawUnoPart(p, r, bar, bar->mapTo(bar->window(), bar->rect().topLeft()), XHandler::opacity());
    p->setPen(Qt::black);
    p->setOpacity(Settings::conf.shadows.opacity/2);
    p->drawLine(r.topLeft(), r.topRight());
    p->setOpacity(Settings::conf.shadows.opacity);
    p->drawLine(r.bottomLeft(), r.bottomRight());
    p->setOpacity(o);
    Render::renderShadow(Render::Sunken, r, p, 32, Render::Top, Settings::conf.shadows.opacity/2);
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

            renderSafariBar(painter, tabBar, Color::mid((QColor)Color::titleBarColors()[1], Qt::black, 20, 1), sides, XHandler::opacity(), r);
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
            renderSafariBar(painter, tabBar, Color::mid((QColor)Color::titleBarColors()[1], Qt::black, 20, 1), sides, XHandler::opacity(), barRect);
            return true;
        }
    }
    Render::renderShadow(Render::Sunken, rect, painter, 7, sides, 0.2f);
//    Render::renderMask(r, painter, opt->palette.color(QPalette::Window), 4, sides);
    return true;
}

static QPixmap closer[2];

bool
StyleProject::drawTabCloser(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const int size(qMin(option->rect.width(), option->rect.height())), _2_(size/2),_4_(size/4), _8_(size/8), _16_(size/16);
    const QRect line(_2_-_16_, _4_, _8_, size-(_4_*2));

//    const bool isTabBar(widget&&qobject_cast<const QTabBar *>(widget->parentWidget()));
    bool safTabs(false);
    const QTabBar *bar(0);
    if (widget && widget->parentWidget())
    {
        bar = qobject_cast<const QTabBar *>(widget->parentWidget());
        if (bar)
            safTabs = Ops::isSafariTabBar(qobject_cast<const QTabBar *>(bar));
    }

    const bool hover(option->HOVER);

    bool isSelected(option->state & State_Selected);
    if (bar && bar->count() == 1)
        isSelected = true;

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
    if (bar)
    {
        if (safTabs)
        {
            QWidget *d(widget->window());
            const QPalette pal(d->palette());
            isDark = Color::luminosity(pal.color(d->foregroundRole())) > Color::luminosity(pal.color(d->backgroundRole()));
        }
        else
        {
            isDark = Color::luminosity(option->palette.color(bar->foregroundRole())) > Color::luminosity(option->palette.color(bar->backgroundRole()));
            if (isSelected)
                isDark = !isDark;
        }
    }
    int cc(isDark?0:255);
    QPixmap tmp(pix);
    Render::colorizePixmap(tmp, QColor(cc, cc, cc, 127));
    QPixmap tmp2(pix);
    QPalette::ColorRole role(Ops::fgRole(bar, QPalette::WindowText));
    if (safTabs)
        role = QPalette::WindowText;
    else if (hover)
        role = QPalette::Highlight;
    else if (isSelected)
        role = Ops::opposingRole(role);
    Render::colorizePixmap(tmp2, option->palette.color(role));

    QPixmap closer = QPixmap(option->rect.size());
    closer.fill(Qt::transparent);
    p.begin(&closer);
    p.drawTiledPixmap(closer.rect().translated(0, 1), tmp);
    p.drawTiledPixmap(closer.rect(), tmp2);
    p.end();

    if (!isSelected)
        painter->setOpacity(0.75f);
    painter->drawTiledPixmap(option->rect, closer);
    return true;
}
