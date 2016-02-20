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
#include <QStyleOptionToolBoxV2>
#include <QToolBox>
#include <QStatusBar>

#include "dsp.h"
#include "stylelib/ops.h"
#include "stylelib/gfx.h"
#include "stylelib/color.h"
#include "stylelib/animhandler.h"
#include "stylelib/xhandler.h"
#include "config/settings.h"
#include "stylelib/handlers.h"
#include "stylelib/fx.h"

using namespace DSP;

static QPixmap s_tabBarShadow;

bool
Style::drawTab(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    drawTabShape(option, painter, widget);
    drawTabLabel(option, painter, widget);
    return true;
}

bool
Style::drawSafariTab(const QStyleOptionTab *opt, QPainter *painter, const QTabBar *bar) const
{
    const bool isFirst(opt->position == QStyleOptionTab::Beginning),
            isOnly(opt->position == QStyleOptionTab::OnlyOneTab),
            isLast(opt->position == QStyleOptionTab::End),
            isSelected(opt->state & State_Selected),
            isLeftOf(bar->tabAt(opt->rect.center()) < bar->currentIndex());
    int o(pixelMetric(PM_TabBarTabOverlap, opt, bar));
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
    GFX::drawTab(r, painter, isLeftOf ? BeforeSelected : isSelected ? Selected : AfterSelected, &p);
    if (isSelected)
    {
        const bool hadAA(painter->testRenderHint(QPainter::Antialiasing));
        painter->setRenderHint(QPainter::Antialiasing);
        QPixmap pix(r.size());
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        Handlers::Window::drawUnoPart(&pt, pix.rect(), bar, bar->mapTo(bar->window(), r.topLeft()));
        pt.end();
        if (XHandler::opacity() < 1.0f)
        {
            const QPainter::CompositionMode mode(painter->compositionMode());
            painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
            painter->fillPath(p, Qt::black);
            painter->setCompositionMode(mode);
        }
        painter->setBrushOrigin(r.topLeft());
        painter->fillPath(p, pix);
        painter->setRenderHint(QPainter::Antialiasing, hadAA);
    }
    return true;
}

