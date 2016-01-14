#include <QDebug>
#include <qmath.h>
#include <QWidget>
#include <QToolBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QBitmap>
#include <QSlider>
#include <QToolBox>
#include <QStyleOption>
#include <QLineEdit>
#include <QToolButton>
#include <QPainter>
#include <QBrush>

#include "fx.h"
#include "gfx.h"
#include "color.h"
#include "../config/settings.h"
#include "macros.h"
#include "ops.h"
#include "handlers.h"
#include "masks.h"
#include "shadows.h"

using namespace DSP;

QPixmap *GFX::s_tab = 0;
QPixmap *GFX::s_noise = 0;
Shadow *GFX::s_shadow[MaxRnd+1][ShadowCount][2] = {0};

void
GFX::generateData()
{
    initShadows();
    initTabs();
    makeNoise();
}

void
GFX::initShadows()
{
    static bool init(false);
    if (init)
        return;
    init = true;
    for (int r = 0; r < MaxRnd+1; ++r)
    for (ShadowStyle s = 0; s < ShadowCount; ++s)
    {
        s_shadow[r][s][Enabled] = new Shadow(s, r, dConf.shadows.opacity);
        s_shadow[r][s][Disabled] = new Shadow(s, r, dConf.shadows.opacity*0.5f);
    }
}


static QPainterPath tab(const QRect &r, int rnd)
{
    int x1, y1, x2, y2;
    r.getCoords(&x1, &y1, &x2, &y2);
    QPainterPath path;
    path.moveTo(x2, y1);
    path.quadTo(x2-rnd, y1, x2-rnd, y1+rnd);
    path.lineTo(x2-rnd, y2-rnd);
    path.quadTo(x2-rnd, y2, x2-rnd*2, y2);
    path.lineTo(x1+rnd*2, y2);
    path.quadTo(x1+rnd, y2, x1+rnd, y2-rnd);
    path.lineTo(x1+rnd, y1+rnd);
    path.quadTo(x1+rnd, y1, x1, y1);
    path.lineTo(x2, y1);
    path.closeSubpath();
    return path;
}

static const int ts(4); //tab shadow...

void
GFX::initTabs()
{
    if (!s_tab)
        s_tab = new QPixmap[PartCount]();
    const int tr(dConf.tabs.safrnd);
    int hsz = (tr*4)+(ts*4);
    int vsz = hsz/2;
    ++hsz;
    ++vsz;
    QImage img(hsz, vsz, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    const QRect r(img.rect());
    QPainterPath path(tab(r.adjusted(ts, 0, -ts, 0), tr));
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::black, 3.0f));
    p.setBrush(Qt::black);
    p.drawPath(path);
    p.end();
//    img = FX::blurred(img, img.rect(), ts);
    FX::expblur(img, 1);
    p.begin(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::black, 2.0f));
    p.setBrush(Qt::black);
    p.drawPath(path);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut); //erase the tabbar shadow... otherwise double.
    p.setPen(Qt::NoPen);
    p.drawPath(path);
    p.setPen(Qt::black);
    p.drawLine(img.rect().topLeft(), img.rect().topRight());
    Shadow(Sunken, 0, 0xff).render(img.rect().translated(0, -1), &p, Top);
    p.end();
    --hsz;
    hsz/=2;
    --vsz;
    vsz/=2;
    s_tab[TopLeftPart] = QPixmap::fromImage(img.copy(0, 0, hsz, vsz));
    s_tab[TopMidPart] = QPixmap::fromImage(img.copy(hsz, 0, 1, vsz));
    s_tab[TopRightPart] = QPixmap::fromImage(img.copy(hsz+1, 0, hsz, vsz));
    s_tab[LeftPart] = QPixmap::fromImage(img.copy(0, vsz, hsz, 1));
    s_tab[CenterPart] = QPixmap::fromImage(img.copy(hsz, vsz, 1, 1));
    s_tab[RightPart] = QPixmap::fromImage(img.copy(hsz+1, vsz, hsz, 1));
    s_tab[BottomLeftPart] = QPixmap::fromImage(img.copy(0, vsz+1, hsz, vsz));
    s_tab[BottomMidPart] = QPixmap::fromImage(img.copy(hsz, vsz+1, 1, vsz));
    s_tab[BottomRightPart] = QPixmap::fromImage(img.copy(hsz+1, vsz+1, hsz, vsz));
}

