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
#include <QMainWindow>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QStyleOptionToolButton>
#include <QVBoxLayout>

#include "fx.h"
#include "gfx.h"
#include "color.h"
#include "../config/settings.h"
#include "macros.h"
#include "ops.h"
#include "handlers.h"
#include "masks.h"
#include "shadows.h"
#include "windowhelpers.h"
#include "windowdata.h"
#include "xhandler.h"

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
        s_shadow[r][s][1] = new Shadow(s, r, dConf.shadows.opacity, dConf.shadows.illumination);
        s_shadow[r][s][0] = new Shadow(s, r, dConf.shadows.opacity*0.5f, dConf.shadows.illumination*0.5f);
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
    path.closeSubpath();
    return path;
}

static const int ts(4); //tab shadow...

static QPixmap s_tbs;

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
    FX::expblur(img, 2);
    p.begin(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::black, 2.0f));
    p.setBrush(Qt::black);
    p.drawPath(path);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.fillPath(path, Qt::black);
    p.end();
    s_tab[CenterPart] = QPixmap::fromImage(img.copy(ts, 0, 1, img.height()));

    QImage tbs(1, 8, QImage::Format_ARGB32_Premultiplied);
    tbs.fill(Qt::transparent);
    p.begin(&tbs);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::black, 3.0f));
    p.drawLine(0, 0, 1, 0);
    p.end();
    FX::expblur(tbs, 2);
    p.begin(&tbs);
    p.setPen(QPen(Qt::black, 2.0f));
    p.drawLine(0, 0, 1, 0);
    p.end();

    p.begin(&img);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.fillRect(img.rect(), QColor(0, 0, 0, 0xff-(dConf.shadows.opacity)));
    p.fillRect(0, 0, img.width(), 8, tbs);
//    drawTabBarShadow(&p, img.rect());
    p.end();
    p.begin(&tbs);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.fillRect(tbs.rect(), QColor(0, 0, 0, 0xff-(dConf.shadows.opacity)));
    p.end();
    s_tbs = QPixmap::fromImage(tbs);
    --hsz;
    hsz/=2;
    --vsz;
    vsz/=2;
    s_tab[TopLeftPart] = QPixmap::fromImage(img.copy(0, 0, hsz, vsz));
    s_tab[TopMidPart] = QPixmap::fromImage(img.copy(hsz, 0, 1, vsz));
    s_tab[TopRightPart] = QPixmap::fromImage(img.copy(hsz+1, 0, hsz, vsz));
    s_tab[LeftPart] = QPixmap::fromImage(img.copy(0, vsz, hsz, 1));
    s_tab[RightPart] = QPixmap::fromImage(img.copy(hsz+1, vsz, hsz, 1));
    s_tab[BottomLeftPart] = QPixmap::fromImage(img.copy(0, vsz+1, hsz, vsz));
    s_tab[BottomMidPart] = QPixmap::fromImage(img.copy(hsz, vsz+1, 1, vsz));
    s_tab[BottomRightPart] = QPixmap::fromImage(img.copy(hsz+1, vsz+1, hsz, vsz));
}

void
GFX::drawTabBarShadow(QPainter *p, QRect r)
{
    r.setHeight(s_tbs.height());
    p->drawTiledPixmap(r, s_tbs);
}

