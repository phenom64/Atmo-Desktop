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
//    return false;
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
    GFX::drawTab(r, painter, isLeftOf ? BeforeSelected : isSelected ? Selected : AfterSelected, &p, dConf.shadows.opacity);
    if (isSelected)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        QPixmap pix(r.size());
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        Handlers::Window::drawUnoPart(&pt, pix.rect(), bar, bar->mapTo(bar->window(), r.topLeft()));
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
    if (opt->ENABLED && !(opt->state & State_On) && bar)
    {
        int hl(Anim::Tabs::level(bar, bar->tabAt(opt->rect.center())));
        bgc = Color::mid(bgc, sc, Steps-hl, hl);
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
    GFX::drawClickable(dConf.tabs.shadow, r, painter, lg, dConf.tabs.rnd, sides, opt, bar);
    const quint8 sm = GFX::shadowMargin(dConf.tabs.shadow);
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

static QPainterPath tabPath(QRect rect, int r = 8)
{
//    r = qMin(r, rect.height()/2);
//    int x1, y1, x2, y2;
//    rect.getCoords(&x1, &y1, &x2, &y2);
//    QPainterPath path;
//    path.moveTo(x1, y2);
//    path.quadTo(x1+r, y2, x1+r, y2-r);
//    path.lineTo(x1+r, y1+r);
//    path.quadTo(x1+r, y1, x1+(r*2), y1);
//    path.lineTo(x2-(r*2), y1);
//    path.quadTo(x2-r, y1, x2-r, y1+r);
//    path.lineTo(x2-r, y2-r);
//    path.quadTo(x2-r, y2, x2, y2);
//    path.closeSubpath();

    int x = rect.x(), y = rect.y(), w = x+(rect.width()-1), h = y+(rect.height()-1);
    QPainterPath path;
//    if (shape == Standard)
//    {
//        path.moveTo(x, h);
//        path.quadTo(x+(round), h, x+round, h-round);
//        path.lineTo(x+round, y+round);
//        path.quadTo(x+round, y, x+(round*2), y);
//        path.lineTo(w-(round*2), y);
//        path.quadTo(w-round, y, w-round, y+round);
//        path.lineTo(w-round, h-round);
//        path.quadTo(w-round, h, w, h);
//    }
//    else if (shape == Chrome)
//    {
        int half = h/2, hf = r/2;
        path.moveTo(x, h);
        path.quadTo(x+hf, h, x+r, half);
        path.quadTo(x+r+hf, y, x+(r*2), y);
        path.lineTo(w-(r*2), y);
        path.quadTo(w-(r+hf), y, w-r, half);
        path.quadTo(w-hf, h, w, h);
//    }

    return path;
}

bool
Style::drawTabShape(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionTab *opt = qstyleoption_cast<const QStyleOptionTab *>(option);
    if (!opt)
        return true;
    if (opt->position == QStyleOptionTab::OnlyOneTab) //if theres only one tab then thats the selected one right? right? am I missing something?
        const_cast<QStyleOption *>(option)->state |= State_Selected;
    const QTabBar *bar = qobject_cast<const QTabBar *>(widget);
    if (Ops::isSafariTabBar(bar))
        return drawSafariTab(opt, painter, bar);
    if (styleHint(SH_TabBar_Alignment, option, widget) == Qt::AlignCenter)
        return drawSelector(opt, painter, bar);

    const bool isFirst(opt->position == QStyleOptionTab::Beginning),
            isOnly(opt->position == QStyleOptionTab::OnlyOneTab),
            isLast(opt->position == QStyleOptionTab::End),
            isRtl(opt->direction == Qt::RightToLeft),
            isSelected(opt->state & State_Selected);
    const int topMargin(!isSelected*2);
    const int level(isSelected?0:bar?Anim::Tabs::level(bar, bar->tabAt(opt->rect.topLeft())):0);
    const int index(bar->tabAt(opt->rect.topLeft()));
    const bool beforeSelected(index<bar->currentIndex());
    const bool afterSelected(index>bar->currentIndex());
    static const int ext(16);

//    QSize sz(opt->rect.size()+QSize(ext, 1));
#if 0
    static QMap<int, QVector<QPixmap> > s_shadows;
    enum TabShadowPart { LeftBeforeSelected = 0, RightBeforeSelected, LeftSelected, RightSelected, LeftAfterSelected, RightAfterSelected };
    if (!s_shadows.contains(sz.height()))
    {
        static const int round(8);
        QImage img(round*2+1, sz.height(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        const int br(2);
        QPainterPath path(tabPath(img.rect().adjusted(0, br, 0, 0)));
        QPainter pt(&img);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.fillPath(path, Qt::black);
        pt.end();

        Render::expblur(img, br);

        pt.begin(&img);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.fillPath(path, Qt::black);
        pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
        pt.setRenderHint(QPainter::Antialiasing, false);

        QPoint offset = afterSelected ? img.rect().topLeft()-QPoint(img.width()-(ext), 0) : img.rect().topRight()-QPoint(ext, 0);
        if (!isSelected && !isOnly)
        {
            pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            pt.fillPath(path.translated(offset), Qt::black); //erase the side
            const QRect winBgRect(img.rect().adjusted(0, img.height()-s_tabBarShadow.height(), 0, 0));
            pt.drawTiledPixmap(winBgRect, s_tabBarShadow);
        }
        pt.end();
    }
#elseif 0  //a more chrome-like approach

    QImage img(sz, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    const int br(2);
    QPainterPath path(tabPath(img.rect().adjusted(0, br, 0, 0)));
    QPainter pt(&img);
    pt.setRenderHint(QPainter::Antialiasing);
    pt.fillPath(path, Color::mid(Qt::black, opt->palette.color(QPalette::Highlight), STEPS-level, level));
    pt.end();
    Render::expblur(img, br);
    pt.begin(&img);

    const QRect geo(bar->mapTo(widget->window(), opt->rect.topLeft()-QPoint(ext/2, 0)), img.size());
    QPixmap winBg(img.size());
    widget->window()->render(&winBg, QPoint(), geo, QWidget::DrawWindowBackground);

    if (s_tabBarShadow.isNull())
        s_tabBarShadow = QPixmap::fromImage(img.copy(img.width()/2, 0, 1, 2));

    pt.setRenderHint(QPainter::Antialiasing);
    if (isSelected)
        pt.fillPath(path, winBg);
    else
    {
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.fillPath(path, Qt::black);
        pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
    pt.setRenderHint(QPainter::Antialiasing, false);

    QPoint offset = afterSelected ? img.rect().topLeft()-QPoint(img.width()-(ext), 0) : img.rect().topRight()-QPoint(ext, 0);
    if (!isSelected && !isOnly)
    {
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.fillPath(path.translated(offset), Qt::black); //erase the side
        const QRect winBgRect(img.rect().adjusted(0, img.height()-s_tabBarShadow.height(), 0, 0));
        pt.drawTiledPixmap(winBgRect, s_tabBarShadow);
    }

    pt.end();
    painter->drawImage(opt->rect.topLeft()-QPoint(ext/2, 0), img);
#endif
    QColor c(opt->palette.color(widget?widget->backgroundRole():QPalette::Window));
//    if (!isSelected)
//        c = Color::mid(c, opt->palette.color(widget?widget->foregroundRole():QPalette::WindowText), 5, 1);

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
//        l.setPoints(r.topLeft(), r.topRight());
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
//        l.setPoints(r.bottomLeft(), r.bottomRight());
        s.setPoints(r.topRight(), r.bottomRight());
        lg = QLinearGradient(r.bottomLeft(), r.topLeft());
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        r.setLeft(r.left()+m);
        if (!isSelected)
            r.setRight(r.right()-1);
//        l.setPoints(r.topLeft(), r.bottomLeft());
        s.setPoints(r.bottomLeft(), r.bottomRight());
        lg = QLinearGradient(r.topLeft(), r.topRight());
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        r.setRight(r.right()-m);
        if (!isSelected)
            r.setLeft(r.left()+1);
//        l.setPoints(r.topRight(), r.bottomRight());
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
//    painter->setPen(QColor(255, 255, 255, 192));
//    painter->drawLine(l);

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
Style::drawTabLabel(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QStyleOptionTabV3 *opt = qstyleoption_cast<const QStyleOptionTabV3 *>(option);
    if (!opt)
        return true;
    const QTabBar *bar = qobject_cast<const QTabBar *>(widget);
    painter->save();
    const bool isSelected(opt->state & State_Selected),
            leftClose(styleHint(SH_TabBar_CloseButtonPosition, opt, widget) == QTabBar::LeftSide && bar && bar->tabsClosable());
    QRect ir(subElementRect(leftClose?SE_TabBarTabRightButton:SE_TabBarTabLeftButton, opt, widget));
    QRect tr(subElementRect(SE_TabBarTabText, opt, widget));
    int trans(0);
    bool west(false), east(false);
    switch (opt->shape)
    {
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        west = true;
        trans = -90;
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        east = true;
        trans = 90;
        break;
    default: break;
    }

    if (qAbs(trans))
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

    QFont f(painter->font());
    if (isSelected)
    {
        f.setBold(true);
        painter->setFont(f);
    }
    QPalette::ColorRole fg(Ops::fgRole(widget, QPalette::ButtonText));
    const bool safari(Ops::isSafariTabBar(bar));
    if (safari || styleHint(SH_TabBar_Alignment, opt, widget) == Qt::AlignLeft)
        fg = QPalette::WindowText;
    else if (isSelected)
        fg = Ops::opposingRole(fg);

    const QFontMetrics fm(painter->fontMetrics());
    Qt::TextElideMode elide((Qt::TextElideMode)styleHint(SH_TabBar_ElideMode, opt, widget));
    if (isVertical(opt, bar))
        elide = Qt::ElideRight;
    const QString text(fm.elidedText(opt->text, elide, tr.width()));
    drawItemText(painter, tr, Qt::AlignCenter, option->palette, isEnabled(opt), text, fg);
    if (!opt->icon.isNull())
        drawItemPixmap(painter, ir, Qt::AlignCenter, opt->icon.pixmap(opt->iconSize));
    painter->restore();
    return true;
}

static void drawDocTabBar(QPainter *p, const QTabBar *bar, QRect rect, QTabBar::Shape shape = QTabBar::RoundedNorth)
{
    QRect r(rect.isValid()?rect:bar->rect());
    if (Ops::isSafariTabBar(bar))
    {
        if (XHandler::opacity() < 1.0f)
        {
            p->setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p->fillRect(r, Qt::black);
            p->setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
        const bool hadAA(p->testRenderHint(QPainter::Antialiasing));
        p->setRenderHint(QPainter::Antialiasing, false);
        Handlers::Window::drawUnoPart(p, r, bar, bar->mapTo(bar->window(), bar->rect().topLeft()));
        p->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        p->drawLine(r.bottomLeft(), r.bottomRight());
        GFX::drawShadow(Sunken, r.translated(0, -1), p, bar->isEnabled(), 0, Top);
        p->setRenderHint(QPainter::Antialiasing, hadAA);
    }
    else if (bar->documentMode() && bar->isVisible())
    {
//        if (!s_tabBarShadow.isNull())
//            p->drawTiledPixmap(rect.adjusted(0, rect.height()-s_tabBarShadow.height(), 0, 0), s_tabBarShadow);
        bool aboveStatusBar(false);
        QLine l, s, bl;
        QRect orgRect(r);
        bool isUp(false);
        const bool hasGradient(!dConf.tabs.gradient.isEmpty());
        QLinearGradient lg;
        switch (bar->shape())
        {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth:
        {
            isUp = true;
            if (hasGradient)
                lg = QLinearGradient(r.topLeft(), r.bottomLeft());
            s.setPoints(r.topLeft(), r.topRight());
            r.setTop(r.top()+1);
            l.setPoints(r.topLeft(), r.topRight());
            bl.setPoints(r.bottomLeft(), r.bottomRight());
            break;
        }
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
        {
            if (hasGradient)
                lg = QLinearGradient(r.bottomLeft(), r.topLeft());
            if (dConf.uno.enabled)
            {
                const QPoint below(bar->mapTo(bar->window(), bar->rect().bottomRight()+QPoint(0, 2)));
                if (qobject_cast<QStatusBar *>(bar->window()->childAt(below)))
                    aboveStatusBar = true;
            }
            if (!aboveStatusBar)
            {
                s.setPoints(r.bottomLeft(), r.bottomRight());
                r.setBottom(r.bottom()-1);
            }
            l.setPoints(r.bottomLeft(), r.bottomRight());
            bl.setPoints(r.topLeft(), r.topRight());
            break;
        }
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
        {
            if (hasGradient)
                lg = QLinearGradient(r.topLeft(), r.topRight());
            s.setPoints(r.topLeft(), r.bottomLeft());
            r.setLeft(r.left()+1);
            l.setPoints(r.topLeft(), r.bottomLeft());
            bl.setPoints(r.topRight(), r.bottomRight());
            break;
        }
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
        {
            if (hasGradient)
                lg = QLinearGradient(r.topRight(), r.topLeft());
            s.setPoints(r.topRight(), r.bottomRight());
            r.setRight(r.right()-1);
            l.setPoints(r.topRight(), r.bottomRight());
            bl.setPoints(r.topLeft(), r.bottomLeft());
            break;
        }
        default: break;
        }
        QColor c = Color::mid(bar->palette().color(bar->backgroundRole()), bar->palette().color(bar->foregroundRole()), 5, 1);
        QBrush b(c);
        if (hasGradient)
        {
            lg.setStops(DSP::Settings::gradientStops(dConf.tabs.gradient, c));
            b = QBrush(lg);
        }
        p->fillRect(orgRect, b);
        const QPen pen(p->pen());
        const QBrush brush(p->brush());

        p->setPen(QColor(0, 0, 0, dConf.shadows.opacity));
        p->setBrush(Qt::NoBrush);
//        p->drawRect(orgRect.adjusted(0, 0, -1, -!aboveStatusBar));
        if (!s.isNull())
            p->drawLine(s);
        p->drawLine(bl);
        if (isUp)
        {
            p->setPen(QColor(255, 255, 255, 63));
            p->drawLine(l);
        }


        p->setPen(pen);
        p->setBrush(brush);
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

    if (const QTabBar *tabBar = qobject_cast<const QTabBar *>(widget))
    {
        if (tabBar->documentMode() || styleHint(SH_TabBar_Alignment, opt, widget) == Qt::AlignLeft)
        {
            QRect r(tabBar->rect());
            if (opt->rect.width() > r.width())
                r = opt->rect;

            drawDocTabBar(painter, tabBar, r);
            return true;
        }
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
            rect.setTop(b-h);
            break;
        }
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
        {
            const int t(barRect.top()-1);
            rect.setBottom(t+h);
            break;
        }
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
        {
            const int right(barRect.right()+1);
            rect.setLeft(right-wh);
            break;
        }
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
        {
            const int l(barRect.left()-1);
            rect.setRight(l+wh);
            break;
        }
        default: break;
        }
        if (tabBar->documentMode())
        {
            drawDocTabBar(painter, tabBar, barRect);
            return true;
        }
    }
    if (!tabWidget->documentMode())
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
    QRect geo(opt->rect);
    QWidget *w(0);
    QRect theRect(opt->rect);
    const bool first(opt->position == QStyleOptionToolBoxV2::Beginning);
    const bool last(opt->position == QStyleOptionToolBoxV2::End);
    if (const QToolBox *box = qobject_cast<const QToolBox *>(widget))
    {
        if (box->frameShadow() == QFrame::Sunken)
        {
            painter->fillRect(option->rect, pal.button());
            const QPen saved(painter->pen());
            painter->setPen(QColor(0, 0, 0, 255.0f*dConf.shadows.opacity));
            if (option->SUNKEN || !last)
                painter->drawLine(theRect.bottomLeft(), theRect.bottomRight());
            if (opt->selectedPosition == QStyleOptionToolBoxV2::PreviousIsSelected)
                painter->drawLine(theRect.topLeft(), theRect.topRight());
            painter->setPen(saved);
            return true;
        }
        const QList<QWidget *> tabs(widget->findChildren<QWidget *>("qt_toolbox_toolboxbutton"));
        for (int i = 0; i < tabs.size(); ++i)
        {
            QWidget *tab(tabs.at(i));
            if (tab == painter->device())
            {
                w = tab;
                geo = w->geometry();
            }
            if (tab->geometry().bottom() > theRect.bottom())
                theRect.setBottom(tab->geometry().bottom());
        }
    }

    Sides sides(All);
    if (!first)
        sides &= ~Top;
    if (!last)
        sides &= ~Bottom;

    QLinearGradient lg(theRect.topLeft(), theRect.bottomLeft());
    lg.setStops(DSP::Settings::gradientStops(dConf.tabs.gradient, pal.color(QPalette::Button)));
    GFX::drawClickable(dConf.tabs.shadow, theRect, painter, lg, qMin<int>(opt->rect.height()/2, dConf.tabs.rnd), sides, option, w);
    return true;
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
