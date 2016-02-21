#include "masks.h"
#include "fx.h"
#include "../config/settings.h"
#include <QTabBar>
#include <QPainterPath>
#include <QPainter>
#include <QBrush>
#include <QRect>
#include "color.h"

using namespace DSP;


//adapted from QPainterPath::addRoundedRect()
static void addRoundedRect(const QRectF &rect, const qreal radius, const Sides s, QPainterPath &path)
{
    QRectF r = rect.normalized();

    if (r.isNull())
        return;

//    qreal xRadius = radius;
//    qreal yRadius = radius;

//    qreal w = r.width() / 2;
//    qreal h = r.height() / 2;

//    if (w == 0) {
//        xRadius = 0;
//    } else {
//        xRadius = 100 * qMin(xRadius, w) / w;
//    }
//    if (h == 0) {
//        yRadius = 0;
//    } else {
//        yRadius = 100 * qMin(yRadius, h) / h;
//    }

    qreal x = r.x();
    qreal y = r.y();
    qreal w = r.width();
    qreal h = r.height();
    qreal rxx2 = radius*2/*w*xRadius/100*/;
    qreal ryy2 = radius*2/*h*yRadius/100*/;
    if ((s & (Top|Left)) == (Top|Left))
    {
        path.arcMoveTo(x, y, rxx2, ryy2, 180);
        path.arcTo(x, y, rxx2, ryy2, 180, -90);
    }
    else
        path.moveTo(x, y);
    if ((s & (Top|Right)) == (Top|Right))
        path.arcTo(x+w-rxx2, y, rxx2, ryy2, 90, -90);
    else
        path.lineTo(x+w, y);
    if ((s & (Bottom|Right)) == (Bottom|Right))
        path.arcTo(x+w-rxx2, y+h-ryy2, rxx2, ryy2, 0, -90);
    else
        path.lineTo(x+w, y+h);
    if ((s & (Bottom|Left)) == (Bottom|Left))
        path.arcTo(x, y+h-ryy2, rxx2, ryy2, 270, -90);
    else
        path.lineTo(x, y+h);
    path.closeSubpath();
}

void
Mask::render(const QRectF &r, const QBrush &b, QPainter *p, float round, const Sides s, const QPoint &offset)
{
    if (!r.isValid())
        return;
    if (!round || !s)
    {
        p->fillRect(r, b);
        return;
    }
    QPainterPath path;
    addRoundedRect(r, maxRnd(r, s, round), s, path);
    const bool hadAA(p->testRenderHint(QPainter::Antialiasing));
    p->setRenderHint(QPainter::Antialiasing);
    const QPoint origin(p->brushOrigin());
    if (!offset.isNull())
        p->setBrushOrigin(offset);
    p->fillPath(path, b);
    p->setBrushOrigin(origin);
    p->setRenderHint(QPainter::Antialiasing, hadAA);
}

float
Mask::maxRnd(const QRectF &r, const Sides s, const float rnd)
{
    float w = r.width(), h = r.height();
    if (s&Left && s&Right)
        w /= 2.0f;
    if (s&Top && s&Bottom)
        h /= 2.0f;
    return qMin(rnd, qMin(w, h));
}

quint8
Mask::maxRnd(const QRect &r, const Sides s, const quint8 rnd)
{
    int w = r.width(), h = r.height();
    if (s&Left && s&Right)
        w >>= 1;
    if (s&Top && s&Bottom)
        h >>= 1;
    return qMin<uint>(rnd, qMin(w, h));
}

//------------------------------------------------------------------------------

QPainterPath
Mask::Tab::chromePath(QRect r, const int shape)
{
    static const int round = 8;
    const int x = r.x(), y = r.y(), w = r.width(), h = r.height(), x2 = x+w, y2 = y+h;
    QPainterPath path;
    switch (shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
    {
        path.moveTo(x, y2);
        path.quadTo(x+round/2, y2, x+round/2, y2-round/2);
        path.quadTo(x+round, y+round/4, x+round*1.5f, y);
        path.lineTo(x2-round*1.5f, y);
        path.quadTo(x2-round, y+round/4, x2-round/2, y2-round/2);
        path.quadTo(x2-round/2, y2, x2, y2);
        break;
    }
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
    {
        path.moveTo(x, y);
        path.quadTo(x+round/2, y, x+round/2, y+round/2);
        path.quadTo(x+round, y2-round/4, x+round*1.5f, y2);
        path.lineTo(x2-round*1.5f, y2);
        path.quadTo(x2-round, y2-round/2, x2-round/2, y+round/2);
        path.quadTo(x2-round/2, y, x2, y);
        break;
    }
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
    {
        path.moveTo(x2, y);
        path.quadTo(x2, y+round/2, x2-round/2, y+round/2);
        path.quadTo(x+round/4, y+round, x, y+round*1.5f);
        path.lineTo(x, y2-round*1.5f);
        path.quadTo(x+round/4, y2-round, x2-round/2, y2-round/2);
        path.quadTo(x2, y2-round/2, x2, y2);
        break;
    }
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
    {
        path.moveTo(x, y);
        path.quadTo(x, y+round/2, x+round/2, y+round/2);
        path.quadTo(x2-round/4, y+round, x2, y+round*1.5f);
        path.lineTo(x2, y2-round*1.5f);
        path.quadTo(x2-round/4, y2-round, x+round/2, y2-round/2);
        path.quadTo(x, y2-round/2, x, y2);
        break;
    }
    default: break;
    }
    path.closeSubpath();
    return path;
}