void
GFX::drawTab(const QRect &r, QPainter *p, const TabPos t, QPainterPath *path)
{
    const QSize sz(s_tab[TopLeftPart].size());
    if (r.width()*2+1 < sz.width()*2+1)
        return;

    int x1, y1, x2, y2;
    r.getCoords(&x1, &y1, &x2, &y2);
    int halfH(r.width()-(sz.width()*2)), halfV(r.height()-(sz.height()*2));

    if (t > BeforeSelected)
    {
        p->drawTiledPixmap(QRect(x1+sz.width()+halfH, y1, sz.width(), sz.height()), s_tab[TopRightPart]);
        p->drawTiledPixmap(QRect(x1+sz.width()+halfH, y1+sz.height(), sz.width(), halfV), s_tab[RightPart]);
        p->drawTiledPixmap(QRect(x1+sz.width()+halfH, y1+sz.height()+halfV, sz.width(), sz.height()), s_tab[BottomRightPart]);
    }
    if (t < AfterSelected)
    {
        p->drawTiledPixmap(QRect(x1, y1, sz.width(), sz.height()), s_tab[TopLeftPart]);
        p->drawTiledPixmap(QRect(x1, y1+sz.height(), sz.width(), halfV), s_tab[LeftPart]);
        p->drawTiledPixmap(QRect(x1, y1+sz.height()+halfV, sz.width(), sz.height()), s_tab[BottomLeftPart]);
    }
    p->drawTiledPixmap(QRect(x1+sz.width(), y1+sz.height()+halfV, halfH, sz.height()), s_tab[BottomMidPart]);

    if (path)
        *path = tab(r.adjusted(ts, 0, -ts, 0), dConf.tabs.safrnd);
}

