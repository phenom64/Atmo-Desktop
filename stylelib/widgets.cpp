#include "widgets.h"
#include "../config/settings.h"
#include "color.h"
#include "render.h"
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

#define ISIZE 4

ButtonBase::ButtonBase(Type type)
    : m_type(type)
    , m_hasPress(false)
    , m_hasMouse(false)
    , m_hoverLock(false)
{
    for (int i = 0; i < TypeCount; ++i)
        m_paintEvent[i] = 0;

    m_paintEvent[Close] = &ButtonBase::paintCloseButton;
    m_paintEvent[Min] = &ButtonBase::paintMinButton;
    m_paintEvent[Max] = &ButtonBase::paintMaxButton;
    m_paintEvent[OnAllDesktops] = &ButtonBase::paintOnAllDesktopsButton;
    m_paintEvent[WindowMenu] = &ButtonBase::paintWindowMenuButton;
    m_paintEvent[KeepAbove] = &ButtonBase::paintKeepAboveButton;
    m_paintEvent[KeepBelow] = &ButtonBase::paintKeepBelowButton;
    m_paintEvent[Shade] = &ButtonBase::paintShadeButton;
    m_paintEvent[Resize] = &ButtonBase::paintResizeButton;
    m_paintEvent[QuickHelp] = &ButtonBase::paintQuickHelpButton;
    m_paintEvent[AppMenu] = &ButtonBase::paintApplicationMenuButton;
}

ButtonBase::~ButtonBase()
{

}

