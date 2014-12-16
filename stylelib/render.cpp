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

#include "render.h"
#include "color.h"
#include "settings.h"
#include "macros.h"
#include "ops.h"

Q_DECL_EXPORT Render Render::m_instance;
QPixmap Render::m_mask[][Render::PartCount];

/* blurring function below from:
 * http://stackoverflow.com/questions/3903223/qt4-how-to-blur-qpixmap-image
 * unclear to me who wrote it.
 */
QImage
Render::blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly)
{
    if (image.isNull())
        return QImage();
   int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
   int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

   QImage result = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
   int r1 = rect.top();
   int r2 = rect.bottom();
   int c1 = rect.left();
   int c2 = rect.right();

   int bpl = result.bytesPerLine();
   int rgba[4];
   unsigned char* p;

   int i1 = 0;
   int i2 = 3;

   if (alphaOnly)
       i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

   for (int col = c1; col <= c2; col++) {
       p = result.scanLine(r1) + col * 4;
       for (int i = i1; i <= i2; i++)
           rgba[i] = p[i] << 4;

       p += bpl;
       for (int j = r1; j < r2; j++, p += bpl)
           for (int i = i1; i <= i2; i++)
               p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
   }

   for (int row = r1; row <= r2; row++) {
       p = result.scanLine(row) + c1 * 4;
       for (int i = i1; i <= i2; i++)
           rgba[i] = p[i] << 4;

       p += 4;
       for (int j = c1; j < c2; j++, p += 4)
           for (int i = i1; i <= i2; i++)
               p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
   }

   for (int col = c1; col <= c2; col++) {
       p = result.scanLine(r2) + col * 4;
       for (int i = i1; i <= i2; i++)
           rgba[i] = p[i] << 4;

       p -= bpl;
       for (int j = r1; j < r2; j++, p -= bpl)
           for (int i = i1; i <= i2; i++)
               p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
   }

   for (int row = r1; row <= r2; row++) {
       p = result.scanLine(row) + c2 * 4;
       for (int i = i1; i <= i2; i++)
           rgba[i] = p[i] << 4;

       p -= 4;
       for (int j = c1; j < c2; j++, p -= 4)
           for (int i = i1; i <= i2; i++)
               p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
   }

   return result;
}

/*
// Exponential blur, Jani Huhtanen, 2006 ==========================
*  expblur(QImage &img, int radius)
*
*  In-place blur of image 'img' with kernel of approximate radius 'radius'.
*  Blurs with two sided exponential impulse response.
*
*  aprec = precision of alpha parameter in fixed-point format 0.aprec
*  zprec = precision of state parameters zR,zG,zB and zA in fp format 8.zprec
*/

template<int aprec, int zprec>
static inline void blurinner(unsigned char *bptr, int &zR, int &zG, int &zB, int &zA, int alpha)
{
    int R,G,B,A;
    R = *bptr;
    G = *(bptr+1);
    B = *(bptr+2);
    A = *(bptr+3);

    zR += (alpha * ((R<<zprec)-zR))>>aprec;
    zG += (alpha * ((G<<zprec)-zG))>>aprec;
    zB += (alpha * ((B<<zprec)-zB))>>aprec;
    zA += (alpha * ((A<<zprec)-zA))>>aprec;

    *bptr =     zR>>zprec;
    *(bptr+1) = zG>>zprec;
    *(bptr+2) = zB>>zprec;
    *(bptr+3) = zA>>zprec;
}

template<int aprec,int zprec>
static inline void blurrow( QImage & im, int line, int alpha)
{
    int zR,zG,zB,zA;

    QRgb *ptr = (QRgb *)im.scanLine(line);

    zR = *((unsigned char *)ptr    )<<zprec;
    zG = *((unsigned char *)ptr + 1)<<zprec;
    zB = *((unsigned char *)ptr + 2)<<zprec;
    zA = *((unsigned char *)ptr + 3)<<zprec;

    for(int index=1; index<im.width(); index++)
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);

    for(int index=im.width()-2; index>=0; index--)
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
}

template<int aprec, int zprec>
static inline void blurcol( QImage & im, int col, int alpha)
{
    int zR,zG,zB,zA;

    QRgb *ptr = (QRgb *)im.bits();
    ptr+=col;

    zR = *((unsigned char *)ptr    )<<zprec;
    zG = *((unsigned char *)ptr + 1)<<zprec;
    zB = *((unsigned char *)ptr + 2)<<zprec;
    zA = *((unsigned char *)ptr + 3)<<zprec;

    for(int index=im.width(); index<(im.height()-1)*im.width(); index+=im.width())
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);

    for(int index=(im.height()-2)*im.width(); index>=0; index-=im.width())
        blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
}

void
Render::expblur(QImage &img, int radius, Qt::Orientations o)
{
    if(radius<1)
        return;

    static const int aprec = 16; static const int zprec = 7;

    // Calculate the alpha such that 90% of the kernel is within the radius. (Kernel extends to infinity)
    int alpha = (int)((1<<aprec)*(1.0f-expf(-2.3f/(radius+1.f))));

    if (o & Qt::Horizontal) {
        for(int row=0;row<img.height();row++)
            blurrow<aprec,zprec>(img,row,alpha);
    }

    if (o & Qt::Vertical) {
        for(int col=0;col<img.width();col++)
            blurcol<aprec,zprec>(img,col,alpha);
    }
}

Render
*Render::instance()
{
    return &m_instance;
}

