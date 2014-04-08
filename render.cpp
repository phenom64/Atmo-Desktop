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
    switch (part)
    {
    case TopLeftPart: return QRect(0, 0, roundNess, roundNess);
    case TopMidPart: return QRect(sides&Left?roundNess:0, 0, midWidth, roundNess);
    case TopRightPart: return QRect(w-(sides&Right?roundNess:0), 0, roundNess, roundNess);
    case LeftPart: return QRect(0, sides&Top?roundNess:0, roundNess, midHeight);
    case CenterPart: return QRect(sides&Left?roundNess:0, sides&Top?roundNess:0, midWidth, midHeight);
    case RightPart: return QRect(w-(sides&Right?roundNess:0), roundNess, roundNess, midHeight);
    case BottomLeftPart: return QRect(0, h-(sides&Bottom?roundNess:0), roundNess, roundNess);
    case BottomMidPart: return QRect(sides&Left?roundNess:0, h-(sides&Bottom?roundNess:0), midWidth, roundNess);
    case BottomRightPart: return QRect(w-(sides&Left?roundNess:0), h-(sides&Bottom?roundNess:0), roundNess, roundNess);
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
    inSource = partRect(source.rect(), part, roundNess, sides);
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
//    switch (part)
//    {
//    case TopLeftPart: return sides&Top||sides&Left;
//    case TopMidPart: return sides&Top;
//    case TopRightPart: return sides&Top||sides&Right;
//    case LeftPart: return sides&Left;
//    case CenterPart: return true;
//    case RightPart: return sides&Right;
//    case BottomLeftPart: return sides&Left||sides&Bottom;
//    case BottomMidPart: return sides&Bottom;
//    case BottomRightPart: return sides&Right||sides&Bottom;
//    default: return false;
//    }
    if (part < LeftPart)
        return sides & Top;
    if (part == LeftPart)
        return sides & Left;
    if (part == RightPart)
        return sides & Right;
    if (part > RightPart)
        return sides & Bottom;
    return true; //center
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
            painter->drawTiledPixmap(r, partPix);
        }
    }
}