void
GFX::drawTab(const QRect &r, QPainter *p, const TabPos t, QPainterPath *path, const quint8 o)
{
    const QSize sz(s_tab[TopLeftPart].size());
    if (r.width()*2+1 < sz.width()*2+1)
        return;

    QPixmap pix(r.size());
    pix.fill(Qt::transparent);
    QPainter pt(&pix);

    int x1, y1, x2, y2;
    pix.rect().getCoords(&x1, &y1, &x2, &y2);
    int halfH(r.width()-(sz.width()*2)), halfV(r.height()-(sz.height()*2));

    if (t > BeforeSelected)
    {
        pt.drawTiledPixmap(QRect(x1+sz.width()+halfH, y1, sz.width(), sz.height()), s_tab[TopRightPart]);
        pt.drawTiledPixmap(QRect(x1+sz.width()+halfH, y1+sz.height(), sz.width(), halfV), s_tab[RightPart]);
        pt.drawTiledPixmap(QRect(x1+sz.width()+halfH, y1+sz.height()+halfV, sz.width(), sz.height()), s_tab[BottomRightPart]);
    }
    if (t < AfterSelected)
    {
        pt.drawTiledPixmap(QRect(x1, y1, sz.width(), sz.height()), s_tab[TopLeftPart]);
        pt.drawTiledPixmap(QRect(x1, y1+sz.height(), sz.width(), halfV), s_tab[LeftPart]);
        pt.drawTiledPixmap(QRect(x1, y1+sz.height()+halfV, sz.width(), sz.height()), s_tab[BottomLeftPart]);
    }
    pt.drawTiledPixmap(QRect(x1+sz.width(), y1, halfH, sz.height()), s_tab[TopMidPart]);
    pt.drawTiledPixmap(QRect(x1+sz.width(), y1+sz.height(), halfH, halfV), s_tab[CenterPart]);
    pt.drawTiledPixmap(QRect(x1+sz.width(), y1+sz.height()+halfV, halfH, sz.height()), s_tab[BottomMidPart]);
    pt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    pt.fillRect(pix.rect(), QColor(0, 0, 0, o));
    pt.end();

    if (path)
        *path = tab(r.adjusted(ts, 0, -ts, 0), dConf.tabs.safrnd);
    p->drawPixmap(r, pix);
}

void
GFX::drawMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess, const Sides sides)
{
    Mask::render(rect, brush, painter, roundNess, sides);
}

void
GFX::drawShadow(const ShadowStyle shadow, const QRect &rect, QPainter *painter, const bool isEnabled, int roundNess, const Sides sides)
{
    if (rect.isValid() && roundNess >= 0 && roundNess <= MaxRnd)
        s_shadow[roundNess][shadow][isEnabled]->render(rect, painter, sides);
}

