#include "shadows.h"
#include "fx.h"
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QtGlobal>
#include <QDebug>
#include "masks.h"
#include "macros.h"
#include "../config/settings.h"

using namespace DSP;

Shadow::Shadow(const ShadowStyle t, const quint8 r, const quint8 o, const quint8 i)
    : m_type(t)
    , m_opacity(o)
    , m_illumination(i)
    , m_round(r)
    , m_pix(0)
{
}

quint8
Shadow::shadowMargin(const ShadowStyle s)
{
    switch (s)
    {
    case Sunken: return 1;
    case Etched: return 1;
    case Raised: return 2;
    case Carved: return 2;
    case SemiCarved: return 1;
    default: return 0;
    }
}

void
Shadow::genShadow()
{
    const int cornerSize(qMax<int>(4, m_round));
    const int size = cornerSize*2+1;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    switch (m_type)
    {
    case Sunken:
    {
        QRect rect(pix.rect());
        rect.adjust(1, 1, -1, 0);

        //illumination
        QPixmap light(pix.size());
        light.fill(Qt::transparent);
        QPainter lightp(&light);
        lightp.setClipRect(rect.adjusted(0, rect.height()/2, 0, 0));
        Mask::render(rect, QColor(255, 255, 255, m_illumination), &lightp, m_round);
        lightp.setCompositionMode(QPainter::CompositionMode_DestinationOut);

        rect.setBottom(rect.bottom()-1);

        lightp.setClipping(false);
        Mask::render(rect, Qt::black, &lightp, m_round);
        lightp.end();

        //the blur
        QImage blurredImage(pix.size(), QImage::Format_ARGB32_Premultiplied);
        blurredImage.fill(Qt::transparent);
        QPainter pt(&blurredImage);
        QRectF rt(rect);
        float rad(0.75f);
        rt.adjust(1.0f*rad, 1.5f*rad, -1.0f*rad, -0.75f*rad);
        int rnd(m_round);
        if (rnd)
            --rnd;
        Mask::render(rt, Qt::black, &pt, rnd);
        pt.end();
        FX::expblur(blurredImage, 1);

        //the outline
        QImage outlineImage(pix.size(), QImage::Format_ARGB32_Premultiplied);
        outlineImage.fill(Qt::transparent);
        pt.begin(&outlineImage);
//        Mask::render(rect, Qt::black, &pt, qMax<int>(0, m_round-1));
//        pt.end();
//        FX::expblur(outlineImage, 1);
        pt.begin(&outlineImage);
        Mask::render(rect, Qt::black, &pt, qMax<int>(0, m_round));
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Mask::render(rect.shrinked(1), Qt::black, &pt, rnd);
        pt.end();

        //actual shadow
        QImage dark(pix.size(), QImage::Format_ARGB32_Premultiplied);
        dark.fill(Qt::transparent);
        pt.begin(&dark);
        Mask::render(rect, Qt::black, &pt, m_round);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.fillRect(dark.rect(), blurredImage);
        pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
        pt.fillRect(dark.rect(), outlineImage);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);

        QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
        lg.setColorAt(0, Qt::transparent);
        lg.setColorAt(1, QColor(0, 0, 0, 63));
        pt.fillRect(dark.rect(), lg);

        pt.fillRect(dark.rect(), QColor(0, 0, 0, 255-m_opacity));
        pt.end();

        //finally dump it on the resulting pixmap
        p.fillRect(pix.rect(), light);
        p.fillRect(pix.rect(), dark);
        break;
    }
    case Etched:
    {
        QPixmap white(size, size);
        white.fill(Qt::transparent);
        QRect rect(pix.rect().adjusted(1, 2, -1, 0));
        QPainter pt(&white);
        Mask::render(rect, Qt::white, &pt, m_round);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Mask::render(rect.adjusted(1, 1, -1, -1), Qt::black, &pt, m_round-1);
        pt.fillRect(rect, QColor(0, 0, 0, 255-m_illumination));
        pt.end();

        rect.setBottom(rect.bottom()-1);
        rect.setTop(rect.top()-1);

        QPixmap black(size, size);
        black.fill(Qt::transparent);
        pt.begin(&black);
        pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
        Mask::render(rect, Qt::black, &pt, m_round-1);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);

        QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
        lg.setColorAt(0, Qt::transparent);
        lg.setColorAt(1, QColor(0, 0, 0, 63));
        pt.fillRect(rect, lg);

        Mask::render(rect.adjusted(1, 1, -1, -1), Qt::black, &pt, m_round/*-1*/);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.fillRect(rect, QColor(0, 0, 0, 255-m_opacity));
        pt.end();

        p.drawPixmap(pix.rect(), white);
        p.drawPixmap(pix.rect(), black);
        break;
    }
    case Raised:
    {
        QRect rect(pix.rect());
        QImage blurred(pix.size()+QSize(2,2), QImage::Format_ARGB32_Premultiplied);
        blurred.fill(Qt::transparent);
        QPainter pt(&blurred);
        Mask::render(rect.adjusted(1, 1, -1, -1), Qt::black, &pt, m_round+2);
        pt.end();
        FX::expblur(blurred, 1);
        blurred = blurred.copy(rect);

        QImage dark(pix.size(), QImage::Format_ARGB32_Premultiplied);
        dark.fill(Qt::transparent);
        pt.begin(&dark);
        pt.fillRect(dark.rect(), blurred);
        rect.shrink(1);
        Mask::render(rect, Qt::black, &pt, m_round+1);

        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        rect.shrink(1);

        QLinearGradient gradient(dark.rect().topLeft(), dark.rect().bottomLeft());
        gradient.setColorAt(0, QColor(0, 0, 0, 127));
        gradient.setColorAt(0.9, Qt::transparent);
        pt.fillRect(dark.rect(), gradient);

        Mask::render(rect, Qt::black, &pt, m_round);

        if (dConf.shadows.darkRaisedEdges)
        {
            pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
            const QColor edge(QColor(0, 0, 0, m_opacity>>1));
            QLinearGradient leftLg(rect.topLeft(), rect.topLeft()+QPoint(1 + (m_round>>1), 0));
            leftLg.setColorAt(0, edge);
            leftLg.setColorAt(1, Qt::transparent);
            QLinearGradient rightLg(rect.topRight(), rect.topRight()-QPoint(m_round>>1, 0));
            rightLg.setColorAt(0, edge);
            rightLg.setColorAt(1, Qt::transparent);
            Mask::render(rect, leftLg, &pt, m_round+1);
            Mask::render(rect, rightLg, &pt, m_round+1);
        }

        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.fillRect(dark.rect(), QColor(0, 0, 0, 255-m_opacity));
        pt.end();

        QImage light(pix.size(), QImage::Format_ARGB32_Premultiplied);
        light.fill(Qt::transparent);
        pt.begin(&light);
        Mask::render(rect, Qt::white, &pt, m_round);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Mask::render(rect.translated(0, 1), Qt::white, &pt, m_round);
        pt.fillRect(light.rect(), QColor(0, 0, 0, 255-m_illumination));
        pt.end();

        p.fillRect(pix.rect(), dark);
        p.fillRect(pix.rect(), light);
        break;
    }
    case Yosemite:
    {
        Mask::render(pix.rect(), Qt::black, &p, m_round);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Mask::render(pix.rect().adjusted(1, 1, -1, -1), Qt::black, &p, m_round-1);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(pix.rect(), QColor(0, 0, 0, 255-m_opacity));
        break;
    }
    case Rect:
    {
        QRect rt(pix.rect());
        const int w(1);
        Mask::render(rt, Qt::black, &p, m_round);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Mask::render(rt.adjusted(w, w, -w, -w), Qt::black, &p, m_round-w);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(pix.rect(), QColor(0, 0, 0, 255-m_opacity));
        break;
    }
    case ElCapitan:
    {
        Mask::render(pix.rect(), Qt::black, &p, m_round);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Mask::render(pix.rect().adjusted(1, 1, -1, -1), Qt::black, &p, m_round-1);
        Mask::render(pix.rect().translated(0, -1), QColor(0, 0, 0, 63), &p, m_round);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(pix.rect(), QColor(0, 0, 0, 255-m_opacity));
        break;
    }
    default: break;
    }
    p.end();

    m_pix = split(pix, size, cornerSize);
    m_pix[CenterPart] = QPixmap();
}