QPainterPath
Mask::Tab::simplePath(QRect r, const int shape)
{
    static const int round = 8;
    const int x = r.x(), y = r.y(), w = r.width(), h = r.height(), x2 = x+w, y2 = y+h;
    QPainterPath path;
    switch (shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
    {
        path.moveTo(x, y2);
        path.arcTo(x, y2-round, round, round, 270, 90);
        path.arcTo(x+round, y, round, round, 180, -90);
        path.arcTo(x2-round*2, y, round, round, 90, -90);
        path.arcTo(x2-round, y2-round, round, round, 180, 90);
        break;
    }
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
    {
        path.moveTo(x, y);
        path.arcTo(x, y, round, round, 90, -90);
        path.arcTo(x+round, y2-round, round, round, 180, 90);
        path.arcTo(x2-round*2, y2-round, round, round, 270, 90);
        path.arcTo(x2-round, y, round, round, 180, -90);
        break;
    }
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
    {
        path.moveTo(x2, y);
        path.arcTo(x2-round, y, round, round, 0, -90);
        path.arcTo(x, y+round, round, round, 90, 90);
        path.arcTo(x, y2-round*2, round, round, 180, 90);
        path.arcTo(x2-round, y2-round, round, round, 90, -90);
        break;
    }
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
    {
        path.moveTo(x, y);
        path.arcTo(x, y, round, round, 180, 90);
        path.arcTo(x2-round, y+round, round, round, 90, -90);
        path.arcTo(x2-round, y2-round*2, round, round, 0, -90);
        path.arcTo(x, y2-round, round, round, 90, 90);
        break;
    }
    default: break;
    }
    path.closeSubpath();
    return path;
}

QPainterPath
Mask::Tab::tabPath(const TabStyle s, QRect r, const int shape)
{
    switch (s)
    {
    case Chrome:
        return chromePath(r, shape);
    case Simple:
        return simplePath(r, shape);
    default: return QPainterPath();
    }
}

int
Mask::Tab::overlap(const TabStyle s)
{
    switch (s)
    {
    case Chrome:
        return 18;
    case Simple:
        return 22; // 2*roundness + 4*2 - 2
    default: return 0;
    }
}

QRect
Mask::Tab::maskAdjusted(const QRect &r, const int shape)
{
    switch (shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth: return r.adjusted(2, 4, -2, -TabBarBottomSize); break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest: return r.adjusted(4, 2, -TabBarBottomSize, -2); break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast: return r.adjusted(TabBarBottomSize, 2, -4, -2); break;
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth: return r.adjusted(2, TabBarBottomSize, -2, -4); break;
    default: return r;
    }
}

void
Mask::Tab::drawTab(const TabStyle s, const TabPos pos, QRect r, QPainter *p, const QBrush &b, const QStyleOptionTabV3 *opt, const quint8 hover)
{
    if (!opt)
        return;

    static const int padding = overlap(s)/2;
    const bool vert = isVertical(opt->shape);

    r.adjust(-padding*!vert, -padding*vert, padding*!vert, padding*vert);
    drawMask(p, r, s, pos, b, opt->shape);
    const QColor shadow = Color::mid(Qt::black, opt->palette.color(QPalette::Highlight), Steps-hover, hover);
    drawShadow(p, r, s, pos, shadow, opt->shape);
    if (pos == Selected)
    {
        switch (opt->shape)
        {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth: r.translate(0, 1); break;
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth: r.translate(0, -1);  break;
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest: r.translate(1, 0); break;
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast: r.translate(-1, 0);  break;
        default: break;
        }
        p->fillPath(tabPath(s, maskAdjusted(r, opt->shape), opt->shape), b);
    }
}

QSize
Mask::Tab::maskSize(const TabStyle s, const int shape)
{
    const quint8 base(128/*overlap(s)*4+1*/);
    if (isVertical(shape))
        return QSize(dConf.baseSize + TabBarBottomSize, base);
    return QSize(base, dConf.baseSize + TabBarBottomSize);
}