Render::Render()
{
}

void
Render::_generateData()
{
    initMaskParts();
    initShadowParts();
    initTabs();
    makeNoise();
}

void
Render::initMaskParts()
{
    for (int i = 0; i < MAXRND+1; ++i)
    {
        const int size = i*2+1;
        QPixmap pix(size, size);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(pix.rect(), Qt::black);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setBrush(Qt::black);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(pix.rect(), i, i);
        p.end();

        m_mask[i][TopLeftPart] = pix.copy(0, 0, i, i);
        //m_mask[TopMidPart] = pix.copy(RND, 0, SIZE-RND*2, RND);
        m_mask[i][TopRightPart] = pix.copy(size-i, 0, i, i);
        //m_mask[Left] = pix.copy(0, RND, RND, SIZE-RND*2);
        //m_mask[CenterPart] = pix.copy(RND, RND, SIZE-RND*2, SIZE-RND*2);
        //m_mask[Right] = pix.copy(SIZE-RND, RND, RND, SIZE-RND*2);
        m_mask[i][BottomLeftPart] = pix.copy(0, size-i, i, i);
        //m_mask[BottomMidPart] = pix.copy(RND, SIZE-RND, SIZE-RND*2, RND);
        m_mask[i][BottomRightPart] = pix.copy(size-i, size-i, i, i);
    }
}

void
Render::initShadowParts()
{
    for (Shadow s = 0; s < ShadowCount; ++s)
    for (int r = 0; r < MAXRND+1; ++r)
    {
        const int cornerSize(qMax(3, r));
        const int size = cornerSize*2+1;
        QPixmap pix(size, size);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        switch (s)
        {
        case Sunken:
        {
            QRect rect(pix.rect());
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 255, 255, 192));
            p.setClipRect(rect.adjusted(0, rect.height()/2, 0, 0));
            p.drawRoundedRect(rect, r, r);
            p.setClipping(false);
            rect.setBottom(rect.bottom()-1);
            p.setBrush(Qt::black);
            p.drawRoundedRect(rect, r, r);

            if (r)
            {
                QImage img(pix.size(), QImage::Format_ARGB32);
                img.fill(Qt::transparent);
                QPainter pt(&img);
                pt.setRenderHint(QPainter::Antialiasing);
                pt.setPen(Qt::NoPen);
                pt.setBrush(Qt::black);
                QRectF rt(rect);
                rt.adjust(1.0f, 1.5f, -1.0f, -0.75f);
                int rnd(r);
                if (rnd)
                    --rnd;
                pt.drawRoundedRect(rt, rnd, rnd);
                pt.end();

                expblur(img, r>4?2:1);

                p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
                p.fillRect(pix.rect(), img);
            }
            else
            {
                p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
                QRectF rf(rect);
                rf.adjust(0.5f, 1.0f, -0.5f, -0.5f);
                p.fillRect(rf, Qt::black);
                p.setCompositionMode(QPainter::CompositionMode_SourceOver);
                QLinearGradient lg(rf.topLeft(), rf.bottomLeft());
                lg.setColorAt(0.0f, QColor(0, 0, 0, 170));
                lg.setColorAt(0.5f, Qt::transparent);
                p.fillRect(rf, lg);
            }
            break;
        }
        case Raised:
        {
            QImage img(pix.size(), QImage::Format_ARGB32);
            img.fill(Qt::transparent);
            QPainter pt(&img);
            pt.setRenderHint(QPainter::Antialiasing);
            pt.setPen(Qt::NoPen);
            pt.setBrush(Qt::black);
            QRectF rf(img.rect());
            float f(1.0f);
            rf.adjust(f, f, -f, -f);
            pt.drawRoundedRect(rf.translated(0.0f, 0.5f), r, r);
            pt.end();
            expblur(img, 1);
            p.fillRect(pix.rect(), img);
            break;
        }
        case Etched:
        {
            QPixmap white(pix.size()-QSize(0, 1));
            white.fill(Qt::transparent);
            QPainter pt(&white);
            pt.setRenderHint(QPainter::Antialiasing);
            pt.setBrush(QColor(255, 255, 255, 192));
            pt.setPen(Qt::NoPen);
            pt.drawRoundedRect(white.rect(), r, r);
            pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            pt.setBrush(Qt::black);
            pt.drawRoundedRect(white.rect().adjusted(1, 1, -1, -1), r-1, r-1);
            pt.end();

            QPixmap black(pix.size()-QSize(0, 1));
            black.fill(Qt::transparent);
            pt.begin(&black);
            pt.setRenderHint(QPainter::Antialiasing);
            pt.setPen(Qt::NoPen);
            pt.setBrush(Qt::black);
            pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
            pt.drawRoundedRect(black.rect(), r, r);
            pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            pt.drawRoundedRect(black.rect().adjusted(1, 1, -1, -1), r-1, r-1);
            pt.end();

            p.drawTiledPixmap(white.rect().translated(0, 1), white);
            p.drawTiledPixmap(black.rect(), black);
            break;
        }
        case Yosemite:
        {
            p.setPen(Qt::NoPen);
            p.setBrush(Qt::black);
            p.drawRoundedRect(pix.rect(), r, r);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.drawRoundedRect(pix.rect().adjusted(1, 1, -1, -1), r-1, r-1);
            break;
        }
        case Rect:
        {
            p.setPen(Qt::NoPen);
            p.setBrush(Qt::black);
            QRect rt(pix.rect());
            const int w(1);
            p.drawRoundedRect(rt, r, r);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.drawRoundedRect(rt.adjusted(w, w, -w, -w), r-w, r-w);
            break;
        }
        default: break;
        }
        p.end();
        splitShadowParts(s, r, size, pix);
    }
}

