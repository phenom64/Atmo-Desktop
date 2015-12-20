#include "shadows.h"
#include "fx.h"
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QtGlobal>

using namespace DSP;

Shadow::Shadow(const ShadowStyle t, const quint8 r, const quint8 o)
    : m_type(t)
    , m_opacity(o)
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
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 255)); //TODO: get luminosity from window bg
        p.setClipRect(rect.adjusted(0, rect.height()/2, 0, 0));
        p.drawRoundedRect(rect, m_round, m_round);
        p.setClipping(false);
        rect.setBottom(rect.bottom()-1);
        p.setBrush(Qt::black);
        p.drawRoundedRect(rect, m_round, m_round);
        QImage img(pix.size(), QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter pt(&img);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(Qt::NoPen);
        pt.setBrush(Qt::black);
        QRectF rt(rect);
        rt.adjust(1.0f, 1.5f, -1.0f, -0.75f);
        int rnd(m_round);
        if (rnd)
            --rnd;
        pt.drawRoundedRect(rt, rnd, rnd);
        pt.end();

        FX::expblur(img, 1);

        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(pix.rect(), img);
        break;
    }
    case Etched:
    {
        QPixmap white(pix.size()-QSize(0, 1));
        white.fill(Qt::transparent);
        QPainter pt(&white);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setBrush(Qt::white);
        pt.setPen(Qt::NoPen);
        pt.drawRoundedRect(white.rect(), m_round, m_round);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.setBrush(Qt::black);
        pt.drawRoundedRect(white.rect().adjusted(1, 1, -1, -1), m_round-1, m_round-1);
        pt.end();

        QPixmap black(pix.size()-QSize(0, 1));
        black.fill(Qt::transparent);
        pt.begin(&black);
        pt.setRenderHint(QPainter::Antialiasing);
        pt.setPen(Qt::NoPen);
        pt.setBrush(Qt::black);
        pt.setCompositionMode(QPainter::CompositionMode_SourceOver);
        pt.drawRoundedRect(black.rect(), m_round, m_round);
        pt.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pt.drawRoundedRect(black.rect().adjusted(1, 1, -1, -1), m_round-1, m_round-1);
        pt.end();

        p.drawTiledPixmap(white.rect().translated(0, 1), white);
        p.drawTiledPixmap(black.rect(), black);
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
        pt.drawRoundedRect(QRectF(img.rect()).adjusted(0.75f, 1.0f, -0.75f, -0.5f), m_round, m_round);
        pt.end();
        FX::expblur(img, 1);
        p.drawImage(QPoint(0, 0), img);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.drawRoundedRect(pix.rect().adjusted(1, 1, -1, -1), m_round, m_round);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.drawRoundedRect(pix.rect().adjusted(2, 2, -2, -2), m_round, m_round);
        break;
    }
    case Yosemite:
    {
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.drawRoundedRect(pix.rect(), m_round, m_round);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.drawRoundedRect(pix.rect().adjusted(1, 1, -1, -1), m_round-1, m_round-1);
        break;
    }
    case Rect:
    {
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        QRect rt(pix.rect());
        const int w(1);
        p.drawRoundedRect(rt, m_round, m_round);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.drawRoundedRect(rt.adjusted(w, w, -w, -w), m_round-w, m_round-w);
        break;
    }
    case ElCapitan:
    {
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        p.drawRoundedRect(pix.rect(), m_round, m_round);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.drawRoundedRect(pix.rect().adjusted(1, 1, -1, -1), m_round-1, m_round-1);
        p.setBrush(QColor(0, 0, 0, 63));
        p.drawRoundedRect(pix.rect().translated(0, -1), m_round, m_round);
        break;
    }
    default: break;
    }
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.fillRect(pix.rect(), QColor(0, 0, 0, 255-m_opacity));
    p.end();

    split(pix, size, cornerSize);
    m_pix[CenterPart] = QPixmap();
}

void
Shadow::render(const QRect &r, QPainter *p, const Sides s)
{
    if (!m_pix)
        genShadow();
    const quint16 x(r.x()), y(r.y()), w(r.width()), h(r.height()), x2(x+w), y2(y+h), block(m_pix[TopLeftPart].width());

    //corners
    if ((s & (Top|Left)) == (Top|Left))
        p->drawPixmap(QRect(x, y, block, block), m_pix[TopLeftPart]);
    if ((s & (Top|Right)) == (Top|Right))
        p->drawPixmap(QRect(x2-block, y, block, block), m_pix[TopRightPart]);
    if ((s & (Bottom|Left)) == (Bottom|Left))
        p->drawPixmap(QRect(x, y2-block, block, block), m_pix[BottomLeftPart]);
    if ((s & (Bottom|Right)) == (Bottom|Right))
        p->drawPixmap(QRect(x2-block, y2-block, block, block), m_pix[BottomRightPart]);

    quint16 cx(x), cy(y), cw(w), ch(h);

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
    {
        QRect rect(cx, cy, cw, ch);
        p->drawTiledPixmap(rect, m_pix[CenterPart]);
    }
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