void
GFX::drawClickable(ShadowStyle s,
                   QRect r,
                   QPainter *p,
                   const QBrush &mask,
                   int rnd,
                   const Sides sides,
                   const QStyleOption *opt,
                   const QWidget *w)
{
    if (s >= ShadowCount)
        return;

    const bool isToolBox(w && qobject_cast<const QToolBox *>(w->parentWidget()));
    const bool isLineEdit(qobject_cast<const QLineEdit *>(w));
    const bool sunken(opt && opt->state & (QStyle::State_Selected|QStyle::State_On|QStyle::State_NoChange));
//    const bool inActive(dConf.differentInactive
//                        &&!Handlers::Window::isActiveWindow(w)
//                        &&!sunken
//                        &&!(s==Raised||s==Carved)
//                        &&qobject_cast<const QToolButton *>(w)
//                        &&qobject_cast<const QToolBar *>(w->parentWidget()));
    const bool isEnabled(!opt || (qstyleoption_cast<const QStyleOptionToolButton *>(opt) || (opt->state & QStyle::State_Enabled)));
    if (opt
            && (opt->state & (QStyle::State_Sunken | QStyle::State_On) || qstyleoption_cast<const QStyleOptionTab *>(opt) && opt->state & QStyle::State_Selected)
            && s != Carved && s != Yosemite && s != Rect && s != ElCapitan && !isToolBox && !isLineEdit)
    {
        s = Sunken;   
    }

    if (rnd)
        rnd = qBound(0, rnd, (qMin(r.height(), r.width())>>1));

    int bgLum(127), fgLum(127), pbgLum(127), pfgLum(127);
    if (w)
    {
        int count(0);
        QColor bgc(Qt::white);
        int bgl(255);
        if (mask.gradient())
        {
            const QGradientStops stops(mask.gradient()->stops());
            count = stops.count();
            for (int i = 0; i < count; ++i)
            {
                const QColor nbgc(stops.at(i).second);
                const int nbgl(Color::luminosity(nbgc));
                if (nbgl < bgl)
                {
                    bgl = nbgl;
                    bgc = nbgc;
                }
            }
        }
        //checkboxes have windowtext as fg, and button(bg) as bg... so we just simply check the bg from opposingrole...
        const bool isCheckRadio(qobject_cast<const QCheckBox *>(w)||qobject_cast<const QRadioButton *>(w));
        QPalette::ColorRole bg(sunken&&isCheckRadio?QPalette::Highlight:w->backgroundRole());
        QPalette::ColorRole fg(sunken&&isCheckRadio?QPalette::HighlightedText:Ops::opposingRole(bg));
        bgLum = count?bgl:Color::luminosity(w->palette().color(QPalette::Active, bg));
        fgLum = Color::luminosity(w->palette().color(QPalette::Active, fg));
        if (QWidget *parent = w->parentWidget())
        {
            pbgLum = Color::luminosity(parent->palette().color(QPalette::Active, parent->backgroundRole()));
            pfgLum = Color::luminosity(parent->palette().color(QPalette::Active, parent->foregroundRole()));
        }
        else
        {
            pbgLum = bgLum;
            pfgLum = fgLum;
        }
    }


    if (isToolBox && s!=Carved) //carved gets special handling, need to save that for now
        r = w->rect();

    const quint8 m = shadowMargin(s);

    switch (s)
    {
    case Yosemite:
    {
        drawShadow(s, r, p, isEnabled, rnd, sides);
        const bool inToolBar(w&&qobject_cast<const QToolBar *>(w->parentWidget()));
        r.sAdjust(!inToolBar, !inToolBar, -!inToolBar, -1);
        break;
    }
    case Carved:
    {
        QLinearGradient lg(0, 0, 0, r.height());
        if (isToolBox)
            r = w->rect();
        const bool darkParent(pfgLum>pbgLum);
        int high(darkParent?32:192), low(darkParent?170:85);
        lg.setColorAt(0, QColor(0, 0, 0, low));
        lg.setColorAt(1, QColor(255, 255, 255, high));
        drawMask(r, p, lg, rnd, sides);
        const int add(qMin(r.height(), r.width())<9?2:3);
        const bool needHor(!qobject_cast<const QRadioButton *>(w)&&!qobject_cast<const QCheckBox *>(w)&&r.width()>r.height());
        r.sAdjust((add+needHor), add, -(add+needHor), -add);
        rnd = qMax(rnd-add, 0);
        break;
    }
    default: r.sAdjust(m, m, -m, -m); break;
    }

    /// BEGIN ACTUAL PLATE PAINTING

    drawMask(r, p, mask, rnd, sides);

    /// END PLATE

    switch (s)
    {
    case Carved: drawShadow(Rect, r, p, isEnabled, rnd, sides); break;
    case Sunken:
    case Etched: drawShadow(s, r.sAdjusted(-m, -m, m, m), p, isEnabled, rnd, sides); break;
    case Raised:
    {
        if (isToolBox)
            break;
        drawShadow(s, r.sAdjusted(-m, -m, m, m), p, isEnabled, rnd, sides);
        if (dConf.shadows.darkRaisedEdges && (sides & (Left|Right)))
        {
            const float lumop(qMin<float>(255.0f, 1.5f*bgLum));
            QLinearGradient edges(0, 0, r.width(), 0);
            const QColor edge(QColor(0, 0, 0, lumop));
            const float position(5.0f/r.width());
            if (sides & Left)
            {
                edges.setColorAt(0.0f, edge);
                edges.setColorAt(position, Qt::transparent);
            }
            if (sides & Right)
            {
                edges.setColorAt(1.0f-position, Qt::transparent);
                edges.setColorAt(1.0f, edge);
            }
            drawMask(r, p, edges, rnd, sides);
        }
        break;
    }
    case Rect:
    case ElCapitan: drawShadow(s, r, p, isEnabled, rnd, sides); break;
    default: break;
    }
}

