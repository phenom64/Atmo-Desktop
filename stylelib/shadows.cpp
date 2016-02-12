#include "shadows.h"
#include "fx.h"
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QtGlobal>
#include <QDebug>
#include "masks.h"
#include "macros.h"

using namespace DSP;

Shadow::Shadow(const ShadowStyle t, const quint8 r, const quint8 o, const quint8 i)
    : m_type(t)
    , m_opacity(o)
    , m_illumination(i)
    , m_round(r)
    , m_pix(0)
{
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
        Mask::render(rect, Qt::black, &pt, m_round);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Mask::render(rect.adjusted(1, 1, -1, -1), Qt::black, &pt, rnd);
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
        QRect rect(pix.rect().adjusted(1, 1, -1, 0));
        QPainter pt(&white);
        Mask::render(rect, Qt::white, &pt, m_round);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Mask::render(rect.adjusted(1, 1, -1, -1), Qt::black, &pt, m_round-1);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.fillRect(rect, QColor(0, 0, 0, 255-m_illumination));
        pt.end();

        rect.setBottom(rect.bottom()-1);

        QPixmap black(size, size);
        black.fill(Qt::transparent);
        pt.begin(&black);
        pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
        Mask::render(rect, Qt::black, &pt, m_round-0.5f);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);

        QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
        lg.setColorAt(0, Qt::transparent);
        lg.setColorAt(1, QColor(0, 0, 0, 63));
        pt.fillRect(rect, lg);

        Mask::render(rect.adjusted(1, 1, -1, -1), Qt::black, &pt, m_round+0.5f/*-1*/);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.fillRect(rect, QColor(0, 0, 0, 255-m_opacity));
        pt.end();

        p.drawPixmap(pix.rect(), white);
        p.drawPixmap(pix.rect(), black);
        break;
    }
    case Raised:
    {
        QRectF rect(QRectF(pix.rect())); //ummmmm, need to figure this out at some point...
        QImage blurred(pix.size(), QImage::Format_ARGB32_Premultiplied);
        blurred.fill(Qt::transparent);
        QPainter pt(&blurred);
        Mask::render(rect.adjusted(0.75f, 1.0f, -0.75f, -0.5f), QColor(0, 0, 0, 185), &pt, m_round+2);
        pt.end();
        FX::expblur(blurred, 1);

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
        pt.fillRect(dark.rect(), QColor(0, 0, 0, 255-m_opacity));
        pt.end();

        QImage light(pix.size(), QImage::Format_ARGB32_Premultiplied);
        light.fill(Qt::transparent);
        pt.begin(&light);
        Mask::render(rect, Qt::white, &pt, m_round);

        QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
        lg.setColorAt(0, Qt::transparent);
        lg.setColorAt(1, QColor(0, 0, 0, 127));
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        Mask::render(rect, lg, &pt, m_round);
        rect.shrink(1);
        Mask::render(rect, Qt::black, &pt, m_round-1);
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

    split(pix, size, cornerSize);
    m_pix[CenterPart] = QPixmap();
}

void
Shadow::render(const QRect &r, QPainter *p, const Sides s)
{
    if (!m_pix)
        genShadow();
    const qint16 x(r.x()), y(r.y()), w(r.width()), h(r.height()), x2(x+w), y2(y+h), block(m_pix[TopLeftPart].width());

    //corners
    if ((s & (Top|Left)) == (Top|Left))
        p->drawPixmap(QRect(x, y, block, block), m_pix[TopLeftPart]);
    if ((s & (Top|Right)) == (Top|Right))
        p->drawPixmap(QRect(x2-block, y, block, block), m_pix[TopRightPart]);
    if ((s & (Bottom|Left)) == (Bottom|Left))
        p->drawPixmap(QRect(x, y2-block, block, block), m_pix[BottomLeftPart]);
    if ((s & (Bottom|Right)) == (Bottom|Right))
        p->drawPixmap(QRect(x2-block, y2-block, block, block), m_pix[BottomRightPart]);

    qint16 cx(x), cy(y), cw(w), ch(h);

    //sides
    if (s & Top)
    {
        QRect rect(x+bool(s&Left)*block, y, w-((bool(s&Left)*block)+(bool(s&Right)*block)), block);
        p->drawTiledPixmap(rect, m_pix[TopMidPart]);
        cy += block;
        ch -= block;
    }
    if (s & Right)
    {
        QRect rect(x2-block, y+bool(s&Top)*block, block, h-((bool(s&Top)*block)+(bool(s&Bottom)*block)));
        p->drawTiledPixmap(rect, m_pix[RightPart]);
        cw -= block;
    }
    if (s & Bottom)
    {
        QRect rect(x+bool(s&Left)*block, y2-block, w-((bool(s&Left)*block)+(bool(s&Right)*block)), block);
        p->drawTiledPixmap(rect, m_pix[BottomMidPart]);
        ch -= block;
    }
    if (s & Left)
    {
        QRect rect(x, y+bool(s&Top)*block, block, h-((bool(s&Top)*block)+(bool(s&Bottom)*block)));
        p->drawTiledPixmap(rect, m_pix[LeftPart]);
        cx += block;
        cw -= block;
    }

    //center
    if (!m_pix[CenterPart].isNull())
        p->drawTiledPixmap(QRect(cx, cy, cw, ch), m_pix[CenterPart]);
}

void
Shadow::split(const QPixmap &pix, const quint8 size, const quint8 cornerSize)
{
    m_pix = new QPixmap[PartCount]();
    m_pix[TopLeftPart] = pix.copy(0, 0, cornerSize, cornerSize);
    m_pix[TopMidPart] = pix.copy(cornerSize, 0, size-cornerSize*2, cornerSize);
    m_pix[TopRightPart] = pix.copy(size-cornerSize, 0, cornerSize, cornerSize);
    m_pix[LeftPart] = pix.copy(0, cornerSize, cornerSize, size-cornerSize*2);
    m_pix[CenterPart] = pix.copy(cornerSize, cornerSize, size-cornerSize*2, size-cornerSize*2);
    m_pix[RightPart] = pix.copy(size-cornerSize, cornerSize, cornerSize, size-cornerSize*2);
    m_pix[BottomLeftPart] = pix.copy(0, size-cornerSize, cornerSize, cornerSize);
    m_pix[BottomMidPart] = pix.copy(cornerSize, size-cornerSize, size-cornerSize*2, cornerSize);
    m_pix[BottomRightPart] = pix.copy(size-cornerSize, size-cornerSize, cornerSize, cornerSize);
}

//---------------------------------------------------------------------------------------------------

Focus::Focus(const quint8 r, const QColor &c)
{

}
