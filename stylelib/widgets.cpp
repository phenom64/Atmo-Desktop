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

ButtonBase::ButtonBase(Type type)
    : m_type(type)
    , m_hasPress(false)
    , m_hasMouse(false)
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
ButtonBase::processMouseEvent(QMouseEvent *e)
{
    switch (e->type())
    {
    case QEvent::MouseButtonPress:
    {
        e->accept();
        m_hasPress = true;
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        if (m_hasPress)
        {
            e->accept();
            m_hasPress = false;
            if (buttonRect().contains(e->pos()))
                onClick(e, m_type);
        }
        break;
    }
    case QEvent::MouseMove:
    {
        bool hadMouse(m_hasMouse);
        m_hasMouse = buttonRect().contains(e->pos());
        if (hadMouse != m_hasMouse)
            hoverChanged();
    }
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
        const QColor low(Color::mid(c, Qt::black, 5, 3+isDark()*10));
        const QColor high(QColor(255, 255, 255, qMin(255.0f, bgLum*1.1f)));
        r.adjust(2, 2, -2, -2);
        p.setBrush(high);
        p.drawEllipse(r.translated(0, 1));
        p.setBrush(low);
        p.drawEllipse(r);
        r.adjust(1, 1, -1, -1);

        QRadialGradient rg(r.center()+QPoint(1, r.height()/2-1), r.height()-1);
        rg.setColorAt(0.0f, Color::mid(c, Qt::white));
        rg.setColorAt(0.5f, c);
        rg.setColorAt(1.0f, Color::mid(c, Qt::black));
        p.setBrush(rg);
        p.drawEllipse(r);

        QRect rr(r);
        rr.setWidth(6);
        rr.setHeight(3);
        rr.moveCenter(r.center());
        rr.moveTop(r.top()+1);
        QLinearGradient lg(rr.topLeft(), rr.bottomLeft());
        lg.setColorAt(0.0f, QColor(255, 255, 255, 192));
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
#if 0
    case 4:
    {
        p.save();
        p.setPen(QPen(c, 2.0f));
        r.shrink(1);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(r);
        p.restore();
        break;
    }
#endif
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

static uint fcolors[3] = { 0xFFFF7E71, 0xFFFBD185, 0xFF37CC40 };

void
ButtonBase::paintCloseButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(buttonRect());
    if (dConf.deco.buttons == -1)
    {
        const int s(buttonRect().width()/8);
        r = buttonRect().adjusted(s, s, -s, -s);
        const QPen pen(color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        r.adjust(s, s, -s, -s);
        QPixmap pix(buttonRect().size());
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        pt.drawLine(r.topLeft(), r.bottomRight());
        pt.drawLine(r.topRight(), r.bottomLeft());
        pt.end();
        p.drawTiledPixmap(buttonRect(), Render::sunkenized(buttonRect(), pix, isDark(), color(Mid)));
    }
    else
    {
        drawBase(isActive()?fcolors[m_type]:color(Bg), p, r);
        if (isHovered())
        {
            p.setBrush(color(Fg));
            p.drawEllipse(r.adjusted(3, 3, -3, -3));
        }
    }
}

void
ButtonBase::paintMaxButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(buttonRect());
    if (dConf.deco.buttons == -1)
    {
        const int s(buttonRect().width()/8);
        QRect r = buttonRect().adjusted(s, s, -s, -s);
        const QPen pen(color(isMaximized()?Highlight:Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        r.adjust(s, s, -s, -s);
        QPixmap pix(buttonRect().size());
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        int x, y, w, h;
        r.getRect(&x, &y, &w, &h);
        pt.drawLine(x+w/2, y, x+w/2, y+h);
        pt.drawLine(x, y+h/2, x+w, y+h/2);
        pt.end();
        p.drawTiledPixmap(buttonRect(), Render::sunkenized(buttonRect(), pix, isDark(), color(Mid)));
    }
    else
    {
        drawBase(isActive()?fcolors[m_type]:color(Bg), p, r);
        if (isHovered())
        {
            p.setBrush(color(Fg));
            p.drawEllipse(r.adjusted(3, 3, -3, -3));
        }
    }
}

void
ButtonBase::paintMinButton(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r(buttonRect());
    if (dConf.deco.buttons == -1)
    {
        const int s(buttonRect().width()/8);
        QRect r = buttonRect().adjusted(s, s, -s, -s);
        const QPen pen(color(Mid), s*2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        r.adjust(s, s, -s, -s);
        QPixmap pix(buttonRect().size());
        pix.fill(Qt::transparent);
        QPainter pt(&pix);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(pen);
        int x, y, w, h;
        r.getRect(&x, &y, &w, &h);
        pt.drawLine(x, y+h/2, x+w, y+h/2);
        pt.end();
        p.drawTiledPixmap(buttonRect(), Render::sunkenized(buttonRect(), pix, isDark(), color(Mid)));
    }
    else
    {
        drawBase(isActive()?fcolors[m_type]:color(Bg), p, r);
        if (isHovered())
        {
            p.setBrush(color(Fg));
            p.drawEllipse(r.adjusted(3, 3, -3, -3));
        }
    }
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
Button::mousePressEvent(QMouseEvent *e)
{
    ButtonBase::processMouseEvent(e);
}

void
Button::mouseReleaseEvent(QMouseEvent *e)
{
    ButtonBase::processMouseEvent(e);
}

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
        m_hasPress = false;
        hide();
        setParent(0);
        m_splitter = 0;
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