QPoint
Mask::Tab::eraseOffset(const QSize &sz, const TabPos pos, const int shape, const int overlap)
{
    int offX(0);
    int offY(0);
    switch (shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth: pos == BeforeSelected ? offX = sz.width()-overlap : offX = -(sz.width()-overlap); break;
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth: pos == AfterSelected ? offX = sz.width()-overlap : offX = -(sz.width()-overlap); break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest: pos == AfterSelected ? offY = sz.height()-overlap : offY = -(sz.height()-overlap); break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast: pos == BeforeSelected ? offY = sz.height()-overlap : offY = -(sz.height()-overlap);  break;
    default: break;
    }
    return QPoint(offX, offY);
}

void
Mask::Tab::eraseSides(const QRect &r, QPainter *p, const QPainterPath &path, const TabPos pos, const int shape, const int overlap)
{
    if (pos == Selected)
        return;

    const QPainter::CompositionMode mode = p->compositionMode();
    p->setCompositionMode(QPainter::CompositionMode_DestinationOut);
    const bool hadAA = p->testRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::Antialiasing);
    QImage img(r.size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter tmpp(&img);
    tmpp.setRenderHint(QPainter::Antialiasing);
    tmpp.setPen(QPen(Qt::black, 2.0f));
    tmpp.setBrush(Qt::black);
    tmpp.drawPath(path);
    tmpp.end();
    p->drawImage(eraseOffset(r.size(), pos, shape, overlap), img);
    FX::expblur(img, 2);
    p->drawImage(eraseOffset(r.size(), pos, shape, overlap), img);
    p->setRenderHint(QPainter::Antialiasing, hadAA);
    p->setCompositionMode(mode);
}

QPixmap
*Mask::Tab::tabMask(const TabStyle s, const TabPos pos, const int shape, const bool outline)
{
    static QMap<quint32, QPixmap *> map;
    const quint32 key = ((quint32)(s+1)<<24) | ((quint32)(pos+1)<<16) | ((quint32)(shape+1)<<8) | ((quint32)(outline+1));
    if (map.contains(key))
        return map.value(key);

    const QRect rect(QPoint(0, 0), maskSize(s, shape));
    const QPainterPath path = tabPath(s, maskAdjusted(rect, shape), shape);
    QImage mask(rect.size(), QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);
    QPainter p(&mask);
    p.setRenderHint(QPainter::Antialiasing);
    if (outline)
    {
        p.setPen(QPen(Qt::black, 2.0f));
        p.setBrush(Qt::black);
        p.drawPath(path);
    }
    else
        p.fillPath(path, Qt::black);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    eraseSides(rect, &p, path, pos, shape, overlap(s));
    p.fillRect(tabBarRect(mask.rect(), pos, shape), Qt::black);
    p.end();

    QPixmap *ret = new QPixmap[3]();
    split(ret, mask, overlap(s), shape);
    map.insert(key, ret);
    return ret;
}