void
ButtonBase::paint(QPainter &p)
{
    if (m_type < TypeCount && m_paintEvent[m_type])
        (this->*m_paintEvent[m_type])(p);
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

const Button::ButtonStyle
ButtonBase::buttonStyle() const
{
    return dConf.deco.buttons;
}

/**
 * @brief Button::drawBase
 * @param c
 * @param p
 * @param r
 * draw a maclike decobuttonbase using color c
 */

void
ButtonBase::drawBase(QColor c, QPainter &p, QRect &r) const
{
    const int /*fgLum(Color::luminosity(color(Fg))),*/ bgLum(Color::luminosity(color(Bg)));
    const float rat(isActive()?1.5f:0.5f);
    if (buttonStyle())
        c.setHsv(c.hue(), qBound<int>(0, (float)c.saturation()*rat, 255), qMax(isActive()?127:0, color(Bg).value()), c.alpha());
    switch (buttonStyle())
    {
    case Yosemite:
    {
        p.save();
        p.setPen(Qt::NoPen);
        r.adjust(2, 2, -2, -2);
        QColor dark(Color::mid(c, Qt::black, 5, 3+isDark()*10));
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
        const QColor high(QColor(255, 255, 255, qMin(255.0f, bgLum*1.1f)));
        r.adjust(2, 2, -2, -2);
        p.setBrush(high);
        p.drawEllipse(r.translated(0, 1));
        p.setBrush(low);
        p.drawEllipse(r);
        r.adjust(1, 1, -1, -1);

        QRadialGradient rg(r.center()+QPoint(1, r.height()/2), r.height());
        rg.setColorAt(0.0f, Color::mid(c, Qt::white, 1, 3));
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
        const QColor high(QColor(255, 255, 255, qMin(255.0f, bgLum*1.1f)));
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
        r.adjust(2, 2, -2, -2);
        QLinearGradient lg(0, 0, 0, r.height()-Render::shadowMargin(dConf.toolbtn.shadow));
        lg.setStops(Settings::gradientStops(dConf.toolbtn.gradient, c));
        QBrush b(lg);
        const bool hasDark(dConf.shadows.darkRaisedEdges);
        dConf.shadows.darkRaisedEdges = false;
        Render::drawClickable(dConf.toolbtn.shadow, r, &p, 32, dConf.shadows.opacity, 0, 0, &b);
        dConf.shadows.darkRaisedEdges = hasDark;
        r.adjust(4, 4, -4, -4);
        break;
    }
    default: break;
    }
}

/** Stolen colors from bespin....
 *  none of them are perfect, but
 *  the uncommented ones are 'good enough'
 *  for me, search the web/pick better
 *  ones yourself if not happy
 */

// static uint fcolors[3] = {0x9C3A3A/*0xFFBF0303*/, 0xFFEB55/*0xFFF3C300*/, 0x77B753/*0xFF00892B*/};
// Font
//static uint fcolors[3] = {0xFFBF0303, 0xFFF3C300, 0xFF00892B};
// Aqua
// static uint fcolors[3] = { 0xFFD86F6B, 0xFFD8CA6B, 0xFF76D86B };

// static uint fcolors[3] = { 0xFFFF7E71, 0xFFFBD185, 0xFFB1DE96 }; <<<-best from bespin
// Aqua2
// static uint fcolors[3] = { 0xFFBF2929, 0xFF29BF29, 0xFFBFBF29 };

//static uint fcolors[3] = { 0xFFFF7E71, 0xFFFBD185, 0xFF37CC40 };

static uint fcolors[3] = { 0xFFFE8D88, 0xFFF8C96C, 0xFF8AC96B };

void
ButtonBase::paintCloseButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(buttonRect());
    const QColor c(color(Mid));
    const quint64 check((r.width()*r.height())
                        | ((quint64)(dConf.deco.buttons+1)<<24)
                        | ((quint64)isActive()<<28)
                        | ((quint64)isHovered()<<30)
                        | ((quint64)c.rgba()<<32));
    if (!m_bgPix.contains(check))
    {
        if (dConf.deco.buttons == -1)
        {
            QPixmap pix(buttonRect().size());
            const int s(pix.width()/8);
            const QPen pen(color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            QRect rect = pix.rect().adjusted(s, s, -s, -s);
            rect.adjust(s, s, -s, -s);
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setRenderHint(QPainter::Antialiasing);
            pt.setPen(pen);
            pt.drawLine(rect.topLeft(), rect.bottomRight());
            pt.drawLine(rect.topRight(), rect.bottomLeft());
            pt.end();
            p.drawTiledPixmap(buttonRect(), Render::sunkenized(pix.rect(), pix, isDark(), color(Mid)));
        }
        else
        {
            QPixmap pix(r.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setPen(Qt::NoPen);
            pt.setRenderHint(QPainter::Antialiasing);
            QRect rect(pix.rect());
            drawBase(isActive()?fcolors[m_type]:color(Bg), pt, rect);
            if (isHovered())
            {
                pt.setBrush(color(Fg));
                QRect r(QPoint(), QSize(ISIZE, ISIZE));
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
    const QColor c(color(Mid));
    const quint64 check((r.width()*r.height())
                        | ((quint64)(dConf.deco.buttons+1)<<24)
                        | ((quint64)isMaximized()<<26)
                        | ((quint64)isActive()<<28)
                        | ((quint64)isHovered()<<30)
                        | ((quint64)c.rgba()<<32));
    if (!m_bgPix.contains(check))
    {
        if (dConf.deco.buttons == NoStyle)
        {
            QPixmap pix(buttonRect().size());
            const int s(pix.width()/8);
            const QPen pen(color(isMaximized()?Highlight:Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
            p.drawTiledPixmap(buttonRect(), Render::sunkenized(pix.rect(), pix, isDark(), c));
        }
        else
        {
            QPixmap pix(r.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setPen(Qt::NoPen);
            pt.setRenderHint(QPainter::Antialiasing);
            QRect rect(pix.rect());
            drawBase(isActive()?fcolors[m_type]:color(Bg), pt, rect);
            if (isHovered())
            {
                pt.setBrush(color(Fg));
                QRect r(QPoint(), QSize(ISIZE, ISIZE));
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
    const QColor c(color(Mid));
    const quint64 check((r.width()*r.height())
                        | ((quint64)(dConf.deco.buttons+1)<<24)
                        | ((quint64)isActive()<<28)
                        | ((quint64)isHovered()<<30)
                        | ((quint64)c.rgba()<<32));
    if (!m_bgPix.contains(check))
    {
        if (dConf.deco.buttons == NoStyle)
        {
            QPixmap pix(buttonRect().size());
            const int s(pix.width()/8);
            const QPen pen(color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
            m_bgPix.insert(check, Render::sunkenized(pix.rect(), pix, isDark(), c));
        }
        else
        {
            QPixmap pix(r.size());
            pix.fill(Qt::transparent);
            QPainter pt(&pix);
            pt.setPen(Qt::NoPen);
            pt.setRenderHint(QPainter::Antialiasing);
            QRect rect(pix.rect());
            drawBase(isActive()?fcolors[m_type]:color(Bg), pt, rect);
            if (isHovered())
            {
                QRect r(QPoint(), QSize(ISIZE, ISIZE));
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
    const quint64 check((buttonRect().width()*buttonRect().height())|((quint64)all<<28)|((quint64)c.rgba()<<32));
    if (!m_bgPix.contains(check))
    {
        QPixmap pix(buttonRect().size());
        const int s(pix.width()/8);
        const QPen pen(color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
        p.drawTiledPixmap(buttonRect(), Render::sunkenized(pix.rect(), pix, isDark(), c));
        m_bgPix.insert(check, Render::sunkenized(pix.rect(), pix, isDark(), c));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

void
ButtonBase::paintWindowMenuButton(QPainter &p)
{
    const QColor c(color(Mid));
    const quint64 check((buttonRect().width()*buttonRect().height())|((quint64)c.rgba()<<32));
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
        m_bgPix.insert(check, Render::sunkenized(pix.rect(), pix, isDark(), color(Mid)));
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
    const quint64 check((buttonRect().width()*buttonRect().height())|((quint64)c.rgba()<<32));
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
        m_bgPix.insert(check, Render::sunkenized(pix.rect(), pix, isDark(), color(Mid)));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

void
ButtonBase::paintKeepBelowButton(QPainter &p)
{
    const QColor c(color(keepBelow()?Highlight:Mid));
    const quint64 check((buttonRect().width()*buttonRect().height())|((quint64)c.rgba()<<32));
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
        m_bgPix.insert(check, Render::sunkenized(pix.rect(), pix, isDark(), color(Mid)));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

void
ButtonBase::paintShadeButton(QPainter &p)
{
    const QColor c(color(shade()?Highlight:Mid));
    const quint64 check((buttonRect().width()*buttonRect().height())|((quint64)c.rgba()<<32));
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
        m_bgPix.insert(check, Render::sunkenized(pix.rect(), pix, isDark(), color(Mid)));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

void
ButtonBase::paintQuickHelpButton(QPainter &p)
{
    const QColor c(color(Mid));
    const quint64 check((buttonRect().width()*buttonRect().height())|((quint64)c.rgba()<<32));
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
        m_bgPix.insert(check, Render::sunkenized(pix.rect(), pix, isDark(), c));
    }
    p.drawTiledPixmap(buttonRect(), m_bgPix.value(check));
}

////////////////////////////////////////////////////////////////////////////////////////

Button::Button(Type type, QWidget *parent)
    : ButtonBase(type),
      QWidget(parent)
{
    setAttribute(Qt::WA_Hover);
    setFixedSize(16, 16);
    setForegroundRole(QPalette::ButtonText);
    setBackgroundRole(QPalette::Button);
}

void
Button::onClick(const Qt::MouseButton &btn)
{
    switch (type())
    {
    case Close: window()->close(); break;
    case Min: window()->showMinimized(); break;
    case Max: window()->isMaximized()?window()->showNormal():window()->showMaximized(); break;
    default: break;
    }
}

//void
//Button::mousePressEvent(QMouseEvent *e)
//{
//    ButtonBase::processMouseEvent(e);
//}

//void
//Button::mouseReleaseEvent(QMouseEvent *e)
//{
//    ButtonBase::processMouseEvent(e);
//}

const QColor
Button::color(const ColorRole &c) const
{
    if (c==Highlight)
        return palette().color(QPalette::Highlight);
    if (c==Mid)
        return Color::mid(palette().color(foregroundRole()), palette().color(backgroundRole()));
    return palette().color(c==Fg?foregroundRole():backgroundRole());
}

const bool
Button::isDark() const
{
    const QWidget *w(parentWidget()?parentWidget():this);
    return Color::luminosity(w->palette().color(w->foregroundRole())) > Color::luminosity(w->palette().color(w->backgroundRole()));
}

void
Button::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    ButtonBase::paint(p);
    p.end();
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