bool
Style::drawSelector(const QStyleOptionTab *opt, QPainter *painter, const QTabBar *bar) const
{
    const bool isFirst(opt->position == QStyleOptionTab::Beginning),
            isOnly(opt->position == QStyleOptionTab::OnlyOneTab),
            isLast(opt->position == QStyleOptionTab::End),
            isRtl(opt->direction == Qt::RightToLeft),
            isSelected(opt->state & State_Selected);

    Sides sides(All);
    QPalette::ColorRole fg(Ops::fgRole(bar, QPalette::ButtonText)),
            bg(Ops::bgRole(bar, QPalette::Button));

    if (isSelected)
        Ops::swap(fg, bg);

    QColor bgc(opt->palette.color(bg));
    QColor sc = Color::mid(bgc, opt->palette.color(QPalette::Highlight), 2, 1);
    const int hl = Anim::Tabs::level(bar, bar->tabAt(opt->rect.center()));
    if (opt->ENABLED && !(opt->state & State_On) && bar)
        bgc = Color::mid(bgc, sc, Steps-hl, hl);
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
            sides &= ~((!isFirst*Right)|(!isLast*Left));
        else
            sides &= ~((!isFirst*Left)|(!isLast*Right));
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        lg = QLinearGradient(0, 0, r.width(), 0);
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        if (opt->shape != QTabBar::RoundedWest && opt->shape != QTabBar::TriangularWest)
        {
            lg = QLinearGradient(r.width(), 0, 0, 0);
        }
        vert = true;
        if (isOnly)
            break;
        sides &= ~((!isLast*Bottom)|(!isFirst*Top));
        break;
    default: break;
    }
    lg.setStops(DSP::Settings::gradientStops(dConf.tabs.gradient, bgc));
    GFX::drawClickable(dConf.tabs.shadow, r, painter, lg, dConf.tabs.rnd, hl, sides, opt, bar);
    static const quint8 sm = GFX::shadowMargin(dConf.tabs.shadow);
    const QRect mask(r.sAdjusted(sm, sm, -sm, -sm));
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
Style::drawTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionTabV3 *opt = qstyleoption_cast<const QStyleOptionTabV3 *>(option);
    if (!opt)
        return true;
    if (opt->position == QStyleOptionTab::OnlyOneTab) //if theres only one tab then thats the selected one right? right? am I missing something?
        const_cast<QStyleOption *>(option)->state |= State_Selected;
    const QTabBar *bar = qobject_cast<const QTabBar *>(widget);
    if (Ops::isSafariTabBar(bar))
        return drawSafariTab(opt, painter, bar);
    if (!opt->documentMode && !dConf.tabs.regular)
        return drawSelector(opt, painter, bar);
    return drawDocumentTabShape(option, painter, widget);

    const bool isSelected(opt->state & State_Selected);
    QColor c(opt->palette.color(widget?widget->backgroundRole():QPalette::Window));

    bool aboveStatusBar(false);
    QRect r(opt->rect);
    QLine s;
    QLinearGradient lg;
    static const int m(1);
    switch (opt->shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
        r.setTop(r.top()+m);
        if (!isSelected)
            r.setBottom(r.bottom()-1);
        s.setPoints(r.topRight(), r.bottomRight());
        lg = QLinearGradient(r.topLeft(), r.bottomLeft());
        break;
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        if (dConf.uno.enabled && bar)
        {
            const QPoint below(bar->mapTo(bar->window(), bar->rect().bottomRight()+QPoint(0, 2)));
            if (qobject_cast<QStatusBar *>(bar->window()->childAt(below)))
                aboveStatusBar = true;
        }
        if (!aboveStatusBar)
            r.setBottom(r.bottom()-m);
        if (!isSelected)
            r.setTop(r.top()+1);
        s.setPoints(r.topRight(), r.bottomRight());
        lg = QLinearGradient(r.bottomLeft(), r.topLeft());
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        r.setLeft(r.left()+m);
        if (!isSelected)
            r.setRight(r.right()-1);
        s.setPoints(r.bottomLeft(), r.bottomRight());
        lg = QLinearGradient(r.topLeft(), r.topRight());
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        r.setRight(r.right()-m);
        if (!isSelected)
            r.setLeft(r.left()+1);
        s.setPoints(r.bottomLeft(), r.bottomRight());
        lg = QLinearGradient(r.topRight(), r.topLeft());
        break;
    default: break;
    }
    const QPen pen(painter->pen());
    if (isSelected)
    {
        if (bar)
        {
            const QPoint tl(bar->mapTo(bar->window(), r.topLeft()));
            widget->window()->render(painter, tl, QRegion(QRect(tl, r.size())), QWidget::DrawWindowBackground);
            lg.setColorAt(0.0f, QColor(255, 255, 255, 63));
            lg.setColorAt(1.0f, Qt::transparent);
            painter->fillRect(r, lg);
        }
        else
            painter->fillRect(r, c);
    }
    if (r.right()+8 < painter->device()->width())
    {
        painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        painter->drawLine(s);
    }
    painter->setPen(pen);
    return true;
}

bool
Style::isVertical(const QStyleOptionTabV3 *tab, const QTabBar *bar)
{
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
Style::drawTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionTabV3 *opt = qstyleoption_cast<const QStyleOptionTabV3 *>(option);
    if (!opt)
        return true;

    int trans(0);
    switch (opt->shape)
    {
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest: trans = -90; break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast: trans = 90; break;
    default: break;
    }

    const QTabBar *bar = qobject_cast<const QTabBar *>(widget);
    const bool leftClose(styleHint(SH_TabBar_CloseButtonPosition, opt, widget) == QTabBar::LeftSide && bar && bar->tabsClosable());
    QRect ir(subElementRect(leftClose?SE_TabBarTabRightButton:SE_TabBarTabLeftButton, opt, widget));
    QRect tr(subElementRect(SE_TabBarTabText, opt, widget));

    painter->save();
    if (trans)
    {
        painter->translate(tr.center());
        painter->rotate(trans);
        painter->translate(-tr.center());
        QSize sz(tr.size());
        const QPoint center(tr.center());
        sz.transpose();
        tr.setSize(sz);
        tr.moveCenter(center);
    }

    const bool selected = isSelected(opt);
    QFont f(painter->font());
    if (selected)
    {
        f.setBold(true);
        painter->setFont(f);
    }

    QPalette::ColorRole fg = widget ? widget->foregroundRole() : QPalette::ButtonText;
    const bool safari(Ops::isSafariTabBar(bar));
    if (selected && !safari && !(bar && bar->documentMode()) && !dConf.tabs.regular)
        fg = Ops::opposingRole(fg);

    const QFontMetrics &fm = painter->fontMetrics();
    const Qt::TextElideMode elide = (Qt::TextElideMode)styleHint(SH_TabBar_ElideMode, opt, widget);
    const QString &text = fm.elidedText(opt->text, elide, tr.width(), Qt::TextShowMnemonic);
    drawItemText(painter, tr, Qt::AlignCenter, option->palette, isEnabled(opt), text, fg);
    if (!opt->icon.isNull())
        drawItemPixmap(painter, ir, Qt::AlignCenter, opt->icon.pixmap(opt->iconSize));
    painter->restore();
    return true;
}