void
Shadow::render(QPixmap *shadow, const QRect &r, QPainter *p, const Sides s)
{
    if (!shadow)
        return;
    const qint16 x(r.x()), y(r.y()), w(r.width()), h(r.height()), x2(x+w), y2(y+h), block(shadow[TopLeftPart].width());

    //corners
    if ((s & (Top|Left)) == (Top|Left))
        p->drawPixmap(QRect(x, y, block, block), shadow[TopLeftPart]);
    if ((s & (Top|Right)) == (Top|Right))
        p->drawPixmap(QRect(x2-block, y, block, block), shadow[TopRightPart]);
    if ((s & (Bottom|Left)) == (Bottom|Left))
        p->drawPixmap(QRect(x, y2-block, block, block), shadow[BottomLeftPart]);
    if ((s & (Bottom|Right)) == (Bottom|Right))
        p->drawPixmap(QRect(x2-block, y2-block, block, block), shadow[BottomRightPart]);

    qint16 cx(x), cy(y), cw(w), ch(h);

    //sides
    if (s & Top)
    {
        QRect rect(x+bool(s&Left)*block, y, w-((bool(s&Left)*block)+(bool(s&Right)*block)), block);
        p->drawTiledPixmap(rect, shadow[TopMidPart]);
        cy += block;
        ch -= block;
    }
    if (s & Right)
    {
        QRect rect(x2-block, y+bool(s&Top)*block, block, h-((bool(s&Top)*block)+(bool(s&Bottom)*block)));
        p->drawTiledPixmap(rect, shadow[RightPart]);
        cw -= block;
    }
    if (s & Bottom)
    {
        QRect rect(x+bool(s&Left)*block, y2-block, w-((bool(s&Left)*block)+(bool(s&Right)*block)), block);
        p->drawTiledPixmap(rect, shadow[BottomMidPart]);
        ch -= block;
    }
    if (s & Left)
    {
        QRect rect(x, y+bool(s&Top)*block, block, h-((bool(s&Top)*block)+(bool(s&Bottom)*block)));
        p->drawTiledPixmap(rect, shadow[LeftPart]);
        cx += block;
        cw -= block;
    }

    //center
    if (!shadow[CenterPart].isNull())
        p->drawTiledPixmap(QRect(cx, cy, cw, ch), shadow[CenterPart]);
}