void
Render::splitShadowParts(const Shadow shadow, int roundNess, int size, const QPixmap &source)
{
    int cornerSize(qMax(3, roundNess));
    m_shadow[shadow][roundNess][TopLeftPart] = source.copy(0, 0, cornerSize, cornerSize);
    m_shadow[shadow][roundNess][TopMidPart] = source.copy(cornerSize, 0, size-cornerSize*2, cornerSize);
    m_shadow[shadow][roundNess][TopRightPart] = source.copy(size-cornerSize, 0, cornerSize, cornerSize);
    m_shadow[shadow][roundNess][LeftPart] = source.copy(0, cornerSize, cornerSize, size-cornerSize*2);
    m_shadow[shadow][roundNess][CenterPart] = source.copy(cornerSize, cornerSize, size-cornerSize*2, size-cornerSize*2);
    m_shadow[shadow][roundNess][RightPart] = source.copy(size-cornerSize, cornerSize, cornerSize, size-cornerSize*2);
    m_shadow[shadow][roundNess][BottomLeftPart] = source.copy(0, size-cornerSize, cornerSize, cornerSize);
    m_shadow[shadow][roundNess][BottomMidPart] = source.copy(cornerSize, size-cornerSize, size-cornerSize*2, cornerSize);
    m_shadow[shadow][roundNess][BottomRightPart] = source.copy(size-cornerSize, size-cornerSize, cornerSize, cornerSize);
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
Render::initTabs()
{
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
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawPath(path);
    p.end();
    img = blurred(img, img.rect(), ts);
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
    renderShadow(Sunken, img.rect(), &p, ts*4, Top, 0.2f);
    p.end();
    --hsz;
    hsz/=2;
    --vsz;
    vsz/=2;
    m_tab[TopLeftPart] = QPixmap::fromImage(img.copy(0, 0, hsz, vsz));
    m_tab[TopMidPart] = QPixmap::fromImage(img.copy(hsz, 0, 1, vsz));
    m_tab[TopRightPart] = QPixmap::fromImage(img.copy(hsz+1, 0, hsz, vsz));
    m_tab[LeftPart] = QPixmap::fromImage(img.copy(0, vsz, hsz, 1));
    m_tab[CenterPart] = QPixmap::fromImage(img.copy(hsz, vsz, 1, 1));
    m_tab[RightPart] = QPixmap::fromImage(img.copy(hsz+1, vsz, hsz, 1));
    m_tab[BottomLeftPart] = QPixmap::fromImage(img.copy(0, vsz+1, hsz, vsz));
    m_tab[BottomMidPart] = QPixmap::fromImage(img.copy(hsz, vsz+1, 1, vsz));
    m_tab[BottomRightPart] = QPixmap::fromImage(img.copy(hsz+1, vsz+1, hsz, vsz));
}

void
Render::_renderTab(const QRect &r, QPainter *p, const Tab t, QPainterPath *path, const float o)
{
    const QSize sz(m_tab[TopLeftPart].size());
    if (r.width()*2+1 < sz.width()*2+1)
        return;
    int x1, y1, x2, y2;
    r.getCoords(&x1, &y1, &x2, &y2);
    int halfH(r.width()-(sz.width()*2)), halfV(r.height()-(sz.height()*2));
    p->save();
    p->setOpacity(o);

    if (t > BeforeSelected)
    {
        p->drawTiledPixmap(QRect(x1+sz.width()+halfH, y1, sz.width(), sz.height()), m_tab[TopRightPart]);
        p->drawTiledPixmap(QRect(x1+sz.width()+halfH, y1+sz.height(), sz.width(), halfV), m_tab[RightPart]);
        p->drawTiledPixmap(QRect(x1+sz.width()+halfH, y1+sz.height()+halfV, sz.width(), sz.height()), m_tab[BottomRightPart]);
    }
    if (t < AfterSelected)
    {
        p->drawTiledPixmap(QRect(x1, y1, sz.width(), sz.height()), m_tab[TopLeftPart]);
        p->drawTiledPixmap(QRect(x1, y1+sz.height(), sz.width(), halfV), m_tab[LeftPart]);
        p->drawTiledPixmap(QRect(x1, y1+sz.height()+halfV, sz.width(), sz.height()), m_tab[BottomLeftPart]);
    }
    p->drawTiledPixmap(QRect(x1+sz.width(), y1, halfH, sz.height()), m_tab[TopMidPart]);
    p->drawTiledPixmap(QRect(x1+sz.width(), y1+sz.height(), halfH, halfV), m_tab[CenterPart]);
    p->drawTiledPixmap(QRect(x1+sz.width(), y1+sz.height()+halfV, halfH, sz.height()), m_tab[BottomMidPart]);

    if (path)
        *path = tab(r.adjusted(ts, 0, -ts, 0), dConf.tabs.safrnd);
    p->restore();
}

QRect
Render::partRect(const QRect &rect, const Part part, int roundNess, const Sides sides, bool isShadow)
{
    if (isShadow && roundNess < 3)
        roundNess = 3;
    int x, y, w, h;
    rect.getRect(&x, &y, &w, &h);

    int midWidth = w;
    if (sides&Right)
        midWidth-=roundNess;
    if (sides&Left)
        midWidth-=roundNess;

    int midHeight = h;
    if (sides&Top)
        midHeight-=roundNess;
    if (sides&Bottom)
        midHeight-=roundNess;

    int left(bool(sides&Left)*roundNess)
            , top(bool(sides&Top)*roundNess)
            , right(bool(sides&Right)*roundNess)
            , bottom(bool(sides&Bottom)*roundNess);

    switch (part)
    {
    case TopLeftPart: return QRect(0, 0, roundNess, roundNess);
    case TopMidPart: return QRect(left, 0, midWidth, roundNess);
    case TopRightPart: return QRect(w-right, 0, roundNess, roundNess);
    case LeftPart: return QRect(0, top, roundNess, midHeight);
    case CenterPart: return QRect(left, top, midWidth, midHeight);
    case RightPart: return QRect(w-right, top, roundNess, midHeight);
    case BottomLeftPart: return QRect(0, h-bottom, roundNess, roundNess);
    case BottomMidPart: return QRect(left, h-bottom, midWidth, roundNess);
    case BottomRightPart: return QRect(w-right, h-bottom, roundNess, roundNess);
    default: return QRect();
    }
}

bool
Render::isCornerPart(const Part part) const
{
    return part==TopLeftPart||part==TopRightPart||part==BottomLeftPart||part==BottomRightPart;
}

void
Render::shapeCorners(QWidget *w, QPainter *p, Sides s, int roundNess)
{
    const QPainter::CompositionMode mode(p->compositionMode());
    const QBrush brush(p->brush());
    const QPen pen(p->pen());
    p->setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::black);
    for (int i = 0; i < PartCount; ++i)
    if (needPart(i, s))
    {
        if (i != CenterPart && !roundNess)
            continue;
        p->drawTiledPixmap(partRect(w->rect(), i, roundNess, s), m_mask[roundNess][i]);
    }
    p->setCompositionMode(mode);
    p->setBrush(brush);
    p->setPen(pen);
}