void
GFX::drawMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess, const Sides sides, const QPoint &offset)
{
    if (roundNess)
        roundNess = Mask::maxRnd(rect, sides, roundNess);
    Mask::render(rect, brush, painter, roundNess, sides, offset);
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
                   QBrush mask,
                   int rnd,
                   int hover,
                   Sides sides,
                   const QStyleOption *opt,
                   const QWidget *w,
                   QPoint offset,
                   const bool invertedSides)
{
    if (s >= ShadowCount)
        return;

    bool sunken(opt && opt->state & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_Selected | QStyle::State_NoChange));
    const ShadowStyle shadow(s);
    bool isLineEdit(qobject_cast<const QLineEdit *>(w)||qobject_cast<const QSpinBox *>(w));
    if (qobject_cast<const QComboBox *>(w) && static_cast<const QComboBox *>(w)->isEditable())
    {
        isLineEdit = true;
        sunken = static_cast<const QComboBox *>(w)->lineEdit()->hasFocus();
    }
    else if (isLineEdit)
        sunken = w->hasFocus();
    if (isLineEdit && sunken)
        hover = Steps;
    bool inActive(false);
    if (dConf.differentInactive
            && !WindowHelpers::isActiveWindow(w)
            && !sunken
            && s!=Carved&&s!=SemiCarved
            && qobject_cast<const QToolBar *>(w->parentWidget()))
    {
        QToolBar *bar = static_cast<QToolBar *>(w->parentWidget());
        const QMainWindow *win = qobject_cast<QMainWindow *>(bar->window());
        inActive = win && win->toolBarArea(bar) == Qt::TopToolBarArea;
    }

    bool isEnabled(!opt || (qstyleoption_cast<const QStyleOptionToolButton *>(opt) || (opt->state & QStyle::State_Enabled)));
    if (inActive)
        isEnabled = false;
    if (opt
            && (sunken || qstyleoption_cast<const QStyleOptionTab *>(opt) && opt->state & QStyle::State_Selected)
            && (s == Etched || s == Raised))
        s = Sunken;

    const quint8 m = shadowMargin(s);

    if (rnd)
        rnd = Mask::maxRnd(r, sides, rnd);

    switch (s)
    {
    case Yosemite:
    {
        if (w&&qobject_cast<const QToolBar *>(w->parentWidget()))
        {
            drawShadow(s, r, p, isEnabled, rnd, sides);
            r.sAdjust(0, 0, 0, -1);
        }
        break;
    }
    case Carved:
    case SemiCarved:
    {
        bool hor(false);
        if (mask.gradient() && mask.gradient()->type() == QGradient::LinearGradient)
        {
            const QLinearGradient *grad = static_cast<const QLinearGradient *>(mask.gradient());
//            qDebug() << grad->start() << grad->finalStop();
            if (grad->start().y() == grad->finalStop().y())
                hor = true;
        }
//        if (w && w->parentWidget() && qobject_cast<QVBoxLayout *>(w->parentWidget()->layout()))
//            hor = true;
        QLinearGradient lg(r.topLeft(), hor ? r.topRight() : r.bottomLeft());
        lg.setColorAt(0, QColor(0, 0, 0, dConf.shadows.opacity));
        lg.setColorAt(1, QColor(255, 255, 255, dConf.shadows.illumination));
        drawMask(r, p, lg, rnd, sides);
        int add(m);
        const bool needHor(!qobject_cast<const QRadioButton *>(w)&&!qobject_cast<const QCheckBox *>(w)&&r.width()!=r.height()&&s==Carved);
        if (!needHor && s==Carved)
            add = 1;
        r.sAdjust((add+needHor), add, -(add+needHor), -add);
        rnd = qMax(rnd-add, 0);
        break;
    }
    case LagaDesk:
    {
        rnd -= m;
        if (rnd < 0)
            rnd = 0;
    }
    default: r.sShrink(m); break;
    }

    /// BEGIN ACTUAL PLATE PAINTING

    if (!isEnabled) //this is a tad bit expensive but.... the window is usually active when heavy painting occurs
    {
        QPixmap pix(r.size());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.fillRect(pix.rect(), mask);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(pix.rect(), QColor(0, 0, 0, 127));
        p.end();
        mask = pix;
        offset = r.topLeft();
    }

    drawMask(r, p, mask, rnd-(m*(s==Raised)), sides, offset);

    /// END PLATE

    switch (s)
    {
    case Carved:
    case SemiCarved: drawShadow(Rect, r, p, isEnabled, rnd, sides); r.sGrow(1); break;
    case Sunken:
    case Etched:
    case Raised: r.sGrow(m); drawShadow(s, r, p, isEnabled, rnd, sides); break;
    case Yosemite: if (!w||!qobject_cast<const QToolBar *>(w->parentWidget())) drawShadow(s, r, p, isEnabled, rnd, sides); break;
    case Rect:
    case ElCapitan: drawShadow(s, r, p, isEnabled, rnd, sides); break;
    case LagaDesk:
    {
        r.sGrow(m);
        rnd += m;
        drawShadow(LagaDesk, r, p, isEnabled, rnd, sides);
        break;
    }
    default: r.sGrow(m); break;
    }

    if (invertedSides && sunken && (shadow == Etched || shadow == Raised) && sides != All)
    {
        const Sides saved(sides);
        static const quint8 sm = GFX::shadowMargin(Sunken);
        QRect mr(r.sAdjusted(sm, sm, -sm, -sm));
        sides = All & ~sides;
        mr.sAdjust(-sm, -sm, sm, sm);
        drawShadow(s, mr, p, isEnabled, rnd, sides);
        sides = saved;


//        QPixmap pix(r.size());
//        pix.fill(Qt::transparent);
//        QPainter pt(&pix);
//        const Sides saved(sides);
//        static const quint8 sm = GFX::shadowMargin(Sunken);
//        QRect mr(r.sAdjusted(sm, sm, -sm, -sm));
//        sides = All-sides;
//        mr.sAdjust(-sm, -sm, sm, sm);
//        sides = saved;
//        GFX::drawShadow(Sunken, mr, &pt, isEnabled, rnd, All-sides);
//        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
//        GFX::drawShadow(Sunken, mr, &pt, isEnabled, rnd, sides);
//        pt.fillRect(pix.rect(), QColor(0, 0, 0, 0xff-dConf.shadows.opacity));
//        pt.end();
//        p->drawTiledPixmap(r, pix);
    }

    if (hover && (!sunken || isLineEdit) && s != Yosemite)
    {
        QColor highLight = opt ? opt->palette.color(QPalette::Highlight) : qApp->palette().color(QPalette::Highlight);
        highLight.setAlpha((255/Steps) * hover);

        if (m)
            Hover::render(r, highLight, p, rnd, sides, hover);
//        highLight.setHsv(highLight.hue(), 255, 255);
//        highLight.setAlpha(highLight.alpha()>>1);
//        const QPainter::CompositionMode mode = p->compositionMode();
//        p->setCompositionMode(QPainter::CompositionMode_Screen);
//        GFX::drawMask(r, p, highLight, rnd, sides);
//        p->setCompositionMode(mode);
    }
}

