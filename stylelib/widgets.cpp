#include "widgets.h"
#include "../config/settings.h"
#include "color.h"
#include "gfx.h"
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QImage>
#include <QSplitterHandle>
#include <QApplication>
#include <QMouseEvent>
#include <QMainWindow>
#include "xhandler.h"
#include "macros.h"
#include "windowdata.h"
#include "fx.h"
#include "masks.h"
#include "shapes.h"

using namespace DSP;

static const float s_pen(2.0f);

ButtonBase::ButtonBase(const Type type, ButtonGroupBase *group)
    : m_type(type)
    , m_hasPress(false)
    , m_hasMouse(false)
    , m_hoverLock(false)
    , m_buttonStyle(dConf.deco.buttons)
    , m_shadowOpacity(dConf.shadows.opacity)
    , m_shadowIllumination(dConf.shadows.illumination)
    , m_shadowStyle(dConf.toolbtn.shadow)
    , m_gradient(dConf.toolbtn.gradient)
    , m_group(group)
{
    if (group)
        group->addButton(this);
    for (int i = 0; i < Custom; ++i)
        m_paintMethod[i] = 0;

    m_paintMethod[Close] = &ButtonBase::paintCloseButton;
    m_paintMethod[Minimize] = &ButtonBase::paintMinButton;
    m_paintMethod[Maximize] = &ButtonBase::paintMaxButton;
    m_paintMethod[OnAllDesktops] = &ButtonBase::paintOnAllDesktopsButton;
    m_paintMethod[Menu] = &ButtonBase::paintWindowMenuButton;
    m_paintMethod[KeepAbove] = &ButtonBase::paintKeepAboveButton;
    m_paintMethod[KeepBelow] = &ButtonBase::paintKeepBelowButton;
    m_paintMethod[Shade] = &ButtonBase::paintShadeButton;
    m_paintMethod[ContextHelp] = &ButtonBase::paintQuickHelpButton;
    m_paintMethod[ApplicationMenu] = &ButtonBase::paintApplicationMenuButton;


    //0 0 0 min max close
    //static uint fcolors[6] = { 0x0, 0x0, 0x0, 0xFFF8C96C, 0xFF8AC96B, 0xFFFE8D88 };
    //static uint fcolors[6] = { 0x0, 0x0, 0x0, 0xFFFFBE46, 0xFF05C850, 0xFFFB615F };
    //static uint fcolors[6] = { 0x0, 0x0, 0x0, 0xFFFFD580, 0xFF74B435, 0xFFFF8B80 };
//    static uint fcolors[6] = { 0x0, 0x0, 0x0, 0xFFFFC05E, 0xFF88EB51, 0xFFF98862 };

    switch (m_type)
    {
    case Close: m_color = 0xFFF98862; break;
    case Minimize: m_color = 0xFFFFC05E; break;
    case Maximize: m_color = 0xFF88EB51; break;
    default: break;
    }
}

ButtonBase::~ButtonBase()
{
    if (m_group)
        m_group->removeButton(this);
}


const ButtonBase::ButtonStyle
ButtonBase::buttonStyle() const
{
    return m_buttonStyle;
}


const QColor
ButtonBase::color(const ColorRole c) const
{
    if (c == Fg)
        return m_fg;
    if (c == Bg)
        return m_bg;
    if (c == Mid)
        return Color::mid(m_bg, m_fg);
    return qApp->palette().color(QPalette::Highlight);
}

void
ButtonBase::paint(QPainter &p)
{
    if (m_type < Custom && m_paintMethod[m_type])
        (this->*m_paintMethod[m_type])(p);
}

void
ButtonBase::hover()
{
    m_hoverLock = true;
    bool hadMouse(m_hasMouse);
    m_hasMouse = true;
    if (hadMouse != m_hasMouse)
        hoverChanged();
}