bool
Style::drawDocumentTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionTabV3 *opt = qstyleoption_cast<const QStyleOptionTabV3 *>(option);
    if (!opt)
        return true;
    if (opt->position == QStyleOptionTab::OnlyOneTab) //if theres only one tab then thats the selected one right? right? am I missing something?
        const_cast<QStyleOption *>(option)->state |= State_Selected;

    const QTabBar *bar = qobject_cast<const QTabBar *>(widget);
    const bool selected = isSelected(opt);
    const bool first = opt->position == QStyleOptionTabV3::Beginning || opt->position == QStyleOptionTabV3::OnlyOneTab;
    const bool last = opt->position == QStyleOptionTabV3::End;
    const int level = selected ? 0 : (bar ? Anim::Tabs::level(bar, bar->tabAt(opt->rect.topLeft())) : 0);

    QColor c = opt->palette.color(widget?widget->backgroundRole():QPalette::Button);
    if (dConf.differentInactive && Handlers::Window::isActiveWindow(widget))
        c = c.darker(110);

    bool beforeSelected(false);
    const bool vertical = isVertical(opt, bar);
    QRect r(opt->rect);
    QLinearGradient lg;
    Sides sides(All);
    const quint8 margin(!selected&&!opt->documentMode * 2);
    const bool ltr = opt->direction == Qt::LeftToRight;
    switch (opt->shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
        lg = QLinearGradient(r.topLeft(), r.bottomLeft());
        if (!selected && !opt->documentMode)
            r.setTop(r.top()+InactiveTabOffset);
        r.setBottom(r.bottom()-margin);
        sides &= ~Bottom;
        beforeSelected = bar && bar->tabAt(opt->rect.center()) < bar->currentIndex();
        if (opt->documentMode && first)
        {
            r.setLeft(r.left() + ltr*TabDocModePadding);
            r.setRight(r.right() - !ltr*TabDocModePadding);
        }
        else if (opt->documentMode && last)
        {
            r.setRight(r.right() - ltr*TabDocModePadding);
            r.setLeft(r.left() + !ltr*TabDocModePadding);
        }
        break;
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        lg = QLinearGradient(r.bottomLeft(), r.topLeft());
        if (!selected && !opt->documentMode)
            r.setBottom(r.bottom()-InactiveTabOffset);
        r.setTop(r.top()+margin);
        sides &= ~Top;
        beforeSelected = bar && bar->tabAt(opt->rect.center()) > bar->currentIndex();
        if (opt->documentMode && first)
        {
            r.setLeft(r.left() + ltr*TabDocModePadding);
            r.setRight(r.right() - !ltr*TabDocModePadding);
        }
        else if (opt->documentMode && last)
        {
            r.setRight(r.right() - ltr*TabDocModePadding);
            r.setLeft(r.left() + !ltr*TabDocModePadding);
        }
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        lg = QLinearGradient(r.topLeft(), r.topRight());
        if (!selected && !opt->documentMode)
            r.setLeft(r.left()+InactiveTabOffset);
        r.setRight(r.right()-margin);
        sides &= ~Right;
        beforeSelected = bar && bar->tabAt(opt->rect.center()) > bar->currentIndex();
        if (opt->documentMode && first)
            r.setTop(r.top() + TabDocModePadding);
        else if (opt->documentMode && last)
            r.setBottom(r.bottom() - TabDocModePadding);
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        lg = QLinearGradient(r.topRight(), r.topLeft());
        if (!selected && !opt->documentMode)
            r.setRight(r.right()-InactiveTabOffset);
        r.setLeft(r.left()+margin);
        sides &= ~Left;
        beforeSelected = bar && bar->tabAt(opt->rect.center()) < bar->currentIndex();
        if (opt->documentMode && first)
            r.setTop(r.top() + TabDocModePadding);
        else if (opt->documentMode && last)
            r.setBottom(r.bottom() - TabDocModePadding);
        break;
    default: break;
    }
    if (!ltr && !vertical)
        beforeSelected = !beforeSelected && !selected;


    const TabPos pos = beforeSelected?BeforeSelected:selected?Selected:AfterSelected;
    if (opt->documentMode)
    {
        lg.setStops(Settings::gradientStops(dConf.tabs.gradient, c));
        Mask::Tab::drawTab(Chrome, pos, r, painter, lg, qstyleoption_cast<const QStyleOptionTabV3 *>(opt), level);
    }
    else
    {
        QColor h = Color::mid(opt->palette.color(QPalette::Highlight), c, 1, 2);
        c = Color::mid(c, h, Steps-level, level);
        lg.setStops(Settings::gradientStops(dConf.tabs.gradient, c));
        GFX::drawClickable(dConf.tabs.shadow, Mask::Tab::tabRect(r, pos, opt->shape), painter, lg, dConf.tabs.rnd, level, sides);
    }
    return true;
}

