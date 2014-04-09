#include <QDebug>
#include <qmath.h>

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
    initMaskParts();
    initShadowParts();
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
    for (int s = 0; s < ShadowCount; ++s)
    for (int i = 0; i < MAXRND+1; ++i)
    {
        const int size = i*2+1;
        QPixmap pix(size, size);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        float add(i?(float)i/100.0f:0);
        switch (s)
        {
        case Sunken:
        {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 255, 255, 255));
            QRect rect(pix.rect());
            p.drawRoundedRect(rect, i, i);
            p.setBrush(Qt::black);
            rect.setBottom(rect.bottom()-1);
            p.drawRoundedRect(rect, i, i);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            QRadialGradient rg(pix.rect().center()+QPointF(0, qMin(0.25f, add)), qFloor(size/2));
            rg.setColorAt(qMin(0.8f, 0.6f+add), Qt::black);
            rg.setColorAt(1.0f, Qt::transparent);
            p.translate(0.5f, 0.5f); //...and this is needed.... whyyy?
            p.fillRect(rect, rg);
            break;
        }
        case Raised:
        {
            QRadialGradient rg(pix.rect().center()+QPointF(0, add), qFloor(size/2));
            rg.setColorAt(0.7f, Qt::black);
            rg.setColorAt(1.0f, Qt::transparent);
            p.translate(0.5f, 0.5f); //...and this is needed.... whyyy?
            p.fillRect(pix.rect(), rg);
            break;
        }
        case Etched:
            p.setBrush(Qt::NoBrush);
            p.translate(0.5f, 0.5f);
            p.setPen(QColor(255, 255, 255, 96));
            p.drawRoundedRect(pix.rect().adjusted(0, 1, -1, -1), i, i);
            p.setPen(Qt::black);
            p.drawRoundedRect(pix.rect().adjusted(0, 0, -1, -2), i, i);
            break;
        default: break;
        }
        p.end();
        splitShadowParts((Shadow)s, i, size, pix);
    }
}

void
Render::splitShadowParts(const Shadow shadow, const int roundNess, const int size, const QPixmap &source)
{
    m_shadow[shadow][roundNess][TopLeftPart] = source.copy(0, 0, roundNess, roundNess);
    m_shadow[shadow][roundNess][TopMidPart] = source.copy(roundNess, 0, size-roundNess*2, roundNess);
    m_shadow[shadow][roundNess][TopRightPart] = source.copy(size-roundNess, 0, roundNess, roundNess);
    m_shadow[shadow][roundNess][LeftPart] = source.copy(0, roundNess, roundNess, size-roundNess*2);
    m_shadow[shadow][roundNess][CenterPart] = source.copy(roundNess, roundNess, size-roundNess*2, size-roundNess*2);
    m_shadow[shadow][roundNess][RightPart] = source.copy(size-roundNess, roundNess, roundNess, size-roundNess*2);
    m_shadow[shadow][roundNess][BottomLeftPart] = source.copy(0, size-roundNess, roundNess, roundNess);
    m_shadow[shadow][roundNess][BottomMidPart] = source.copy(roundNess, size-roundNess, size-roundNess*2, roundNess);
    m_shadow[shadow][roundNess][BottomRightPart] = source.copy(size-roundNess, size-roundNess, roundNess, roundNess);
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
Render::genPart(const Part &part, const QPixmap &source, const int roundNess) const
{
    QPixmap rt = source.copy(rect(source.rect(), part, roundNess));
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
    roundNess = qMin(qMin(MAXRND, roundNess), qMin(rect.height(), rect.width())/2-1);
    QPixmap pix(rect.size());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.fillRect(pix.rect(), brush);
    p.end();

    for (int i = 0; i < PartCount; ++i)
        if (needPart((Part)i, sides))
        {
            const QPixmap &partPix = genPart((Part)i, pix, roundNess);
            painter->drawTiledPixmap(partRect(QRect(QPoint(0, 0), rect.size()), (Part)i, roundNess, sides).translated(rect.x(), rect.y()), partPix);
        }
}

void
Render::_renderShadow(const Shadow &shadow, const QRect &rect, QPainter *painter, int roundNess, const Sides &sides)
{
    roundNess = qMin(qMin(MAXRND, roundNess), rect.height()/2-1);
    for (int i = 0; i < PartCount; ++i)
        if (needPart((Part)i, sides))
            painter->drawTiledPixmap(partRect(QRect(QPoint(0, 0), rect.size()), (Part)i, roundNess, sides).translated(rect.x(), rect.y()), m_shadow[shadow][roundNess][i]);
}