bool
ButtonBase::processMouseEvent(QMouseEvent *e)
{
    switch (e->type())
    {
    case QEvent::MouseButtonDblClick:
    {
        e->accept();
        return true;
    }
    case QEvent::MouseButtonPress:
    {
        e->accept();
        m_hasPress = true;
        return true;
    }
    case QEvent::MouseButtonRelease:
    {
        if (m_hasPress)
        {
            e->accept();
            m_hasPress = false;
            if (buttonRect().contains(e->pos()))
                onClick(e->button());
        }
        return true;
    }
    case QEvent::MouseMove:
    {
        if (m_hoverLock)
            return true;
        bool hadMouse(m_hasMouse);
        m_hasMouse = buttonRect().contains(e->pos());
        if (hadMouse != m_hasMouse)
            hoverChanged();
        return true;
    }
    default: return false;
    }
}

/**
 * @brief WidgetButton::drawBase
 * @param c
 * @param p
 * @param r
 * draw a maclike decobuttonbase using color c
 */

void
ButtonBase::drawBase(QColor c, QPainter &p, QRect &r) const
{
    const int /*fgLum(Color::luminosity(color(Fg))),*/ bgLum(Color::lum(color(Bg)));
    const float rat(isActive()?1.5f:0.5f);
    if (m_buttonStyle && m_buttonStyle != FollowToolBtn)
        c.setHsv(c.hue(), qBound<int>(0, (float)c.saturation()*rat, 255), qMax(isActive()?127:0, color(Bg).value()), c.alpha());
    switch (m_buttonStyle)
    {
    case Yosemite:
    {
        p.save();
        p.setPen(Qt::NoPen);
        r.adjust(2, 2, -2, -2);
        QColor dark(Color::mid(c, Qt::black, 5, 2+isDark()*10));
//        dark.setAlpha(63);

        p.setBrush(dark);
        p.drawEllipse(r);
        p.setBrush(c);
        r.adjust(1, 1, -1, -1);
        p.drawEllipse(r);
        p.restore();
        break;
    }
    case Lion:
    {
        QColor low(Color::mid(c, Qt::black, 5, 5+isDark()*10));
        low.setHsv(low.hue(), qMin(127, low.saturation()), low.value());
        const QColor high(QColor(255, 255, 255, m_shadowIllumination/*qMin(255.0f, bgLum*1.1f)*/));
        r.adjust(2, 2, -2, -2);
        p.setBrush(high);
        p.drawEllipse(r.translated(0, 1));
        p.setBrush(low);
        p.drawEllipse(r);
        r.adjust(1, 1, -1, -1);

        QRadialGradient rg(r.center()+QPoint(1, r.height()/2), r.height());
        rg.setColorAt(0.0f, Color::mid(c, Qt::white, 1, 4));
        rg.setColorAt(0.5f, c);
        rg.setColorAt(1.0f, Color::mid(c, Qt::black));
        p.setBrush(rg);
        p.drawEllipse(QRectF(r).adjusted(0.25f, 0.25f, -0.25f, -0.25f).translated(0.0f, 0.5f));

        QRect rr(r);
        rr.setWidth(6);
        rr.setHeight(3);
        rr.moveCenter(r.center());
        rr.moveTop(r.top()+1);
        QLinearGradient lg(rr.topLeft(), rr.bottomLeft());
        lg.setColorAt(0.0f, QColor(255, 255, 255, 222));
        lg.setColorAt(1.0f, QColor(255, 255, 255, 64));
        p.setBrush(lg);
        p.drawEllipse(rr);
        break;
    }
    case Sunken:
    {
        const QColor low(Color::mid(c, Qt::black, 5, 3+isDark()*10));
        const QColor high(QColor(255, 255, 255, m_shadowIllumination/*qMin(255.0f, bgLum*1.1f)*/));
        r.adjust(2, 2, -2, -2);
        p.setBrush(high);
        p.drawEllipse(r.translated(0, 1));
        p.setBrush(low);
        p.drawEllipse(r);
        r.adjust(1, 1, -1, -1);

        QRadialGradient rg(r.center()+QPoint(1, r.height()*0.2f), r.height()*0.66f);
        rg.setColorAt(0.0f, c);
        rg.setColorAt(0.33f, c);
        rg.setColorAt(1.0f, Qt::transparent);
        p.setBrush(rg);
        p.drawEllipse(r);
        break;
    }
    case Carved:
    {
        p.save();
        QLinearGradient lg(r.topLeft(), r.bottomLeft());
        int high(0), low(0);
        if (isDark())
        {
            high=32;
            low=192;
        }
        else
        {
            high=170;
            low=85;
        }

        lg.setColorAt(0.0f, QColor(0, 0, 0, low));
        lg.setColorAt(1.0f, QColor(255, 255, 255, high));
        p.setBrush(lg);
        p.setPen(Qt::NoPen);
        r.adjust(0, 1, 0, -1);
        p.drawEllipse(r);
        r.adjust(0, -1, 0, 1);
        r.shrink(3);
        p.setBrush(c.darker(200));
        p.drawEllipse(r);
        r.shrink(1);
        QLinearGradient mask(r.topLeft(), r.bottomLeft());
        mask.setColorAt(0.0f, c.lighter(120));
        mask.setColorAt(1.0f, c.darker(120));
        p.setBrush(mask);
        p.drawEllipse(r);
        p.restore();
        break;
    }
    case FollowToolBtn:
    {
        r.shrink((4-GFX::shadowMargin(m_shadowStyle)));
        QLinearGradient lg(r.topLeft(), r.bottomLeft());
        lg.setStops(DSP::Settings::gradientStops(m_gradient, c));
        const bool hasDark(dConf.shadows.darkRaisedEdges);
        dConf.shadows.darkRaisedEdges = false;
        GFX::drawClickable(m_shadowStyle, r, &p, lg, MaxRnd);
        dConf.shadows.darkRaisedEdges = hasDark;
        r.shrink(4);
        break;
    }
    case MagPie:
    {
//        r.adjust(2, 2, -2, -2);
        r.shrink(2);
        const int bgLum = Color::lum(color(Bg));
        const int fgLum = Color::lum(color(Fg));
        p.setBrush(QColor(255, 255, 255, m_shadowIllumination));
        p.drawEllipse(r.translated(0, 1));
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.drawEllipse(r);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setBrush(QColor(0, 0, 0, fgLum));
        p.drawEllipse(r);
        p.setBrush(QColor(0, 0, 0, qAbs(fgLum-bgLum)));
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        for (int i = 1; i < 3; ++i)
            p.drawEllipse(r.adjusted(i, i, -i, -i));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
#if 0
        r.adjust(2, 2, -2, -2);

        //frame
        p.setBrush(QColor(255, 255, 255, m_shadowIllumination));
        p.drawEllipse(r.translated(0, 1));
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setBrush(Qt::black);
        p.drawEllipse(r);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setBrush(QColor(0, 0, 0, m_shadowOpacity));
        p.drawEllipse(r);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setBrush(Qt::black);
        p.drawEllipse(r.adjusted(1,1,-1,-1));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);

        //button color
        const int bgLum = Color::lum(color(Bg));
        const int fgLum = Color::lum(color(Fg));
        const bool dark = fgLum > bgLum;
        const quint8 rgb = dark ? 0 : 255;
        const quint8 a = dark ? fgLum - bgLum : bgLum - fgLum;
        p.setBrush(QColor(rgb, rgb, rgb, a));
//        p.setBrush(QColor(0, 0, 0, fgLum));
        p.drawEllipse(r.adjusted(1,1,-1,-1));
//        p.setBrush(QColor(0, 0, 0, a));
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        for (int i = 1; i < 3; ++i)
            p.drawEllipse(r.adjusted(i+1, i+1, -(i+1), -(i+1)));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
        break;
    }
    case NeoDesk:
    {
        r.shrink(3);
        r.translate(1, 0);
        Mask::render(r.translated(0, 1), QColor(255, 255, 255, m_shadowIllumination), &p, MaxRnd);
        QColor color = c.darker(160);
        color.setHsv(color.hue(), 255, color.value());
        Mask::render(r, color, &p, MaxRnd);
        r.shrink(1);
        Mask::render(r, c, &p, MaxRnd);

        QImage img(r.size(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter pt(&img);
        Mask::render(img.rect().shrinked(1), Qt::white, &pt, MaxRnd);
        pt.end();
        FX::expblur(img, 1);
        p.setCompositionMode(QPainter::CompositionMode_SoftLight);
        p.drawImage(r.topLeft(), img);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        r.shrink(2);
        break;
    }
    case Anton:
    {
        QLinearGradient lg(r.topLeft(), r.bottomLeft());
        const int start(m_hasPress?140:223), end(/*m_hasPress?140:*/175);
        lg.setColorAt(0, QColor(start,start,start));
        lg.setColorAt(1, QColor(end,end,end));
        GFX::drawClickable(m_hasPress?DSP::Sunken:DSP::Raised, r/*.adjusted(-1, 0, 1, 0)*/, &p, lg, 2);
        r.shrink(2);
        break;
    }
    default: break;
    }
}

void
ButtonBase::drawX(QPainter &p, const QRect &r, const QBrush &brush)
{
    const bool anton = m_buttonStyle == Anton;
    const int size = anton ? 5 : 7;
    QRectF rt(QPoint(), QSize(size, size));
    rt.moveCenter(QRectF(r).center()/*+QPoint(!(r.width() & 1), !(r.height() & 1))*/);
    p.fillPath(Shapes::xMark(rt), brush);
}

void
ButtonBase::drawArrow(QPainter &p, const QRect &r, const QBrush &brush, const bool up)
{
    const bool anton = m_buttonStyle == Anton;
    const int size = anton ? 5 : 7;
    QRectF rt(QPoint(), QSize(size, size));
    rt.moveCenter(QRectF(r).center()/*+QPoint(!(r.width() & 1), !(r.height() & 1))*/);
    if (up)
        p.fillPath(Shapes::upArrow(rt), brush);
    else
        p.fillPath(Shapes::downArrow(rt), brush);
}

quint64
ButtonBase::state() const
{
    QRect r(buttonRect());
    const QColor c(color(Mid));
    const quint64 check((quint64)(r.width()*r.height())
                        | ((quint64)(m_buttonStyle+1)   << 16)
                        | ((quint64)onAllDesktops()     << 20)
                        | ((quint64)isMaximized()       << 21)
                        | ((quint64)isActive()          << 22)
                        | ((quint64)isHovered()         << 23)
                        | ((quint64)keepBelow()         << 24)
                        | ((quint64)keepAbove()         << 25)
                        | ((quint64)m_hasPress          << 26)
                        | ((quint64)c.rgba()            << 32));
    return check;
}

void
ButtonBase::paintCloseButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(buttonRect());
    const quint64 check(state());
    if (!m_bgPix.contains(check))
    {
        if (m_buttonStyle == NoStyle)
        {
            QPixmap pix(buttonRect().size());
            const int s(pix.width()/8);
            const QPen pen(color(Mid), s_pen, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            QRect rect = pix.rect().adjusted(s, s, -s, -s);
            rect.adjust(s, s, -s, -s);
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setRenderHint(QPainter::Antialiasing);
            pt.setPen(pen);
            pt.drawLine(rect.topLeft(), rect.bottomRight());
            pt.drawLine(rect.topRight(), rect.bottomLeft());
            pt.end();
            p.drawTiledPixmap(buttonRect(), FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
        }
        else
        {
            QPixmap pix(r.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setPen(Qt::NoPen);
            pt.setRenderHint(QPainter::Antialiasing);
            QRect rect(pix.rect());
            const bool anton = m_buttonStyle == Anton;
//            if (anton)
//                rect.translate(0,1);
            drawBase(isActive()?m_color:color(Bg), pt, rect);
            if (m_buttonStyle == MagPie || anton)
            {
                QColor c = color(isActive()?Fg:Mid);
                if (anton)
                {
                    drawX(pt, rect.translated(0, 1), QColor(255,255,255));
                    c = QColor(127,127,127);
                }
                drawX(pt, rect, c);
            }
            else if (isHovered())
            {
                pt.setBrush(color(Fg));
                QRect r(QPoint(), QSize(4, 4));
                r.moveCenter(rect.center());
                pt.drawEllipse(r);
            }
            pt.end();
            m_bgPix.insert(check, pix);
        }
    }
    p.drawTiledPixmap(r, m_bgPix.value(check));
}

void
ButtonBase::paintMaxButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(buttonRect());
    const quint64 check(state());
    if (!m_bgPix.contains(check))
    {
        if (m_buttonStyle == NoStyle)
        {
            QPixmap pix(buttonRect().size());
            const int s(pix.width()/8);
            const QPen pen(color(isMaximized()?Highlight:Mid), s_pen, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            QRect rect = pix.rect().adjusted(s, s, -s, -s);
            rect.adjust(s, s, -s, -s);
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setRenderHint(QPainter::Antialiasing);
            pt.setPen(pen);
            int x, y, w, h;
            rect.getRect(&x, &y, &w, &h);
            pt.drawLine(x+w/2, y, x+w/2, y+h);
            pt.drawLine(x, y+h/2, x+w, y+h/2);
            pt.end();
            p.drawTiledPixmap(buttonRect(), FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
        }
        else
        {
            QPixmap pix(r.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setPen(Qt::NoPen);
            pt.setRenderHint(QPainter::Antialiasing);
            QRect rect(pix.rect());
            const bool anton = m_buttonStyle == Anton;
//            if (anton)
//                rect.translate(0,1);
            drawBase(isActive()?m_color:color(Bg), pt, rect);
            if (m_buttonStyle == MagPie || anton)
            {
                QColor c = color(isActive()?Fg:Mid);
                if (anton)
                {
                    drawArrow(pt, rect.translated(0, 1), QColor(255,255,255), true);
                    c = QColor(127,127,127);
                }
                drawArrow(pt, rect, c, true);
            }
            else if (isHovered())
            {
                pt.setBrush(color(Fg));
                QRect r(QPoint(), QSize(4, 4));
                r.moveCenter(rect.center());
                pt.setPen(QPen(color(Fg), 2.0f));
                if (isMaximized())
                {
                    pt.translate(0.5f, 0.5f);
                    pt.drawLine(r.topRight(), r.bottomLeft());
                }
                else
                {
                    int x1, y1, x2, y2;
                    r.getRect(&x1, &y1, &x2, &y2);
                    int halfX(x1+(x2/2));
                    int halfY(y1+(y2/2));
                    pt.drawLine(halfX, y1, halfX, y1+y2);
                    pt.drawLine(x1, halfY, x1+x2, halfY);
                }
            }
            pt.end();
            m_bgPix.insert(check, pix);
        }
    }
    p.drawTiledPixmap(r, m_bgPix.value(check));
}

void
ButtonBase::paintMinButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(buttonRect());
    const quint64 check(state());
    if (!m_bgPix.contains(check))
    {
        if (m_buttonStyle == NoStyle)
        {
            QPixmap pix(buttonRect().size());
            const int s(pix.width()/8);
            const QPen pen(color(Mid), s_pen, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            QRect rect = pix.rect().adjusted(s, s, -s, -s);
            rect.adjust(s, s, -s, -s);
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setRenderHint(QPainter::Antialiasing);
            pt.setPen(pen);
            int x, y, w, h;
            rect.getRect(&x, &y, &w, &h);
            pt.drawLine(x, y+h/2, x+w, y+h/2);
            pt.end();
            m_bgPix.insert(check, FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
        }
        else
        {
            QPixmap pix(r.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setPen(Qt::NoPen);
            pt.setRenderHint(QPainter::Antialiasing);
            QRect rect(pix.rect());
            const bool anton = m_buttonStyle == Anton;
//            if (anton)
//                rect.translate(0,1);
            drawBase(isActive()?m_color:color(Bg), pt, rect);
            if (m_buttonStyle == MagPie || anton)
            {
                QColor c = color(isActive()?Fg:Mid);
                if (anton)
                {
                    drawArrow(pt, rect.translated(0, 1), QColor(255,255,255), false);
                    c = QColor(127,127,127);
                }
                drawArrow(pt, rect, c, false);
            }
            else if (isHovered())
            {
                QRect r(QPoint(), QSize(4, 4));
                r.moveCenter(rect.center());
                pt.setBrush(color(Fg));
                pt.setPen(QPen(color(Fg), 2.0f));
                int x1, y1, x2, y2;
                r.getRect(&x1, &y1, &x2, &y2);
                int halfY(y1+(y2/2));
                pt.drawLine(x1, halfY, x1+x2, halfY);
            }
            pt.end();
            m_bgPix.insert(check, pix);
        }
    }
    p.drawTiledPixmap(r, m_bgPix.value(check));
}

void
ButtonBase::paintOnAllDesktopsButton(QPainter &p)
{
    const bool all(onAllDesktops());
    const QColor c(color(Mid));
    const quint64 check(state());
    if (!m_bgPix.contains(check))
    {
        QPixmap pix(buttonRect().size());
        const int s(pix.width()/8);
        const QPen pen(c, s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QRect rect = pix.rect().adjusted(s, s, -s, -s);
        rect.adjust(s, s, -s, -s);
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        int x, y, w, h;
        rect.getRect(&x, &y, &w, &h);
        pt.drawLine(x, y+h/2, x+w, y+h/2);
        if (!all)
            pt.drawLine(x+w/2, y, x+w/2, y+h);
        pt.end();
        p.drawTiledPixmap(buttonRect(), FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
        m_bgPix.insert(check, FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

void
ButtonBase::paintWindowMenuButton(QPainter &p)
{
    const QColor c(color(Mid));
    const quint64 check(state());
    if (!m_bgPix.contains(check))
    {
        QPixmap pix(buttonRect().size());
        pix.fill(Qt::transparent);
        const int s(pix.width()/8);
        QRect r = pix.rect().adjusted(s, s, -s, -s);
        const QPen pen(c, s, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        r.adjust(s, s, -s, -s);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        int x, y, w, h;
        r.getRect(&x, &y, &w, &h);
        for (int i = 0; i < 3; ++i)
            pt.drawLine(x, y+i*4, x+w, y+i*4);
        pt.end();
        m_bgPix.insert(check, FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

void
ButtonBase::paintApplicationMenuButton(QPainter &p)
{
    paintWindowMenuButton(p);
}

void
ButtonBase::paintKeepAboveButton(QPainter &p)
{
    const QColor c(color(keepAbove()?Highlight:Mid));
    const quint64 check(state());
    if (!m_bgPix.contains(check))
    {
        QPixmap pix(buttonRect().size());
        pix.fill(Qt::transparent);
        const int s(pix.width()/8);
        QRect r = pix.rect().adjusted(s, s, -s, -s);
        const QPen pen(c, s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        r.adjust(s, s, -s, -s);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        pt.setBrush(pt.pen().color());
        int x, y, w, h;
        r.getRect(&x, &y, &w, &h);
        static const int points[] = { x,y+h/2, x+w/2,y, x+w,y+h/2 };
        QPolygon polygon;
        polygon.setPoints(3, points);
        pt.drawPolygon(polygon);
        pt.end();
        m_bgPix.insert(check, FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

void
ButtonBase::paintKeepBelowButton(QPainter &p)
{
    const QColor c(color(keepBelow()?Highlight:Mid));
    const quint64 check(state());
    if (!m_bgPix.contains(check))
    {
        QPixmap pix(buttonRect().size());
        pix.fill(Qt::transparent);
        const int s(pix.width()/8);
        QRect r = pix.rect().adjusted(s, s, -s, -s);
        const QPen pen(c, s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        r.adjust(s, s, -s, -s);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        pt.setBrush(pt.pen().color());
        int x, y, w, h;
        r.getRect(&x, &y, &w, &h);
        static const int points[] = { x,y+h/2, x+w/2,y+h, x+w,y+h/2 };
        QPolygon polygon;
        polygon.setPoints(3, points);
        pt.drawPolygon(polygon);
        pt.end();
        m_bgPix.insert(check, FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

void
ButtonBase::paintShadeButton(QPainter &p)
{
    const QColor c(color(shade()?Highlight:Mid));
    const quint64 check(state());
    if (!m_bgPix.contains(check))
    {
        QPixmap pix(buttonRect().size());
        pix.fill(Qt::transparent);
        const int s(pix.width()/8);
        QRect r = pix.rect().adjusted(s, s, -s, -s);
        const QPen pen(c, s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        r.adjust(s, s, -s, -s);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        int x, y, w, h;
        r.getRect(&x, &y, &w, &h);
        pt.drawLine(x, y+h/3, x+w, y+h/3);
        pt.end();
        m_bgPix.insert(check, FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

void
ButtonBase::paintQuickHelpButton(QPainter &p)
{
    const QColor c(color(Mid));
    const quint64 check(state());
    if (!m_bgPix.contains(check))
    {
        QPixmap pix(buttonRect().size());
        pix.fill(Qt::transparent);
        const int s(pix.height()/8);
        QRectF r(pix.rect().adjusted(s, s, -s, -s));
        QFont f(p.font());
        f.setWeight(QFont::Black);
        f.setPixelSize(r.height()*1.5f);
        const float rnd(r.height()/4.0f);
        const float x(r.x());
        const float y(r.y());
        QPainterPath path;
        path.moveTo(x, y+rnd);
        path.quadTo(r.topLeft(), QPointF(x+rnd, y));
        path.quadTo(QPointF(x+rnd*2, y), QPointF(x+rnd*2, y+rnd));
        path.quadTo(QPointF(x+rnd*2, y+rnd*2), QPointF(x+rnd, y+rnd*2));
        path.lineTo(QPointF(x+rnd, y+rnd*2+rnd/2));

        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(QPen(c, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        pt.setBrush(Qt::NoBrush);
        pt.drawPath(path.translated(rnd, 0));
        pt.setPen(Qt::NoPen);
        pt.setBrush(c);
        pt.drawEllipse(QPoint(x+rnd+rnd, y+rnd*4), 2, 2);
        pt.end();
        m_bgPix.insert(check, FX::sunkenized(pix.rect(), pix, isDark(), isDark()?m_shadowOpacity:m_shadowIllumination));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

////////////////////////////////////////////////////////////////////////////////////////

static QMap<quint64, ButtonGroupBase *> s_buttonBases;

ButtonGroupBase
*ButtonGroupBase::buttonGroup(const quint64 window)
{
    if (!s_buttonBases.contains(window))
        s_buttonBases.insert(window, new ButtonGroupBase(window));
    return s_buttonBases.value(window);
}

void
ButtonGroupBase::cleanUp(const quint64 window)
{
    if (s_buttonBases.contains(window))
        delete s_buttonBases.take(window);
}

ButtonGroupBase::ButtonGroupBase(const quint64 window) : m_window(window)
{

}

void
ButtonGroupBase::setColors(const QColor &bg, const QColor &fg, const QColor &min, const QColor &max, const QColor &close)
{
    for (int i = 0; i < buttons().size(); ++i)
    {
        ButtonBase *button = buttons().at(i);
        button->m_bg = bg;
        button->m_fg = fg;
        if (min.isValid() && max.isValid() && close.isValid())
            switch (button->buttonType())
            {
            case ButtonBase::Close: button->m_color = close; break;
            case ButtonBase::Minimize: button->m_color = min; break;
            case ButtonBase::Maximize: button->m_color = max; break;
            default: break;
            }
    }
}

void
ButtonGroupBase::configure(const int shadowOpacity, const int shadowIllumination, const ButtonBase::ButtonStyle buttonStyle, const ShadowStyle shadowStyle, const Gradient &grad)
{
    for (int i = 0; i < buttons().size(); ++i)
    {
        ButtonBase *button = buttons().at(i);
        button->m_shadowOpacity = shadowOpacity;
        button->m_shadowIllumination = shadowIllumination;
        button->m_buttonStyle = buttonStyle;
        button->m_shadowStyle = shadowStyle;
        button->m_gradient = grad;
    }
}

void
ButtonGroupBase::clearCache()
{
    for (int i = 0; i < buttons().size(); ++i)
    {
        ButtonBase *button = buttons().at(i);
        button->clearBgPix();
    }
}

////////////////////////////////////////////////////////////////////////////////////////

WidgetButton::WidgetButton(Type type, QWidget *parent)
    : ButtonBase(type),
      QWidget(parent)
{
    setAttribute(Qt::WA_Hover);
    setFixedSize(16, 16);
    setForegroundRole(QPalette::ButtonText);
    setBackgroundRole(QPalette::Button);
}

void
WidgetButton::onClick(const Qt::MouseButton &btn)
{
    switch (buttonType())
    {
    case Close: window()->close(); break;
    case Minimize: window()->showMinimized(); break;
    case Maximize: window()->isMaximized()?window()->showNormal():window()->showMaximized(); break;
    default: break;
    }
}

const QColor
WidgetButton::color(const ColorRole &c) const
{
    if (c==Highlight)
        return palette().color(QPalette::Highlight);
    if (c==Mid)
        return Color::mid(palette().color(foregroundRole()), palette().color(backgroundRole()));
    return palette().color(c==Fg?foregroundRole():backgroundRole());
}

const bool
WidgetButton::isDark() const
{
    const QWidget *w(parentWidget()?parentWidget():this);
    return Color::lum(w->palette().color(w->foregroundRole())) > Color::lum(w->palette().color(w->backgroundRole()));
}

void
WidgetButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    ButtonBase::paint(p);
}


////////////////////////////////////////////////////////////////////////////////////////

#define SSIZE 32

static SplitterExt *s_se(0);

void
SplitterExt::manage(QWidget *sh)
{
    if (!sh)
        return;
    if (!s_se)
        s_se = new SplitterExt();
    sh->removeEventFilter(s_se);
    sh->installEventFilter(s_se);
}

SplitterExt::SplitterExt()
    : QWidget(0)
    , m_hasPress(false)
{
    hide();
    setFixedSize(SSIZE, SSIZE);
    setAttribute(Qt::WA_Hover);
    setAttribute(Qt::WA_NoChildEventsForParent);
    setMouseTracking(true);
}

bool
SplitterExt::isActive()
{
    return s_se && s_se->isVisible();
}

bool
SplitterExt::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return false;
    QWidget *w(static_cast<QWidget *>(o));
    switch (e->type())
    {
    case QEvent::Enter:
    {
        m_splitter = w;
        QWidget *win(w->window());
        m_enterPoint = m_splitter->mapFromGlobal(QCursor::pos());
        Qt::CursorShape shape(m_splitter->cursor().shape());
        setCursor(shape);
        setParent(win);
        const QPoint topLeft(win->mapFromGlobal(QCursor::pos()));
        move(topLeft-QPoint(SSIZE>>1, SSIZE>>1));
        raise();
        show();
        return false;
    }
    default: return false;
    }
}

bool
SplitterExt::event(QEvent *e)
{
    if (!m_splitter)
        return false;
    switch (e->type())
    {
    case QEvent::MouseButtonPress:
    {
        e->accept();
        QMouseEvent *me(static_cast<QMouseEvent *>(e));
        QMouseEvent m(me->type(), m_enterPoint, m_splitter->mapToGlobal(m_enterPoint), me->button(), me->buttons(), me->modifiers());
        QCoreApplication::sendEvent(m_splitter, &m);
        m_hasPress = true;
        return true;
    }
    case QEvent::MouseButtonRelease:
    case QEvent::Leave:
    {
        e->accept();
        QMouseEvent *me(static_cast<QMouseEvent *>(e));
        QMouseEvent m(me->type(), m_splitter->mapFromGlobal(QCursor::pos()), QCursor::pos(), me->button(), me->buttons(), me->modifiers());
        QCoreApplication::sendEvent(m_splitter, &m);
        if (e->type() == QEvent::MouseButtonRelease || !m_hasPress)
        {
            m_hasPress = false;
            hide();
            setParent(0);
            m_splitter = 0;
        }
        return true;
    }
    case QEvent::MouseMove:
    {
        e->accept();
        if (!m_hasPress)
            return false;
        move(parentWidget()->mapFromGlobal(QCursor::pos())-QPoint(SSIZE>>1, SSIZE>>1));
        QMouseEvent *me(static_cast<QMouseEvent *>(e));
        QMouseEvent m(me->type(), m_splitter->mapFromGlobal(QCursor::pos()), QCursor::pos(), me->button(), me->buttons(), me->modifiers());
        QCoreApplication::sendEvent(m_splitter, &m);
        return true;
    }
    default: return false;
    }
}