static void drawDocTabBar(QPainter *p, const QTabBar *bar, QRect rect)
{
    if (!bar->isVisible())
        return;
    QRect r(rect.isValid()?rect:bar->rect());
    if (Ops::isUnoTabBar(bar))
    {
        if (XHandler::opacity() < 1.0f)
        {
            p->setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p->fillRect(r, Qt::black);
            p->setCompositionMode(QPainter::CompositionMode_SourceOver);
        }

        Handlers::Window::drawUnoPart(p, r, bar, bar->mapTo(bar->window(), bar->rect().topLeft()));
        if (Ops::isSafariTabBar(bar))
        {
            const bool hadAA(p->testRenderHint(QPainter::Antialiasing));
            p->setRenderHint(QPainter::Antialiasing, false);
            p->fillRect(r, QColor(0, 0, 0, 15));
            p->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
            p->drawLine(r.bottomLeft(), r.bottomRight());
            GFX::drawTabBarShadow(p, r);
            p->setRenderHint(QPainter::Antialiasing, hadAA);
            return;
        }

    }
    if (bar->documentMode() || dConf.tabs.regular)
    {
        QLinearGradient lg;
        Sides sides(All);
        switch (bar->shape())
        {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth: lg = QLinearGradient(r.topLeft(), r.bottomLeft()); sides &= ~(Left|Right); break;
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth: lg = QLinearGradient(r.bottomLeft(), r.topLeft()); sides &= ~(Left|Right); break;
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest: lg = QLinearGradient(r.topLeft(), r.topRight()); sides &= ~(Top|Bottom); break;
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast: lg = QLinearGradient(r.topRight(), r.topLeft()); sides &= ~(Top|Bottom); break;
        default: break;
        }
        QColor c = bar->palette().color(bar->backgroundRole());
        if (dConf.differentInactive && Handlers::Window::isActiveWindow(bar))
            c = c.darker(110);
        lg.setStops(Settings::gradientStops(dConf.tabs.gradient, c));
        GFX::drawClickable(Rect, Mask::Tab::tabBarRect(r, BeforeSelected, bar->shape()), p, lg, 2, 0, sides);
    }
}

bool
Style::drawTabBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (widget && widget->parentWidget() && widget->parentWidget()->inherits("KTabWidget"))
        return true;
    const QStyleOptionTabBarBaseV2 *opt = qstyleoption_cast<const QStyleOptionTabBarBaseV2 *>(option);
    if (!opt)
        return true;
    const QTabBar *tabBar = qobject_cast<const QTabBar *>(widget);
    if (tabBar && tabBar->documentMode())
    {
        QRect r(tabBar->rect());
        QPaintDevice *d = painter->device();
        r = tabBar->geometry();
        if (isVertical(0, tabBar))
        {
            r.setTop(0);
            r.setBottom(d->height());
        }
        else
        {
            r.setLeft(0);
            r.setRight(d->width());
        }
        drawDocTabBar(painter, tabBar, r);
        return true;
    }
    return true;
}