//void something()
//{
//    int bgLum(127), fgLum(127), pbgLum(127), pfgLum(127);
//    if (w)
//    {
//        int count(0);
//        QColor bgc(Qt::white);
//        int bgl(255);
//        if (mask.gradient())
//        {
//            const QGradientStops stops(mask.gradient()->stops());
//            count = stops.count();
//            for (int i = 0; i < count; ++i)
//            {
//                const QColor nbgc(stops.at(i).second);
//                const int nbgl(Color::luminosity(nbgc));
//                if (nbgl < bgl)
//                {
//                    bgl = nbgl;
//                    bgc = nbgc;
//                }
//            }
//        }
//        //checkboxes have windowtext as fg, and button(bg) as bg... so we just simply check the bg from opposingrole...
//        const bool isCheckRadio(qobject_cast<const QCheckBox *>(w)||qobject_cast<const QRadioButton *>(w));
//        QPalette::ColorRole bg(sunken&&isCheckRadio?QPalette::Highlight:w->backgroundRole());
//        QPalette::ColorRole fg(sunken&&isCheckRadio?QPalette::HighlightedText:Ops::opposingRole(bg));
//        bgLum = count?bgl:Color::luminosity(w->palette().color(QPalette::Active, bg));
//        fgLum = Color::luminosity(w->palette().color(QPalette::Active, fg));
//        if (QWidget *parent = w->parentWidget())
//        {
//            pbgLum = Color::luminosity(parent->palette().color(QPalette::Active, parent->backgroundRole()));
//            pfgLum = Color::luminosity(parent->palette().color(QPalette::Active, parent->foregroundRole()));
//        }
//        else
//        {
//            pbgLum = bgLum;
//            pfgLum = fgLum;
//        }
//    }
//}

quint8
GFX::shadowMargin(const ShadowStyle s)
{
    return Shadow::margin(s);
}

