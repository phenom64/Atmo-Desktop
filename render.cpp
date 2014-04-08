#include <QDebug>

#include "render.h"

Render *Render::m_instance = 0;

Render
*Render::instance()
{
    if (!m_instance)
        m_instance = new Render();
    return m_instance;
}

Render::Render()
{
}

void
Render::_generateData()
{
    for (int i = 0; i < MAXRND; ++i)
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

QRect
Render::partRect(const QRect &rect, const Part &part, const int roundNess, const Sides &sides) const
{
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

QRect
Render::rect(const QRect &rect, const Part &part, const int roundNess) const
{
    int x, y, w, h;
    rect.getRect(&x, &y, &w, &h);
    int midWidth = w-(roundNess*2);
    int midHeight = h-(roundNess*2);
    switch (part)
    {
    case TopLeftPart: return QRect(0, 0, roundNess, roundNess);
    case TopMidPart: return QRect(roundNess, 0, midWidth, roundNess);
    case TopRightPart: return QRect(w-roundNess, 0, roundNess, roundNess);
    case LeftPart: return QRect(0, roundNess, roundNess, midHeight);
    case CenterPart: return QRect(roundNess, roundNess, midWidth, midHeight);
    case RightPart: return QRect(w-roundNess, roundNess, roundNess, midHeight);
    case BottomLeftPart: return QRect(0, h-roundNess, roundNess, roundNess);
    case BottomMidPart: return QRect(roundNess, h-roundNess, midWidth, roundNess);
    case BottomRightPart: return QRect(w-roundNess, h-roundNess, roundNess, roundNess);
    default: return QRect();
    }
}

bool
Render::isCornerPart(const Part &part) const
{
    return part==TopLeftPart||part==TopRightPart||part==BottomLeftPart||part==BottomRightPart;
}

QPixmap
Render::genPart(const Part &part, const QPixmap &source, const Sides &sides, QRect &inSource, const int roundNess) const
{
    inSource = rect(source.rect(), part, roundNess);
    QPixmap rt = source.copy(inSource);
    if (!isCornerPart(part))
        return rt;
    QPainter p(&rt);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.drawTiledPixmap(rt.rect(), m_mask[roundNess][part]);
    p.end();
    return rt;
}

bool
Render::needPart(const Part &part, const Sides &sides) const
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
Render::_renderMask(const QRect &rect, QPainter *painter, const QBrush &brush, int roundNess, const Sides &sides)
{
    roundNess = qMin(qMin(MAXRND, roundNess), rect.height()/2-1);
    QPixmap pix(rect.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.fillRect(rect, brush);
    p.end();

    for (int i = 0; i < PartCount; ++i)
    {
        if (needPart((Part)i, sides))
        {
            QRect r;
            const QPixmap &partPix = genPart((Part)i, pix, sides, r, roundNess);
            painter->drawTiledPixmap(partRect(rect, (Part)i, roundNess, sides), partPix);
        }
    }
}