bool
Style::drawTabWidget(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QTabWidget *tabWidget = qobject_cast<const QTabWidget *>(widget);
    const QStyleOptionTabWidgetFrame *opt = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option);

    if (!tabWidget || !opt)
        return true;

    QRect rect(opt->rect);

#if QT_VERSION < 0x050000
    const QTabBar *tabBar = tabWidget->findChild<const QTabBar *>(); //huh? why is the tabBar() protected??
#else
    const QTabBar *tabBar = tabWidget->tabBar();
#endif
    Sides sides(All);
    if (tabBar)
    {
        QRect barRect(tabBar->geometry());
        static const quint8 regularTabOffset = 0/*GFX::shadowMargin(Raised) + 1*/;
        const int h(dConf.tabs.regular ? regularTabOffset : (barRect.height()>>1));
        const int wh(dConf.tabs.regular ? regularTabOffset : (barRect.width()>>1));
        static const quint8 margin(1/*!dConf.tabs.regular*/);
        switch (opt->shape)
        {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth:
        {
            barRect.setLeft(tabWidget->rect().left());
            barRect.setRight(tabWidget->rect().right());
            const int b(barRect.bottom()+margin);
            rect.setTop(b-h);
            sides &= ~Top;
            break;
        }
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
        {
            barRect.setLeft(tabWidget->rect().left());
            barRect.setRight(tabWidget->rect().right());
            const int t(barRect.top()-margin);
            rect.setBottom(t+h);
            sides &= ~Bottom;
            break;
        }
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
        {
            barRect.setTop(tabWidget->rect().top());
            barRect.setBottom(tabWidget->rect().bottom());
            const int right(barRect.right()+margin);
            rect.setLeft(right-wh);
            sides &= ~Left;
            break;
        }
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
        {
            barRect.setTop(tabWidget->rect().top());
            barRect.setBottom(tabWidget->rect().bottom());
            const int l(barRect.left()-margin);
            rect.setRight(l+wh);
            sides &= ~Right;
            break;
        }
        default: break;
        }
        drawDocTabBar(painter, tabBar, barRect);
    }
    if (dConf.tabs.regular)
        GFX::drawShadow(Raised, rect, painter, isEnabled(opt), 3, sides);
    if (!tabWidget->documentMode() && !dConf.tabs.regular)
        GFX::drawShadow(Sunken, rect, painter, isEnabled(opt), 7, All);
    return true;
}

bool
Style::drawTabCloser(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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
        if (safTabs || bar->documentMode())
        {
            QWidget *d(widget->window());
            const QPalette pal(d->palette());
            isDark = Color::lum(pal.color(d->foregroundRole())) > Color::lum(pal.color(d->backgroundRole()));
        }
        else
        {
            isDark = Color::lum(option->palette.color(bar->foregroundRole())) > Color::lum(option->palette.color(bar->backgroundRole()));
            if (isSelected)
                isDark = !isDark;
        }
    }
    int cc(isDark?0:255);
    QPixmap tmp(pix);
    FX::colorizePixmap(tmp, QColor(cc, cc, cc, 127));
    QPixmap tmp2(pix);
    QPalette::ColorRole role(Ops::fgRole(bar, QPalette::WindowText));
    if (safTabs || (bar && bar->documentMode()))
        role = QPalette::WindowText;
    else if (hover)
        role = QPalette::Highlight;
    else if (isSelected)
        role = Ops::opposingRole(role);
    FX::colorizePixmap(tmp2, option->palette.color(role));

    QPixmap closer = QPixmap(option->rect.size());
    closer.fill(Qt::transparent);
    p.begin(&closer);
    p.drawTiledPixmap(closer.rect().translated(0, 1), tmp);
    p.drawTiledPixmap(closer.rect(), tmp2);
    if (!isSelected && !(option->state & State_MouseOver))
    {
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(closer.rect(), QColor(0, 0, 0, 127.0f));
    }
    p.end();
    painter->drawPixmap(option->rect, closer);
    return true;
}

