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
GFX::generateData(const QPalette &pal)
{
    initShadows(pal);
    initTabs();
    makeNoise();
}

void
GFX::initShadows(const QPalette &pal)
{
    const int winLum = Color::luminosity(pal.color(QPalette::Window));
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
    p.setPen(QPen(Qt::black, 2.0f));
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
    drawShadow(Sunken, img.rect(), &p, 0, Top, 0.2f);
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
    Mask::render(rect, brush, painter, qMin(roundNess, qMin(rect.height(), rect.width())/2), sides);
}

void
GFX::drawShadow(const ShadowStyle shadow, const QRect &rect, QPainter *painter, const bool isEnabled, int roundNess, const Sides sides)
{
    if (!rect.isValid())
        return;
    const quint8 r = qBound(0, roundNess, (qMin(rect.height(), rect.width())>>1));
    if (r >= 0 && r <= MaxRnd)
        s_shadow[r][shadow][isEnabled]->render(rect, painter, sides);
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
    rnd = qBound(0, rnd, (qMin(r.height(), r.width())>>1));
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
        if (s == Raised)
            r.sAdjust(1, 1+(r.width()!=r.height()), -1, -1);
        s = Sunken;   
    }

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
    const bool darkParent(pfgLum>pbgLum);
//    const bool parentContrast(qMax(pbgLum, bgLum)-qMin(pbgLum, bgLum) > 127);

    if (isToolBox && s!=Carved) //carved gets special handling, need to save that for now
        r = w->rect();

    if (s==Yosemite)
    {
        drawShadow(s, r, p, isEnabled, rnd, sides);
        const bool inToolBar(w&&qobject_cast<const QToolBar *>(w->parentWidget()));
        r.sAdjust(!inToolBar, !inToolBar, -!inToolBar, -1);
    }
    else if (s==Raised)
        r.sAdjust(2, 2, -2, -2);
    else if (s==Carved)
    {
        QLinearGradient lg(0, 0, 0, r.height());
        if (isToolBox)
            r = w->rect();
        int high(darkParent?32:192), low(darkParent?170:85);
        lg.setColorAt(0.1f, QColor(0, 0, 0, low));
        lg.setColorAt(1.0f, QColor(255, 255, 255, high));
        drawMask(r, p, lg, rnd, sides);
        const int m(qMin(r.height(), r.width())<9?2:3);
        const bool needHor(!qobject_cast<const QRadioButton *>(w)&&!qobject_cast<const QCheckBox *>(w)&&r.width()>r.height());
        r.sAdjust((m+needHor), m, -(m+needHor), -m);
        rnd = qMax(rnd-m, 0);
    }
    else if ((r.height() != r.width() || qobject_cast<const QToolButton *>(w)) && s != ElCapitan)
        r.sAdjust(0, 0, 0, -1);

    /// BEGIN ACTUAL PLATE PAINTING

    const bool n(s == Sunken && bgLum > pbgLum);
    drawMask(r.sAdjusted(n, n, -n, -n), p, mask, rnd, sides);

    /// END PLATE

    if (s==Carved)
    {
        drawShadow(Rect, r, p, isEnabled, rnd, sides);
    }
    else if (s==Sunken||s==Etched)
    {
        drawShadow(s, r.sAdjusted(0, 0, 0, 1), p, isEnabled, rnd, sides);
    }
    else if (s==Raised && !isToolBox)
    {
        drawShadow(s, r.sAdjusted(-2, -2, 2, 2), p, isEnabled, rnd, sides);
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
    }
    else if (s==Rect || s==ElCapitan)
        drawShadow(s, r, p, isEnabled, rnd, sides);
}

Pos
GFX::pos(const Sides s, const Qt::Orientation o)
{
    if (o == Qt::Horizontal)
    {
        if (s&Left&&!(s&Right))
            return First;
        else if (s&Right&&!(s&Left))
            return Last;
        else if (s==All)
            return Alone;
        else
            return Middle;
    }
    else
    {
        if (s&Top&&!(s&Bottom))
            return First;
        else if (s&Bottom&&!(s&Top))
            return Last;
        else if (s==All)
            return Alone;
        else
            return Middle;
    }
}

int
GFX::maskHeight(const ShadowStyle s, const int height)
{
    switch (s)
    {
    case Sunken:
    case Etched:
        return height-1;
    case Raised:
        return height-4;
    case Yosemite:
        return height-1;
    case Carved:
        return height-6;
    case Rect:
        return height;
    default: return height;
    }
}

int
GFX::maskWidth(const ShadowStyle s, const int width)
{
    switch (s)
    {
    case Sunken:
    case Etched:
    case Yosemite:
        return width;
    case Raised:
        return width-2;
    case Carved:
        return width-6;
    default: return 0;
    }
}

QRect
GFX::maskRect(const ShadowStyle s, const QRect &r, const Sides sides)
{
    //sides needed for sAdjusted macro even if seemingly unused
    switch (s)
    {
    case Yosemite:
    case Sunken:
    case Etched: return r.sAdjusted(0, 0, 0, -1);
    case Raised: return r.sAdjusted(2, 2, -2, -(2+(r.height()!=r.width())));
    case Carved: return r.sAdjusted(3, 3, -3, -3);
    case Rect: return r.sAdjusted(1, 1, -1, -1);
    default: return r;
    }
}

int
GFX::shadowMargin(const ShadowStyle s)
{
    switch (s)
    {
    case Sunken:
    case Etched:
    case Raised: return 3; break;
    case Yosemite: return 0; break;
    case Carved: return 4; break;
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

void
GFX::makeNoise()
{
    if (!s_noise)
        s_noise = new QPixmap[2]();
    if (dConf.uno.enabled&&dConf.uno.noiseStyle == 4 || !dConf.uno.enabled&&dConf.windows.noiseStyle == 4)
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
        s_noise[0] = QPixmap::fromImage(img);
    }
    else if (dConf.uno.enabled&&dConf.uno.noiseStyle == 3 || !dConf.uno.enabled&&dConf.windows.noiseStyle == 3)
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
        s_noise[0] = QPixmap::fromImage(img);
    }
    else if (dConf.uno.enabled&&dConf.uno.noiseStyle == 2 || !dConf.uno.enabled&&dConf.windows.noiseStyle == 2)
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
        s_noise[0] = QPixmap::fromImage(img);
    }
    else
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
        if (dConf.uno.enabled&&dConf.uno.noiseStyle == 1 || !dConf.uno.enabled&&dConf.windows.noiseStyle == 1)
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
        s_noise[0] = QPixmap::fromImage(noise);
    }
    const QColor bg = qApp->palette().color(QPalette::Window);

//    s_noise[1] = FX::mid(s_noise[0], bg, dConf.uno.noise, 100-dConf.uno.noise);
    s_noise[1] = QPixmap(s_noise[0].size());
    s_noise[1].fill(bg);
    QPainter pt(&s_noise[1]);

    QPixmap noise(s_noise[0].size());
    noise.fill(Qt::transparent);
    QPainter ptt(&noise);
    ptt.drawTiledPixmap(noise.rect(), s_noise[0]);
    ptt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    ptt.fillRect(noise.rect(), QColor(0, 0, 0, dConf.uno.noise*2.55f));
    ptt.end();
    pt.setCompositionMode(QPainter::CompositionMode_Overlay);
    pt.drawTiledPixmap(s_noise[1].rect(), noise);
    pt.end();
}