quint8
GFX::shadowMargin(const ShadowStyle s)
{
    switch (s)
    {
    case Sunken: return 1;
    case Etched: return 1;
    case Raised: return 2;
    case Carved: return 4;
    default: return 0;
    }
}

void
GFX::drawCheckMark(QPainter *p, const QColor &c, const QRect &r, const bool tristate)
{
    p->save();
    p->translate(r.topLeft());

    int size = qMin(r.width(), r.height());
    const int third = size/3, thirds = third*2, sixth=third/2;
    const int points[] = { sixth,thirds-sixth, third,size-sixth, thirds+sixth,sixth };

    p->setRenderHint(QPainter::Antialiasing);
    QPen pen(c, third*(tristate?0.33f:0.66f), tristate?Qt::DashLine:Qt::SolidLine);
    pen.setDashPattern(QVector<qreal>() << 0.05f << 1.5f);
    pen.setStyle(tristate?Qt::CustomDashLine:Qt::SolidLine);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    QPainterPath path;
    path.addPolygon(QPolygon(3, points));
    p->drawPath(path);

    p->restore();
}

void
GFX::drawArrow(QPainter *p, const QPalette::ColorRole role, const QPalette &pal, const bool enabled, const QRect &r, const Direction d, int size, const Qt::Alignment align)
{
    const QPalette::ColorRole bgRole(Ops::opposingRole(role));
    if (pal.color(role).alpha() == 0xff && pal.color(bgRole).alpha() == 0xff)
    if (role != QPalette::NoRole)
    {
        if (bgRole != QPalette::NoRole && enabled)
        {
            const bool isDark(Color::luminosity(pal.color(role)) > Color::luminosity(pal.color(bgRole)));
            const int rgb(isDark?0:255);
            const QColor bevel(rgb, rgb, rgb, 127);
            drawArrow(p, bevel, r.translated(0, 1), d, size, align);
        }
        const QColor c(pal.color(enabled ? QPalette::Active : QPalette::Disabled, role));
        drawArrow(p, c, r, d, size, align);
    }
}

void
GFX::drawArrow(QPainter *p, const QColor &c, const QRect &r, const Direction d, int size, const Qt::Alignment align, const bool bevel)
{
    if (bevel)
    {
        const int v = Color::luminosity(c);
        const int rgb = v < 128 ? 255 : 0;
        drawArrow(p, QColor(rgb, rgb, rgb, dConf.shadows.opacity), r.translated(0, 1), d, size);
    }
    p->save();
    p->setPen(Qt::NoPen);
    p->setRenderHint(QPainter::Antialiasing);

    if (!size || size > qMax(r.width(), r.height()))
        size = qMin(r.width(), r.height());
    size &= ~1;

    QRect rect(0, 0, size, size);
    if (align & (Qt::AlignVCenter|Qt::AlignHCenter))
        rect.moveCenter(r.center());
    if (align & Qt::AlignLeft)
        rect.moveLeft(r.left());
    if (align & Qt::AlignRight)
        rect.moveRight(r.right());
    if (align & Qt::AlignTop)
        rect.moveTop(r.top());
    if (align & Qt::AlignBottom)
        rect.moveBottom(r.bottom());

    p->translate(rect.topLeft());

    const int half(size >> 1);
    const int points[]  = { 0,0, size,half, 0,size };

    if (d != East)
    {
        p->translate(half, half);
        switch (d)
        {
        case South: p->rotate(90); break;
        case West: p->rotate(180); break;
        case North: p->rotate(-90); break;
        }
        p->translate(-half, -half);
    }
    p->setBrush(c);
    p->drawPolygon(QPolygon(3, points));
    p->restore();
}