void
Shadow::render(const QRect &r, QPainter *p, const Sides s)
{
    if (!m_pix)
        genShadow();
    render(m_pix, r, p, s);
}

QPixmap
*Shadow::split(const QPixmap &pix, const quint8 size, const quint8 cornerSize)
{
    QPixmap *ret = new QPixmap[PartCount]();
    ret[TopLeftPart] = pix.copy(0, 0, cornerSize, cornerSize);
    ret[TopMidPart] = pix.copy(cornerSize, 0, size-cornerSize*2, cornerSize);
    ret[TopRightPart] = pix.copy(size-cornerSize, 0, cornerSize, cornerSize);
    ret[LeftPart] = pix.copy(0, cornerSize, cornerSize, size-cornerSize*2);
    ret[CenterPart] = pix.copy(cornerSize, cornerSize, size-cornerSize*2, size-cornerSize*2);
    ret[RightPart] = pix.copy(size-cornerSize, cornerSize, cornerSize, size-cornerSize*2);
    ret[BottomLeftPart] = pix.copy(0, size-cornerSize, cornerSize, cornerSize);
    ret[BottomMidPart] = pix.copy(cornerSize, size-cornerSize, size-cornerSize*2, cornerSize);
    ret[BottomRightPart] = pix.copy(size-cornerSize, size-cornerSize, cornerSize, cornerSize);
    return ret;
}

//---------------------------------------------------------------------------------------------------

Focus::Focus(const quint8 r, const QColor &c)
{

}

//---------------------------------------------------------------------------------------------------

QPixmap
*Hover::mask(const QColor &h, const quint8 r)
{
    const quint64 key = ((quint64)h.rgba()) | ((quint64)r << 32);
    static QMap<quint64, QPixmap *> map;
    if (map.contains(key))
        return map.value(key);
    static const int bm = 2; //expblur doesnt blur the bottom otherwise....
    const int sz = qMax(9, r*2+1)+bm;
    QImage img(sz, sz, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    static const int m = 1;
    QRect rect = img.rect().adjusted(m, m, -(m+bm), -(m+bm));
    Mask::render(rect, h, &p, r+2);
    p.end();

    FX::expblur(img, 1);
    img = img.copy(0, 0, sz-bm, sz-bm);

    p.begin(&img);
    Mask::render(img.rect().shrinked(1), h, &p, r+1/*+2*/);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    Mask::render(img.rect().shrinked(2), Qt::black, &p, r);
    p.end();

    map.insert(key, split(QPixmap::fromImage(img), img.width(), r));
    return map.value(key);
}

void
Hover::render(const QRect &r, const QColor &c, QPainter *p, const quint8 round, const Sides s, const quint8 level)
{
    if (!level)
        return;

    QPixmap *pix = mask(c, round);
    Shadow::render(pix, r, p, s);
}