void
GFX::drawCheckMark(QPainter *p, const QColor &c, const QRect &r, const bool tristate, const bool bevel)
{
    if (bevel)
    {
        const int v = Color::lum(c);
        const int rgb = v < 128 ? 255 : 0;
        drawCheckMark(p, QColor(rgb, rgb, rgb, dConf.shadows.opacity), r.translated(0, 1), tristate);
    }

    const bool aa(p->testRenderHint(QPainter::Antialiasing));
    const QPen sPen(p->pen());
    const QBrush sBrush(p->brush());

    QPen pen(c, 2.0f, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
    p->setPen(pen);
    p->setRenderHint(QPainter::Antialiasing, false);

    if (tristate)
    {
        p->drawLine(QPoint(r.left()+3, r.center().y()), QPoint(r.right()-2, r.center().y()));
    }
    else
    {
        int size = qMin(r.width(), r.height());
        while (size % 3)
            --size;

        QRect rect(0, 0, size, size);
        rect.moveCenter(r.center()+QPoint(0, -2));
        int x,y,w,h;
        rect.getRect(&x, &y, &w, &h);
        int third = size/3;
        const int points[] = { x,y+size-(third-1), x+third-1,y+size, x+size,y+third-1 };
        p->setBrush(Qt::NoBrush);
        p->drawPolyline(QPolygon(3, points));
    }
    p->setBrush(sBrush);
    p->setPen(sPen);
    p->setRenderHint(QPainter::Antialiasing, aa);
}

void
GFX::drawRadioMark(QPainter *p, const QColor &c, const QRect &r, const bool bevel)
{
    if (bevel)
    {
        const int v = Color::lum(c);
        const int rgb = v < 128 ? 255 : 0;
        drawRadioMark(p, QColor(rgb, rgb, rgb, dConf.shadows.opacity), r.translated(0, 1));
    }

    const bool aa(p->testRenderHint(QPainter::Antialiasing));
    const QPen sPen(p->pen());
    const QBrush sBrush(p->brush());

    p->setBrush(c);
    p->setPen(Qt::NoPen);
    p->setRenderHint(QPainter::Antialiasing);
    p->drawEllipse(r);

    p->setBrush(sBrush);
    p->setPen(sPen);
    p->setRenderHint(QPainter::Antialiasing, aa);
}

void
GFX::drawArrow(QPainter *p, const QColor &c, QRect r, const Direction d, int size, const Qt::Alignment align, const bool bevel)
{
    if (bevel)
    {
        const int v = Color::lum(c);
        const int rgb = v < 128 ? 255 : 0;
        drawArrow(p, QColor(rgb, rgb, rgb, rgb?dConf.shadows.illumination:dConf.shadows.opacity), r.translated(0, 1), d, size, align);
    }

    size = qBound(7, size, qMin(r.width(), r.height()));
    if (!(size%2))
        --size;

    if (!dConf.simpleArrows)
        size -= 2;

    const bool hor(d == West || d == East);
    QRect rect;
    if (hor)
        rect.setRect(0, 0, (size>>1)+1, size);
    else
        rect.setRect(0, 0, size, (size>>1)+1);

    rect = subRect(r, align, rect);

    QPointF points[3];

    if (dConf.simpleArrows)
    {
        int x = rect.x(), y = rect.y();
        switch (d)
        {
        case East: { for (int i = 0; i < rect.width(); ++i) p->fillRect(QRect(x+i, y+i, 1, rect.height()-i*2), c); return; }
        case South:{ for (int i = 0; i < rect.height(); ++i) p->fillRect(QRect(x+i, y+i, rect.width()-i*2, 1), c); return; }
        case West: { for (int i = rect.width()-1; i >= 0; --i) p->fillRect(QRect(x+(rect.width()-i), y+i, 1, rect.height()-i*2), c); return; }
        case North:{ for (int i = rect.height()-1; i >= 0; --i)p->fillRect(QRect(x+i, y+(rect.height()-i), rect.width()-i*2, 1), c); return; }
        default: return;
        }
        return;
    }
    rect.translate(1, 1);
    switch (d)
    {
    case East:
    {
        points[0] = rect.topLeft();
        points[1] = QPoint(rect.right(), rect.center().y());
        points[2] = rect.bottomLeft();
        break;
    }
    case South:
    {
        points[0] = rect.topLeft();
        points[1] = QPoint(rect.center().x(), rect.bottom());
        points[2] = rect.topRight();
        break;
    }
    case West:
    {
        points[0] = rect.topRight();
        points[1] = QPoint(rect.x(), rect.center().y());
        points[2] = rect.bottomRight();
        break;
    }
    case North:
    {
        points[0] = rect.bottomLeft();
        points[1] = QPoint(rect.center().x(), rect.y());
        points[2] = rect.bottomRight();
        break;
    }
    default: break;
    }

    const QPen pen(p->pen());
    const QBrush brush(p->brush());
    const bool aa(p->testRenderHint(QPainter::Antialiasing));
    p->setRenderHint(QPainter::Antialiasing, true);
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(c, 2.0f, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p->drawPolyline(points, 3);
    p->setPen(pen);
    p->setBrush(brush);
    p->setRenderHint(QPainter::Antialiasing, aa);
}

void
GFX::drawHandle(QPainter *p, const QRect &r, const bool hor)
{
    if (hor)
    {
        for (int i = r.top()+1; i < r.bottom(); i+=3)
        {
            const QRect rect(r.left(), i, r.width(), 1);
            p->fillRect(rect, QColor(0,0,0,dConf.shadows.opacity));
            p->fillRect(rect.translated(0, 1), QColor(255,255,255,dConf.shadows.illumination));
        }
    }
    else
    {
        for (int i = r.left()+1; i < r.right(); i+=3)
        {
            const QRect rect(i, r.top(), 1, r.height());
            p->fillRect(rect, QColor(0,0,0,dConf.shadows.opacity));
            p->fillRect(rect.translated(1, 0), QColor(255,255,255,dConf.shadows.illumination));
        }
    }

}

static int randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

enum Noise { ASDF = -1, RandomNoise = 0, BrushedMetal = 1, WhiteDots = 2, DiagonalStripes = 3, RaisedDots = 4, Donpo = 5, BigStripes = 6 };

QPixmap
GFX::noisePix(const qint8 style, const QString &fileName)
{
    switch (style)
    {
    case WhiteDots:
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
    case DiagonalStripes:
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
    case RaisedDots:
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
    case Donpo:
    {
        QImage img(3, 3, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QRgb *rgb = reinterpret_cast<QRgb *>(img.bits());
        const int size(img.width()*img.height());
        static const int px[3*3] = { 134, 20, 217,
                                     83, 255, 0,
                                     191, 134, 230 };
        for (int i = 0; i < size; ++i)
        {
//            qDebug() << ((px[i] - 42.0f) * ((255.0f-0.0f)/(82.0f-42.0f)) + 0);
            const int v(px[i]);
            rgb[i] = qRgb(v, v, v);
        }
        return QPixmap::fromImage(img);
    }
    case BigStripes:
    {
        static const int s(48);
        QImage img(s, s, QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
//        p.setPen(QPen(Qt::black, s/3));
//        p.drawLine(img.rect().topLeft()/*-QPoint(s/4, 0)*/, img.rect().bottomRight()/*+QPoint(s/4, 0)*/);

        const int fourthW(img.width()/4);
        const int fourthH(img.height()/4);
        const int w(img.width());
        const int h(img.height());
        const int topRight[] = { w-fourthW,0, w,0, w,fourthH };
        const int main[] = { 0,0, fourthW,0, w,h-fourthH, w,h, w-fourthW,h, 0,fourthH };
        const int bottomLeft[] = { 0,h-fourthH, fourthW,h, 0,h };
        p.setBrush(Qt::black);
        p.drawPolygon(QPolygon(3, topRight));
        p.drawPolygon(QPolygon(6, main));
        p.drawPolygon(QPolygon(3, bottomLeft));

        p.end();
        return QPixmap::fromImage(img);
    }
    case ASDF:
    {
        const QString file = QString("%1/.local/share/data/dsp/%2").arg(QDir::homePath(), fileName); //needs to be from xdg
//        qDebug() << file;
        QImageReader reader(file);
        if (!reader.canRead())
            return noisePix(RandomNoise);
        QImage img = reader.read();
        FX::autoStretch(img);
        return QPixmap::fromImage(img);
//        img = FX::stretched(img, Qt::black);
//        const int size = img.width() * img.height();
//        QRgb *px = reinterpret_cast<QRgb *>(img.bits());
//        for (int i = 0; i < size; ++i)
//            px...
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
        if (style == BrushedMetal)
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
            FX::autoStretch(noise);
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

    if (s_noise[0].isNull())
        s_noise[0] = noisePix(dConf.uno.noiseStyle, dConf.uno.noiseFile);
    const QColor bg = qApp->palette().color(QPalette::Window);
    static QPixmap wNoise;
    if (wNoise.isNull())
        wNoise = noisePix(dConf.windows.noiseStyle, dConf.windows.noiseFile);

    s_noise[1] = FX::mid(wNoise, bg, dConf.windows.noise, 100-dConf.windows.noise);
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

QRect
GFX::subRect(const QRect &r, const int flags, const QRect &sr)
{
    int x,y,w,h;
    QRect(r.topLeft(), sr.size()).getRect(&x, &y, &w, &h);
    if (sr == r)
        return sr;
    if (flags & Qt::AlignHCenter)
        x += ((r.size().width()>>1)-(w>>1));
    if (flags & Qt::AlignVCenter)
        y += ((r.size().height()>>1)-(h>>1));
    if (flags & Qt::AlignLeft)
        x = r.x();
    else if (flags & Qt::AlignRight)
        x += r.width()-w;
    if (flags & Qt::AlignTop)
        y = r.y();
    else if (flags & Qt::AlignBottom)
        y += r.height()-h;
    return QRect(x, y, w, h);
}

void
GFX::drawWindowBg(QPainter *p, const QWidget *w, const QColor &bg, const QRect &rect, QPoint offset)
{
    if (!w)
        return;

    if (w->autoFillBackground() /*&& XHandler::opacity() == 0xff*/)
        return;

    if (!w->isWindow())
    {
        QWidget *parent = w->parentWidget();
        while (parent)
        {
            if (parent->isWindow())
                break;
            if (parent->autoFillBackground() /*&& XHandler::opacity() == 0xff*/)
            {
                p->fillRect(w->rect(), parent->palette().color(parent->backgroundRole()));
                return;
            }
            else if (QTabWidget *tw = qobject_cast<QTabWidget *>(parent))
            {
                if (!tw->documentMode() && !dConf.tabs.regular)
                    p->fillRect(w->rect(), QColor(0,0,0,15));
            }
            parent = parent->parentWidget();
        }
    }
    int l(0),t(0),r(0),b(0);
    if (!dConf.uno.enabled)
//    if (WindowData *data = WindowData::memory(w->window()->winId(), w->window()))
    {
        const quint32 wid = w->isWindow() ? XHandler::windowId(w) : XHandler::windowId(w->window());
        XHandler::getDecoBorders(l,r,t,b, wid);
        offset += QPoint(0, t);
    }
    if (dConf.windows.noise)
        p->drawTiledPixmap(rect, GFX::noise(true), offset);
    else
        p->fillRect(rect, bg);

    if (!dConf.windows.gradient.isEmpty())
    {
        QRect wr(rect);
        if (!w->isWindow())
            wr = QRect(w->mapFrom(w->window(), QPoint()), w->window()->size());
        wr.adjust(-l, -t, r, b);
        QLinearGradient lg(wr.topLeft(), wr.bottomLeft());
        if (dConf.windows.hor)
            lg = QLinearGradient(wr.topLeft(), wr.topRight());
        static QGradientStops stops;
        if (stops.isEmpty())
            stops = Settings::gradientStops(dConf.windows.gradient);
        lg.setStops(stops);
        const QPainter::CompositionMode mode = p->compositionMode();
        p->setCompositionMode(QPainter::CompositionMode_Overlay);
        p->fillRect(rect, lg);
        p->setCompositionMode(mode);
    }

    if (!w->isWindow())
    {
        QWidget *parent = w->parentWidget();
        while (parent)
        {
            if (parent->isWindow())
                break;
            if (QTabWidget *tw = qobject_cast<QTabWidget *>(parent))
            {
                if (!tw->documentMode() && !dConf.tabs.regular)
                    p->fillRect(w->rect(), QColor(0,0,0,15));
            }
            else if (QGroupBox *gb = qobject_cast<QGroupBox *>(parent))
                p->fillRect(gb->rect(), QColor(0,0,0,15));
            parent = parent->parentWidget();
        }
    }
    if (XHandler::opacity() < 0xff && !dConf.uno.enabled)
    {
        p->setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p->fillRect(w->rect(), QColor(0,0,0,XHandler::opacity()));
        p->setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
}