bool
Style::drawToolBoxTab(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (option && widget && widget->parentWidget())
        const_cast<QStyleOption *>(option)->palette = widget->parentWidget()->palette();
    drawToolBoxTabShape(option, painter, widget);
    drawToolBoxTabLabel(option, painter, widget);
    return true;
}

bool
Style::drawToolBoxTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolBoxV2 *opt = qstyleoption_cast<const QStyleOptionToolBoxV2 *>(option);
    if (!opt)
        return true;
    const QPalette pal(opt->palette);
//    QRect geo(opt->rect);
//    QWidget *w(0);
    QRect theRect(opt->rect);
//    const bool first(opt->position == QStyleOptionToolBoxV2::Beginning);
    const bool last(opt->position == QStyleOptionToolBoxV2::End);
//    if (const QToolBox *box = qobject_cast<const QToolBox *>(widget))
//    {
//        if (box->frameShadow() == QFrame::Sunken)
//        {
            painter->fillRect(option->rect, pal.button());
            const QPen saved(painter->pen());
            painter->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
            if (option->SUNKEN || !last)
                painter->drawLine(theRect.bottomLeft(), theRect.bottomRight());
            if (opt->selectedPosition == QStyleOptionToolBoxV2::PreviousIsSelected)
                painter->drawLine(theRect.topLeft(), theRect.topRight());
            painter->setPen(saved);
            return true;
//        }
//        const QList<QWidget *> tabs(widget->findChildren<QWidget *>("qt_toolbox_toolboxbutton"));
//        for (int i = 0; i < tabs.size(); ++i)
//        {
//            QWidget *tab(tabs.at(i));
//            if (tab == painter->device())
//            {
//                w = tab;
//                geo = w->geometry();
//            }
//            if (tab->geometry().bottom() > theRect.bottom())
//                theRect.setBottom(tab->geometry().bottom());
//        }
//    }

//    Sides sides(All);
//    if (!first)
//        sides &= ~Top;
//    if (!last)
//        sides &= ~Bottom;

//    QLinearGradient lg(theRect.topLeft(), theRect.bottomLeft());
//    lg.setStops(DSP::Settings::gradientStops(dConf.tabs.gradient, pal.color(QPalette::Button)));
//    GFX::drawClickable(dConf.tabs.shadow, theRect, painter, lg, qMin<int>(opt->rect.height()/2, dConf.tabs.rnd), sides, option, w);
//    return true;
}

bool
Style::drawToolBoxTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionToolBoxV2 *opt = qstyleoption_cast<const QStyleOptionToolBoxV2 *>(option);
    if (!opt)
        return true;

    const QPalette pal(widget?widget->palette():opt->palette);
    const bool isSelected(opt->state & State_Selected || opt->position == QStyleOptionTab::OnlyOneTab);
    QRect textRect(opt->rect.adjusted(4, 0, -4, 0));
    if (!opt->icon.isNull())
    {
        const QSize iconSize(16, 16); //no way to get this?
        const QPoint topLeft(textRect.topLeft());
        QRect r(topLeft+QPoint(0, textRect.height()/2-iconSize.height()/2), iconSize);
        opt->icon.paint(painter, r);
        textRect.setLeft(r.right());
    }
    const QFont savedFont(painter->font());
    if (isSelected)
    {
        QFont tmp(savedFont);
        tmp.setBold(true);
        painter->setFont(tmp);
    }
    drawItemText(painter, textRect, Qt::AlignCenter, pal, opt->ENABLED, opt->text, QPalette::ButtonText);
    if (isSelected)
    {
        painter->setClipRegion(QRegion(opt->rect)-QRegion(itemTextRect(painter->fontMetrics(), textRect, Qt::AlignCenter, true, opt->text)));
        int y(textRect.center().y());
        const QPen pen(painter->pen());
        QLinearGradient lg(textRect.topLeft(), textRect.topRight());
        lg.setColorAt(0.0f, Qt::transparent);
        lg.setColorAt(0.5f, pal.color(QPalette::ButtonText));
        lg.setColorAt(1.0f, Qt::transparent);
        painter->setPen(QPen(lg, 1.0f));
        painter->drawLine(textRect.x(), y, textRect.right(), y);
        painter->setPen(pen);
    }
    painter->setFont(savedFont);
    return true;
}