static int randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

QPixmap
GFX::noisePix(const quint8 style)
{
    switch (style)
    {
    case 2:
    {
        QImage img(3, 4, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QRgb *rgb = reinterpret_cast<QRgb *>(img.bits());
        const int size(img.width()*img.height());
        static const int px[12] = { 170, 255, 0, 85, 170, 0, 85, 0, 170, 0, 85, 128 };
        for (int i = 0; i < size; ++i)
        {
            const int v(px[i]);
            rgb[i] = qRgb(v, v, v);
        }
        return QPixmap::fromImage(img);
    }
    case 3:
    {
        QImage img(3, 3, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QRgb *rgb = reinterpret_cast<QRgb *>(img.bits());
        const int size(img.width()*img.height());
        static const int px[9] = { 0, 0, 255, 0, 255, 0, 255, 0, 0 };
        for (int i = 0; i < size; ++i)
        {
            const int v(px[i]);
            rgb[i] = qRgb(v, v, v);
        }
        return QPixmap::fromImage(img);
    }
    case 4:
    {
        QImage img(4, 3, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QRgb *rgb = reinterpret_cast<QRgb *>(img.bits());
        const int size(img.width()*img.height());
        static const int px[4*3] = { 127, 127, 127, 127,
                                     127, 0, 0, 127,
                                     127, 255, 255, 127};
        for (int i = 0; i < size; ++i)
        {
            const int v(px[i]);
            rgb[i] = qRgb(v, v, v);
        }
        return QPixmap::fromImage(img);
    }
    default:
    {
        static int s(512);
        QImage noise(s, s, QImage::Format_ARGB32_Premultiplied);
        noise.fill(Qt::transparent);
        QRgb *rgb = reinterpret_cast<QRgb *>(noise.bits());
        const int size(s*s);
        for (int i = 0; i < size; ++i)
        {
            int v(randInt(0, 255));
            rgb[i] = QColor(v, v, v).rgb();
        }
        if (style == 1)
        {
            const int b(32);
            QImage small = noise.copy(0, 0, 256, 256);
            FX::expblur(small, b, Qt::Horizontal);
            small = FX::stretched(small);
            const QImage horFlip(small.mirrored(true, false));
            const QImage verFlip(small.mirrored(false));
            const QImage bothFlip(small.mirrored(true, true));

            noise.fill(Qt::transparent);
            QPainter p(&noise);
            p.drawImage(QPoint(), small);
            p.drawImage(QPoint(256, 0), horFlip);
            p.drawImage(QPoint(0, 256), verFlip);
            p.drawImage(QPoint(256, 256), bothFlip);
            p.end();
            noise = noise.copy(b>>1, b>>1, noise.width()-b, noise.height()-b);
        }
        return QPixmap::fromImage(noise);
    }
    }
    return QPixmap();
}

void
GFX::makeNoise()
{
    if (!s_noise)
        s_noise = new QPixmap[2]();

    s_noise[0] = noisePix(dConf.uno.noiseStyle);
    const QColor bg = qApp->palette().color(QPalette::Window);
    const QPixmap wNoise = noisePix(dConf.windows.noiseStyle);

    s_noise[1] = FX::mid(wNoise, bg, dConf.uno.noise, 100-dConf.uno.noise);
//    s_noise[1] = QPixmap(wNoise.size());
//    s_noise[1].fill(bg);
//    QPainter pt(&s_noise[1]);

//    QPixmap noise(wNoise.size());
//    noise.fill(Qt::transparent);
//    QPainter ptt(&noise);
//    ptt.drawTiledPixmap(noise.rect(), wNoise);
//    ptt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
//    ptt.fillRect(noise.rect(), QColor(0, 0, 0, dConf.windows.noise*2.55f));
//    ptt.end();
//    pt.setCompositionMode(QPainter::CompositionMode_Overlay);
//    pt.drawTiledPixmap(s_noise[1].rect(), noise);
//    pt.end();
}