QPixmap
*Mask::Tab::tabShadow(const TabStyle s, const TabPos pos, const QColor &c, const int shape)
{
    static QMap<quint64, QPixmap *> map;
    const quint64 key = ((quint64)c.rgba()<<32) | ((quint64)(s+1)<<24) | ((quint64)(pos+1)<<16) | ((quint64)(shape+1)<<8);
    if (map.contains(key))
        return map.value(key);

    QImage img(maskSize(s, shape), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    drawMask(&p, img.rect(), s, pos, c, shape);
    p.end();

    FX::expblur(img, 2);

    p.begin(&img);
    drawMask(&p, img.rect(), s, pos, c, shape, true);

//    eraseSides(img.rect(), &p, tabPath(s, img.rect(), shape), pos, shape);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    drawMask(&p, img.rect(), s, pos, Qt::black, shape);
    p.fillRect(tabBarRect(img.rect(), BeforeSelected, shape), Qt::black);
    p.fillRect(img.rect(), QColor(0, 0, 0, 255-dConf.shadows.opacity));

    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    drawMask(&p, img.rect(), s, pos, Qt::white, shape);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    QRect r(img.rect());
    switch (shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth: r.translate(0, 1); break;
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth: r.translate(0, -1);  break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest: r.translate(1, 0); break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast: r.translate(-1, 0);  break;
    default: break;
    }

    drawMask(&p, r, s, pos, Qt::white, shape);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(tabPath(s, maskAdjusted(img.rect(), shape), shape), QColor(0, 0, 0, 255-dConf.shadows.illumination));
    p.end();

//    if (pos != Selected)
//    {
//        p.begin(&img);
//        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
//        drawMask(&p, img.rect().translated(eraseOffset(img.size(), pos, shape)), s, pos, Qt::black, shape, true);
//        p.end();
//    }

    QPixmap *ret = new QPixmap[3]();
    split(ret, img, overlap(s), shape);
    map.insert(key, ret);
    return ret;
}

void
Mask::Tab::split(QPixmap *p, const QImage &img, const int sideSize, const int shape)
{
    switch (shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        p[0] = QPixmap::fromImage(img.copy(0, 0, sideSize, img.height()));
        p[1] = QPixmap::fromImage(img.copy(sideSize+1, 0, 1, img.height()));
        p[2] = QPixmap::fromImage(img.copy(img.width()-sideSize, 0, sideSize, img.height()));
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        p[0] = QPixmap::fromImage(img.copy(0, img.height()-sideSize, img.width(), sideSize));
        p[1] = QPixmap::fromImage(img.copy(0, sideSize+1, img.width(), 1));
        p[2] = QPixmap::fromImage(img.copy(0, 0, img.width(), sideSize));
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        p[0] = QPixmap::fromImage(img.copy(0, 0, img.width(), sideSize));
        p[1] = QPixmap::fromImage(img.copy(0, sideSize+1, img.width(), 1));
        p[2] = QPixmap::fromImage(img.copy(0, img.height()-sideSize, img.width(), sideSize));
        break;
    default: break;
    }
}

void
Mask::Tab::drawMask(QPainter *p, const QRect &r, const TabStyle s, const TabPos pos, const QBrush &b, const int shape, const bool outline)
{
    QPixmap *mask = tabMask(s, pos, shape, outline);
    for (int i = 0; i < 3; ++i)
        mask[i] = colorized(mask[i], b);
    drawTiles(p, r, mask, shape);
}

void
Mask::Tab::drawShadow(QPainter *p, const QRect &r, const TabStyle s, const TabPos pos, const QColor &c, const int shape)
{
    drawTiles(p, r, tabShadow(s, pos, c, shape), shape);
}

void
Mask::Tab::drawTiles(QPainter *p, const QRect &r, const QPixmap *tiles, const int shape)
{
    const int x = r.x();
    const int y = r.y();
    const int w = r.width();
    const int h = r.height();
    switch (shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        p->drawPixmap(QRect(r.topLeft(), tiles[0].size()), tiles[0]);
        p->drawTiledPixmap(QRect(r.topLeft()+QPoint(tiles[0].width(), 0), r.size()-QSize(tiles[0].width()*2, 0)), tiles[1]);
        p->drawPixmap(QRect(r.topRight()-QPoint(tiles[2].width()-1, 0), tiles[2].size()), tiles[2]);
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        p->drawPixmap(QRect(QPoint(x, (y+h) - tiles[0].height()), tiles[0].size()), tiles[0]);
        p->drawTiledPixmap(QRect(QPoint(x, y+tiles[2].height()), QSize(w, h-tiles[2].height()*2)), tiles[1]);
        p->drawPixmap(QRect(QPoint(x, y), tiles[2].size()), tiles[2]);
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        p->drawPixmap(QRect(QPoint(x, (y+h) - tiles[2].height()), tiles[2].size()), tiles[2]);
        p->drawTiledPixmap(QRect(QPoint(x, y+tiles[0].height()), QSize(w, h-tiles[0].height()*2)), tiles[1]);
        p->drawPixmap(QRect(QPoint(x, y), tiles[0].size()), tiles[0]);
        break;
    default: break;
    }
}

QRect
Mask::Tab::tabBarRect(const QRect &r, const TabPos pos, const int shape)
{
    const int x = r.x();
    const int y = r.y();
    const int w = r.width();
    const int h = r.height();
    int padding = TabBarBottomSize;
    if (pos != Selected)
        ++padding;

    switch (shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
        return QRect(x, (y+h)-padding, w, padding);
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        return QRect(x, y, w, padding);
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        return QRect((x+w)-padding, y, padding, h);
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        return QRect(x, y, padding, h);
    default: return QRect();
    }
}

QRect
Mask::Tab::tabRect(const QRect &r, const TabPos pos, const int shape)
{
    const int x = r.x();
    const int y = r.y();
    const int w = r.width();
    const int h = r.height();
    int padding = TabBarBottomSize;
//    if (pos != Selected)
//        ++padding;

    int topPadding = (2*pos==Selected);
    switch (shape)
    {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
        return QRect(x, y+topPadding, w, h-(padding+topPadding));
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        return QRect(x, y+padding, w, h-(padding+topPadding));
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        return QRect(x+topPadding, y, w-(padding+topPadding), h);
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        return QRect(x+padding, y, w-(padding+topPadding), h);
    default: return QRect();
    }
}

bool
Mask::Tab::isVertical(const int shape)
{
    switch (shape)
    {
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        return true;
    default: return false;
    }
}

QPixmap
Mask::Tab::colorized(const QPixmap &pix, const QBrush &b)
{
    QPixmap ret(pix.size());
    ret.fill(Qt::transparent);
    QPainter p(&ret);
    p.fillRect(ret.rect(), b);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawPixmap(0, 0, pix);
    p.end();
    return ret;
}