QPixmap
Render::genPart(const Part part, const QPixmap &source, const int roundNess, const Sides sides) const
{
    QPixmap rt = source.copy(partRect(source.rect(), part, roundNess, sides));
    if (!isCornerPart(part) || !roundNess)
        return rt;
    QPainter p(&rt);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.drawTiledPixmap(rt.rect(), m_mask[roundNess][part]);
    p.end();
    return rt;
}

bool
Render::needPart(const Part part, const Sides sides)
{
    switch (part)
    {
    case TopLeftPart: return bool((sides&Top)&&(sides&Left));
    case TopMidPart: return bool(sides&Top);
    case TopRightPart: return bool((sides&Top)&&(sides&Right));
    case LeftPart: return bool(sides&Left);
    case CenterPart: return true;
    case RightPart: return bool(sides&Right);
    case BottomLeftPart: return bool((sides&Bottom)&&(sides&Left));
    case BottomMidPart: return bool(sides&Bottom);
    case BottomRightPart: return bool((sides&Bottom)&&(sides&Right));
    default: return false;
    }
}

void
Render::_renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess, const Sides sides, const QPoint &offSet)
{
    if (!rect.isValid())
        return;
    roundNess = qMin(qMin(MAXRND, roundNess), qMin(rect.height(), rect.width())/2);

    QPixmap pix(rect.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    QRect r(pix.rect());
    p.setBrushOrigin(offSet);
    p.fillRect(r, brush);
    p.end();

    for (int i = 0; i < PartCount; ++i)
        if (needPart(i, sides))
        {
            if (i != CenterPart && !roundNess)
                continue;
            const QPixmap &partPix = genPart(i, pix, roundNess, sides);
            painter->drawPixmap(partRect(QRect(QPoint(0, 0), rect.size()), i, roundNess, sides).translated(rect.x(), rect.y()), partPix);
        }
}

void
Render::_renderShadowPrivate(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess, const float opacity, const Sides sides)
{
    if (!rect.isValid())
        return;
    roundNess = qMin(qMin(MAXRND, roundNess), qMin(rect.height(), rect.width())/2);

    const float o(painter->opacity());
    painter->setOpacity(opacity);
    for (int i = 0; i < PartCount; ++i)
        if (i != CenterPart)
            if (needPart((Parts)i, sides))
            {
                QPixmap pix = m_shadow[shadow][roundNess][i];
                painter->drawTiledPixmap(partRect(QRect(QPoint(0, 0), rect.size()), (Parts)i, roundNess, sides, true).translated(rect.x(), rect.y()), pix);
            }
    painter->setOpacity(o);
}

void
Render::_renderShadow(const Shadow shadow, const QRect &rect, QPainter *painter, int roundNess, const Sides sides, const float opacity, const QBrush *brush)
{
    if (!brush)
    {
        _renderShadowPrivate(shadow, rect, painter, roundNess, opacity, sides);
        return;
    }
    QPixmap pix(rect.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    _renderShadowPrivate(shadow, pix.rect(), &p, roundNess, 1.0f, sides);
    p.end();
    colorizePixmap(pix, *brush);
    painter->drawTiledPixmap(rect, pix);
}

Render::Sides
Render::checkedForWindowEdges(const QWidget *w, Sides from)
{
    if (!w)
        return from;
    QPoint topLeft = w->mapTo(w->window(), w->rect().topLeft());
    QRect winRect = w->window()->rect();
    QRect widgetRect = QRect(topLeft, w->size());

    if (widgetRect.left() <= winRect.left())
        from &= ~Render::Left;
    if (widgetRect.right() >= winRect.right())
        from &= ~Render::Right;
    if (widgetRect.bottom() >= winRect.bottom())
        from &= ~Render::Bottom;
    if (widgetRect.top() <= winRect.top())
        from &= ~Render::Top;
    return from;
}

Render::Sides
Render::checkedForParentEdges(const QWidget *w, Sides from)
{
    if (!w || !w->parentWidget())
        return from;

    if (!w->geometry().x())
        from &= ~Render::Left;
    if (w->geometry().right() >= w->parentWidget()->rect().right())
        from &= ~Render::Right;
    if (w->geometry().bottom() >= w->parentWidget()->rect().bottom())
        from &= ~Render::Bottom;
    if (!w->geometry().y())
        from &= ~Render::Top;
    return from;
}


void
Render::colorizePixmap(QPixmap &pix, const QBrush &b)
{
    QPixmap copy(pix);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.fillRect(pix.rect(), b);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawPixmap(pix.rect(), copy);
    p.end();
}

QPixmap
Render::colorized(QPixmap pix, const QBrush &b)
{
    colorizePixmap(pix, b);
    return pix;
}

static int randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

void
Render::makeNoise()
{
    static int s(512);
    QImage noise(s, s, QImage::Format_ARGB32);
    noise.fill(Qt::transparent);
    QRgb *rgb = reinterpret_cast<QRgb *>(noise.bits());
    const int size(s*s);
    for (int i = 0; i < size; ++i)
    {
        int v(randInt(0, 255));
        rgb[i] = QColor(v, v, v).rgb();
    }
    if (dConf.uno.enabled&&dConf.uno.noiseStyle || !dConf.uno.enabled&&dConf.windows.noiseStyle)
    {
        const int b(32);
        QImage small = noise.copy(0, 0, 256, 256);
        expblur(small, b, Qt::Horizontal);
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
    m_noise = QPixmap::fromImage(noise);
}

QPixmap
Render::mid(const QPixmap &p1, const QPixmap &p2, const int a1, const int a2, const QSize &sz)
{
    int w(qMin(p1.width(), p2.width()));
    int h(qMin(p1.height(), p2.height()));
    if (sz.isValid())
    {
        w = sz.width();
        h = sz.height();
    }
    QImage i1 = p1.copy(0, 0, w, h).toImage().convertToFormat(QImage::Format_ARGB32);
    QImage i2 = p2.copy(0, 0, w, h).toImage().convertToFormat(QImage::Format_ARGB32);
    const int size(w*h);
    QImage i3(w, h, QImage::Format_ARGB32); //the resulting image
    i3.fill(Qt::transparent);
    QRgb *rgb1 = reinterpret_cast<QRgb *>(i1.bits());
    QRgb *rgb2 = reinterpret_cast<QRgb *>(i2.bits());
    QRgb *rgb3 = reinterpret_cast<QRgb *>(i3.bits());

    for (int i = 0; i < size; ++i)
    {
        const QColor c = Color::mid(QColor::fromRgba(rgb1[i]), QColor::fromRgba(rgb2[i]), a1, a2);
        rgb3[i] = c.rgba();
    }
    return QPixmap::fromImage(i3);
}

QPixmap
Render::mid(const QPixmap &p1, const QBrush &b, const int a1, const int a2)
{
    QPixmap p2(p1.size());
    p2.fill(Qt::transparent);
    QPainter p(&p2);
    p.fillRect(p2.rect(), b);
    p.end();
    return mid(p1, p2, a1, a2);
}

void
Render::drawClickable(Shadow s,
                      QRect r,
                      QPainter *p,
                      int rnd,
                      float opacity,
                      const QWidget *w,
                      const QStyleOption *opt,
                      QBrush *mask,
                      QBrush *shadow,
                      const Sides sides,
                      const QPoint &offSet)
{
    rnd = qMin(rnd, qMin(r.height(), r.width())/2);
    if (s >= ShadowCount)
        return;

    const bool isToolBox(w && qobject_cast<const QToolBox *>(w->parentWidget()));
    const bool sunken(opt && opt->state & QStyle::State_Selected|QStyle::State_On|QStyle::State_NoChange);
    if (opt
            && (opt->state & (QStyle::State_Sunken | QStyle::State_On) || qstyleoption_cast<const QStyleOptionTab *>(opt) && opt->state & QStyle::State_Selected)
            && s != Carved && s != Yosemite && !isToolBox)
    {
        if (s == Raised)
            r.sAdjust(1, 1+(r.width()!=r.height()), -1, -1);
        s = Sunken;   
    }

    bool needStrong(qobject_cast<const QSlider *>(w));
    int bgLum(255), fgLum(0), pbgLum(255), pfgLum(0);
    if (w)
    {
        int count(0);
//        int r(0), g(0), b(0);
        QColor bgc(Qt::white);
        int bgl(255);
        if (mask && mask->gradient())
        {
            const QGradientStops stops(mask->gradient()->stops());
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
//                int tr, tg, tb;
//                stops.at(i).second.getRgb(&tr, &tg, &tb);
//                r+=tr;
//                g+=tg;
//                b+=tb;
            }
        }
        //checkboxes have windowtext as fg, and button(bg) as bg... so we just simply check the bg from opposingrole...
        const bool isCheckRadio(qobject_cast<const QCheckBox *>(w)||qobject_cast<const QRadioButton *>(w));
        QPalette::ColorRole bg(sunken&&isCheckRadio?QPalette::Highlight:w->backgroundRole());
        QPalette::ColorRole fg(sunken&&isCheckRadio?QPalette::HighlightedText:Ops::opposingRole(bg));
        bgLum = count?bgl:Color::luminosity(w->palette().color(QPalette::Active, bg));
        fgLum = Color::luminosity(w->palette().color(QPalette::Active, fg));
//        if (checked)
//            Ops::swap(bgLum, fgLum);
        if (QWidget *p = w->parentWidget())
        {
            pbgLum = Color::luminosity(p->palette().color(QPalette::Active, p->backgroundRole()));
            pfgLum = Color::luminosity(p->palette().color(QPalette::Active, p->foregroundRole()));
        }
        else
        {
            pbgLum = bgLum;
            pfgLum = fgLum;
        }
    }

//    const bool isDark(fgLum>bgLum);
    const bool darkParent(pfgLum>pbgLum);
    const bool parentContrast(qMax(pbgLum, bgLum)-qMin(pbgLum, bgLum) > 127);
    if (bgLum > pbgLum && !darkParent)
        opacity = qMin(1.0f, opacity+((bgLum-pbgLum)/255.0f));

    if (opt && !(opt->ENABLED) && !qstyleoption_cast<const QStyleOptionToolButton *>(opt))
        opacity/=2.0f;

    if (isToolBox && s!=Carved) //carved gets special handling, need to save that for now
        r = w->rect();
    const int o(opacity*255);
    if (s==Raised || s==Yosemite)
    {
        if (s==Yosemite && !shadow)
        {
            QLinearGradient lg(0, 0, 0, r.height());
            lg.setColorAt(0.0f, QColor(0, 0, 0, o/3));
            lg.setColorAt(0.8f, QColor(0, 0, 0, o/3));
            lg.setColorAt(1.0f, QColor(0, 0, 0, o));
            QBrush sh(lg);
            renderShadow(s, r, p, rnd, sides, opacity, &sh);
        }
        else
            renderShadow(s, r, p, rnd, sides, opacity, shadow);
        const bool inToolBar(w&&qobject_cast<const QToolBar *>(w->parentWidget()));
        const int m(2);
        if (s==Yosemite)
            r.sAdjust(!inToolBar, !inToolBar, -!inToolBar, -1);
        else
            r.sAdjust(m, m, -m, -m);
        if (!inToolBar || s==Raised)
            rnd = qMax(0, rnd-m);
    }
    else if (s==Carved)
    {
        QLinearGradient lg(0, 0, 0, r.height());
        if (isToolBox)
            r = w->rect();
        int high(darkParent?32:192), low(darkParent?170:85);
        lg.setColorAt(0.1f, QColor(0, 0, 0, low));
        lg.setColorAt(1.0f, QColor(255, 255, 255, high));
        renderMask(r, p, lg, rnd, sides, offSet);
        const int m(qMin(r.height(), r.width())<9?2:3);
        const bool needHor(!qobject_cast<const QRadioButton *>(w)&&!qobject_cast<const QCheckBox *>(w)&&r.width()>r.height());
        r.sAdjust((m+needHor), m, -(m+needHor), -m);
        rnd = qMax(rnd-m, 0);
    }
    else
        r.sAdjust(0, 0, 0, -1);

    if (mask)
        renderMask(r, p, *mask, rnd, sides, offSet);
    else if (s==Carved)
    {
        QBrush newMask(Qt::black);
        const QPainter::CompositionMode mode(p->compositionMode());
        p->setCompositionMode(QPainter::CompositionMode_DestinationOut);
        renderMask(r, p, newMask, rnd, sides, offSet);
        p->setCompositionMode(mode);
    }

    if (s==Carved)
    {
        const float o(parentContrast?qMax(pbgLum, bgLum)/255.0f:opacity);
        renderShadow(Rect, r, p, rnd, sides, o);
    }
    else if (s==Sunken||s==Etched)
    {
        if (needStrong)
            renderShadow(Rect, r, p, rnd, sides, opacity);
        r.sAdjust(0, 0, 0, 1);
        renderShadow(s, r, p, rnd, sides, opacity);
    }
    else if (s==Raised && !isToolBox)
    {
        QLinearGradient lg(0, 0, 0, r.height());
        lg.setColorAt(0.0f, QColor(255, 255, 255, dConf.shadows.opacity*255.0f));
        lg.setColorAt(0.5f, Qt::transparent);
        QBrush b(lg);
        renderShadow(Rect, r, p, rnd, sides, 1.0f, &b);

        if (sides & (Left|Right))
        {
            QLinearGradient edges(0, 0, r.width(), 0);
            const QColor edge(QColor(0, 0, 0, dConf.shadows.opacity*127.0f));
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
            renderMask(r, p, edges, rnd, sides);
        }
    }
    else if (s==Rect)
        renderShadow(s, r, p, rnd, sides, opacity);
}

Render::Pos
Render::pos(const Sides s, const Qt::Orientation o)
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
Render::maskHeight(const Shadow s, const int height)
{
    switch (s)
    {
    case Render::Sunken:
    case Render::Etched:
        return height-1;
    case Render::Raised:
        return height-4;
    case Render::Yosemite:
        return height-1;
    case Render::Carved:
        return height-6;
    default: return 0;
    }
}

int
Render::maskWidth(const Shadow s, const int width)
{
    switch (s)
    {
    case Render::Sunken:
    case Render::Etched:
    case Render::Yosemite:
        return width;
    case Render::Raised:
        return width-2;
    case Render::Carved:
        return width-6;
    default: return 0;
    }
}

QRect
Render::maskRect(const Shadow s, const QRect &r, const Sides sides)
{
    //sides needed for sAdjusted macro even if seemingly unused
    switch (s)
    {
    case Yosemite:
    case Sunken:
    case Etched: return r.sAdjusted(0, 0, 0, -1); break;
    case Raised: return r.sAdjusted(2, 2, -2, -(2+(r.height()!=r.width()))); break;
    case Carved: return r.sAdjusted(3, 3, -3, -3); break;
    default: return r;
    }
}

int
Render::shadowMargin(const Shadow s)
{
    switch (s)
    {
    case Sunken:
    case Etched:
    case Raised: return 1; break;
    case Yosemite: return 0; break;
    case Carved: return 3; break;
    default: return 0;
    }
}

QPixmap
Render::sunkenized(const QRect &r, const QPixmap &source, const bool isDark, const QColor &ref)
{
    int m(4);
    QImage img(r.size()+QSize(m, m), QImage::Format_ARGB32);
    m/=2;
    img.fill(Qt::transparent);
    QPainter p(&img);
    int rgb(isDark?255:0);
    int alpha(qMin(255.0f, 2*dConf.shadows.opacity*255.0f));
    p.fillRect(img.rect(), QColor(rgb, rgb, rgb, alpha));
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.drawTiledPixmap(r.translated(m, m), source);
    p.end();
    QImage blur(img);
    Render::expblur(blur, m*2);
//    const QImage blur = Render::blurred(img, img.rect(), m*2);
    QPixmap highlight(r.size());
    highlight.fill(Qt::transparent);
    p.begin(&highlight);
    rgb = isDark?0:255;
    p.fillRect(highlight.rect(), QColor(rgb, rgb, rgb, alpha));
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawTiledPixmap(highlight.rect(), source);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.drawTiledPixmap(highlight.rect().translated(0, -1), source);
    p.end();

//    const int y(isDark?-1:1);
    QPixmap pix(r.size());
    pix.fill(Qt::transparent);
    p.begin(&pix);
    p.drawTiledPixmap(pix.rect(), source);
    if (!isDark)
        p.drawTiledPixmap(pix.rect().translated(0, 1), QPixmap::fromImage(blur), QPoint(m, m));
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawTiledPixmap(pix.rect(), source);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    p.drawTiledPixmap(pix.rect().translated(0, 1), highlight);
    p.end();
    return pix;
}

//colortoalpha directly stolen from gimp

void
colortoalpha (float *a1,
          float *a2,
          float *a3,
          float *a4,
          float c1,
          float c2,
          float c3)
{
  float alpha1, alpha2, alpha3, alpha4;

  alpha4 = *a4;

  if ( *a1 > c1 )
    alpha1 = (*a1 - c1)/(255.0-c1);
  else if ( *a1 < c1 )
    alpha1 = (c1 - *a1)/(c1);
  else alpha1 = 0.0;

  if ( *a2 > c2 )
    alpha2 = (*a2 - c2)/(255.0-c2);
  else if ( *a2 < c2 )
    alpha2 = (c2 - *a2)/(c2);
  else alpha2 = 0.0;

  if ( *a3 > c3 )
    alpha3 = (*a3 - c3)/(255.0-c3);
  else if ( *a3 < c3 )
    alpha3 = (c3 - *a3)/(c3);
  else alpha3 = 0.0;

  if ( alpha1 > alpha2 )
    if ( alpha1 > alpha3 )
      {
    *a4 = alpha1;
      }
    else
      {
    *a4 = alpha3;
      }
  else
    if ( alpha2 > alpha3 )
      {
    *a4 = alpha2;
      }
    else
      {
    *a4 = alpha3;
      }

  *a4 *= 255.0;

  if ( *a4 < 1.0 )
    return;
  *a1 = 255.0 * (*a1-c1)/ *a4 + c1;
  *a2 = 255.0 * (*a2-c2)/ *a4 + c2;
  *a3 = 255.0 * (*a3-c3)/ *a4 + c3;

  *a4 *= alpha4/255.0;
}

static int stretch(const int v, const float n = 2.0f)
{
    static bool isInit(false);
    static float table[256];
    if (!isInit)
    {
        for (int i = 127; i < 256; ++i)
            table[i] = (pow(((float)i/127.5f-1.0f), (1.0f/n))+1.0f)*127.5f;
        for (int i = 0; i < 128; ++i)
            table[i] = 255-table[255-i];
        isInit = true;
    }
    return qRound(table[v]);
}

/**
    http://spatial-analyst.net/ILWIS/htm/ilwisapp/stretch_algorithm.htm
    OUTVAL
    (INVAL - INLO) * ((OUTUP-OUTLO)/(INUP-INLO)) + OUTLO
    where:
    OUTVAL
    Value of pixel in output map
    INVAL
    Value of pixel in input map
    INLO
    Lower value of 'stretch from' range
    INUP
    Upper value of 'stretch from' range
    OUTLO
    Lower value of 'stretch to' range
    OUTUP
    Upper value of 'stretch to' range
 */

static QImage stretched(QImage img, const QColor &c)
{
    img = img.convertToFormat(QImage::Format_ARGB32);
    int size = img.width() * img.height();
    QRgb *pixels[2] = { 0, 0 };
    pixels[0] = reinterpret_cast<QRgb *>(img.bits());
    int r, g, b;
    c.getRgb(&r, &g, &b); //foregroundcolor

#define ENSUREALPHA if (!qAlpha(pixels[0][i])) continue
    QList<int> hues;
    for (int i = 0; i < size; ++i)
    {
        ENSUREALPHA;
        const int hue(QColor::fromRgba(pixels[0][i]).hue());
        if (!hues.contains(hue))
            hues << hue;
    }

    const bool useAlpha(hues.count() == 1);
    if (useAlpha)
    {
        int l(0), h(0);
        for (int i = 0; i < size; ++i)
        {
            ENSUREALPHA;
            const QColor c(QColor::fromRgba(pixels[0][i]));
            if (Color::luminosity(c) < 128)
                ++l;
            else
                ++h;
        }
        const bool dark(l*2>h);
        /**
          * Alpha gets special treatment...
          * we simply steal the colortoalpha
          * function from gimp, color white to
          * alpha, as in remove all white from
          * the image (in case of monochrome icons
          * add some sorta light bevel). Then simply
          * push all remaining alpha values so the
          * highest one is 255.
          */
        float alpha[size], lowAlpha(255), highAlpha(0);
        for (int i = 0; i < size; ++i)
        {
            ENSUREALPHA;
            const QRgb rgb(pixels[0][i]);
            float val(dark?255.0f:0.0f);
            float red(qRed(rgb)), green(qGreen(rgb)), blue(qBlue(rgb)), a(qAlpha(rgb));
            colortoalpha(&red, &green, &blue, &a, val, val, val);
            if (a < lowAlpha)
                lowAlpha = a;
            if (a > highAlpha)
                highAlpha = a;
            alpha[i] = a;
        }
        float add(255.0f/highAlpha);
        for (int i = 0; i < size; ++i)
        {
            ENSUREALPHA;
            pixels[0][i] = qRgba(r, g, b, stretch(qMin<int>(255, alpha[i]*add), 2.0f));
        }
        return img;
    }

    const int br(img.width()/4);
    QImage bg(img.size() + QSize(br*2, br*2), QImage::Format_ARGB32); //add some padding avoid 'edge folding'
    bg.fill(Qt::transparent);

    QPainter bp(&bg);
    bp.drawPixmap(br, br, QPixmap::fromImage(img), 0, 0, img.width(), img.height());
    bp.end();
    Render::blurred(bg, bg.rect(), br);
    bg = bg.copy(bg.rect().adjusted(br, br, -br, -br)); //remove padding so we can easily access relevant pixels with [i]

    enum Rgba { Red = 0, Green, Blue, Alpha };
    enum ImageType { Fg = 0, Bg }; //fg is the actual image, bg is the blurred one we use as reference for the most contrasting channel

    pixels[1] = reinterpret_cast<QRgb *>(bg.bits());
    double rgbCount[3] = { 0.0d, 0.0d, 0.0d };
    int (*rgba[4])(QRgb rgb) = { qRed, qGreen, qBlue, qAlpha };
    int cVal[2][4] = {{ 0, 0, 0, 0 }, {0, 0, 0, 0}};

    for (int i = 0; i < size; ++i) //pass 1, determine most contrasting channel, usually RED
    for (int it = 0; it < 2; ++it)
    for (int d = 0; d < 4; ++d)
    {
        ENSUREALPHA;
        cVal[it][d] = (*rgba[d])(pixels[it][i]);
        if (it && d < 3)
            rgbCount[d] += (cVal[Fg][Alpha]+cVal[Bg][Alpha]) * qAbs(cVal[Fg][d]-cVal[Bg][d]) / (2.0f * cVal[Bg][d]);
    }
    double count = 0;
    int channel = 0;
    for (int i = 0; i < 3; ++i)
        if (rgbCount[i] > count)
        {
            channel = i;
            count = rgbCount[i];
        }
    float values[size];
    float inLo = 255.0f, inHi = 0.0f;
    qulonglong loCount(0), hiCount(0);
    for ( int i = 0; i < size; ++i ) //pass 2, store darkest/lightest pixels
    {
        ENSUREALPHA;
        const float px((*rgba[channel])(pixels[Fg][i]));
        values[i] = px;
        if ( px > inHi )
            inHi = px;
        if ( px < inLo )
            inLo = px;
        const int a(qAlpha(pixels[Fg][i]));
        if (px < 128)
            loCount += a*(255-px);
        else
            hiCount += a*px;
    }
    const bool isDark(loCount>hiCount);
    for ( int i = 0; i < size; ++i )
    {
        ENSUREALPHA;
        int a(qBound<int>(0, qRound((values[i] - inLo) * (255.0f / (inHi - inLo))), 255));
        if (isDark)
            a = qMax(0, qAlpha(pixels[Fg][i])-a);
        pixels[Fg][i] = qRgba(r, g, b, stretch(a));
    }
#undef ENSUREALPHA
    return img;
}

QPixmap
Render::monochromized(const QPixmap &source, const QColor &color, const Effect effect, bool isDark)
{
    const QPixmap &result = QPixmap::fromImage(stretched(source.toImage(), color));
    if (effect == Inset)
        return sunkenized(result.rect(), result, isDark);
    return result;
}

