#include <qtextstream.h>
#include <QDebug>
#include <QPixmap>
#include <QX11Info>
#include <QPainter>
#include <QEvent>
#include <QImage>
#include <QMenu>
#include <QToolButton>
#include <QApplication>

#include "shadowhandler.h"
#include "xhandler.h"
#include "ops.h"
#include "render.h"
#include "../config/settings.h"
#include "macros.h"

Q_DECL_EXPORT ShadowHandler *ShadowHandler::m_instance(0);

ShadowHandler
*ShadowHandler::instance()
{
    if (!m_instance)
        m_instance = new ShadowHandler();
    return m_instance;
}

bool
ShadowHandler::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;
    if (e->type() == QEvent::Show)
    {
        QWidget *w = static_cast<QWidget *>(o);
        if (w->isWindow()
                && w->testAttribute(Qt::WA_WState_Created)
                && w->internalWinId())
        {
            if (QMenu *m = qobject_cast<QMenu *>(w))
                ShadowHandler::installShadows(m);
            else
                ShadowHandler::installShadows(w->winId());
        }
    }
    return false;
}

static XHandler::XPixmap pix[2][8] = {{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }};
static XHandler::XPixmap menupix[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

enum Pos { Top = 0, TopRight, Right, BottomRight, Bottom, BottomLeft, Left, TopLeft };

static QRect part(int part, int size, int d = 1)
{
    switch (part)
    {
    case Top:           return QRect(size,      0,      1,      size);
    case TopRight:      return QRect(size+d,    0,      size,   size);
    case Right:         return QRect(size+d,    size,   size,   1);
    case BottomRight:   return QRect(size+d,    size+d, size,   size);
    case Bottom:        return QRect(size,      size+d, 1,      size);
    case BottomLeft:    return QRect(0,         size+d, size,   size);
    case Left:          return QRect(0,         size,   size,   1);
    case TopLeft:       return QRect(0,         0,      size,   size);
    default: return QRect();
    }
}


XHandler::XPixmap
*ShadowHandler::shadows(bool active)
{
    static XHandler::Value atom[2] = { XHandler::StoreInActiveShadow, XHandler::StoreActiveShadow };
    XHandler::XPixmap *shadows = XHandler::getXProperty<XHandler::XPixmap>(QX11Info::appRootWindow(), atom[active]);
    if (!shadows)
    {
        int size(dConf.deco.shadowSize);
        if (!active)
            size/=2;

        XHandler::XPixmap data[12];

        int s(size*2+1);
        QImage img(s, s, QImage::Format_ARGB32);
        img.fill(Qt::transparent);

        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::NoBrush);
        QRadialGradient rg(QRectF(img.rect()).center(), size);
        rg.setColorAt(0.0f, QColor(0, 0, 0, 160));
//        rg.setColorAt(0.3f, QColor(0, 0, 0, 85));
        rg.setColorAt(0.8f, QColor(0, 0, 0, 15));
        rg.setColorAt(1.0f, Qt::transparent);
        p.fillRect(img.rect(), rg);
        const quint32 sd[4] = { size*0.5f, size*0.7f, size*0.9f, size*0.7f };
        QRect r(0, 0, s, s);
        r.adjust(sd[3], sd[0], -sd[1], -sd[2]);
        p.setBrush(QColor(0, 0, 0, 63));
        p.drawRoundedRect(r.adjusted(-1, -1, 1, 1), 5, 5);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setBrush(Qt::black);
        p.drawRoundedRect(r, 5, 5);
        p.end();

        for (int i = 0; i < 12; ++i)
        {
            if (i < 8)
                data[i] = pix[active][i] = XHandler::x11Pixmap(img.copy(part(i, size)));
            else
                data[i] = sd[i-8];
        }
        XHandler::setXProperty<XHandler::XPixmap>(QX11Info::appRootWindow(), atom[active], XHandler::Long, data, 12);
    }
    return XHandler::getXProperty<XHandler::XPixmap>(QX11Info::appRootWindow(), atom[active]);
}

static QRect menupart(int part, int size, int hor = 128, int ver = 1, int d = 4)
{
    switch (part)
    {
    case 0: return QRect(size+d, 0, hor-d*2, size);
    case 1: return QRect(size+hor, 0, size, size);
    case 2: return QRect(size+hor, size, size, ver);
    case 3: return QRect(size+hor, size+ver, size, size);
    case 4: return QRect(size+d, size+ver, hor-d*2, size);
    case 5: return QRect(0, size+ver, size, size);
    case 6: return QRect(0, size, size, ver);
    case 7: return QRect(0, 0, size, size);
    default: return QRect();
    }
}

XHandler::XPixmap
*ShadowHandler::menuShadow(bool up, QMenu *menu, QToolButton *tb)
{
    static XHandler::XPixmap data[12];
    int size(32);
    QPixmap mask(menu->size()+QSize(size*2, size*2));
    mask.fill(Qt::transparent);

    QPainter p(&mask);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::NoBrush);

    int m(size/2);
    int rnd(4);
    int adj(size-rnd);
    const int sd[4] = { adj, adj, adj, adj };
    QRect rt(mask.rect().adjusted(adj, adj, -adj, -adj));
    p.setBrush(menu->palette().color(menu->backgroundRole()));
    p.drawRoundedRect(rt, rnd, rnd);
    rt = mask.rect();


    QRect arrow(up?QRect(0, m, m, m-rnd):QRect(0, mask.height()-adj, m, m-rnd));
    int center(size+(tb->width()/2-arrow.width()/2));
    arrow.moveLeft(center);

    Ops::drawArrow(&p, p.brush().color(), arrow, up?Ops::Up:Ops::Down, 9000);
    p.end();

    QImage img(mask.size(), QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    p.begin(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);
    const QPixmap &sh(QPixmap::fromImage(Render::blurred(mask.toImage(), mask.rect(), 8)));
    p.drawTiledPixmap(img.rect(), Render::colorized(sh, Qt::black));
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.drawTiledPixmap(img.rect(), mask);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawTiledPixmap(img.rect(), mask);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.drawRoundedRect(img.rect().adjusted(sd[3], sd[0], -sd[1], -sd[2]), rnd, rnd);
    p.end();

    for (int i = 0; i < 12; ++i)
    {
        if (i < 8)
        {
            if (menupix[i])
            {
                XHandler::freePix(menupix[i]);
                menupix[i] = 0;
            }
            QRect r(menupart(i, 32, menu->width(), menu->height()));
            data[i] = XHandler::x11Pixmap(img.copy(r));
        }
        else
            data[i] = sd[i-8];
    }
    return data;
}

void
ShadowHandler::installShadows(WId w, bool active)
{
    if (w != QX11Info::appRootWindow())
        XHandler::setXProperty<XHandler::XPixmap>(w, XHandler::_KDE_NET_WM_SHADOW, XHandler::Long, shadows(active), 12);
}

void
ShadowHandler::installShadows(QMenu *m)
{
    if (!qobject_cast<QMenu *>(m))
        return;

    QToolButton *tb(qobject_cast<QToolButton *>(m->parentWidget()));
    if (!tb)
    {
        QWidget *w(qApp->activeWindow()); 
        if (!w || !(tb = qobject_cast<QToolButton *>(w->childAt(w->mapFromGlobal(QCursor::pos())))))
        {
            installShadows(m->winId());
            return;
        }
    }
    m->setProperty("DSP_hasmenuarrow", true);

    QPoint tbPoint(tb->mapToGlobal(tb->rect().topLeft()));
    QPoint muPoint(m->mapToGlobal(m->rect().topLeft()));
    const bool up(tbPoint.y()<muPoint.y());
    if (m->winId() != QX11Info::appRootWindow() && tbPoint.x() == muPoint.x())
    {
        XHandler::XPixmap *shadows = menuShadow(up, m, tb);
        XHandler::setXProperty<XHandler::XPixmap>(m->winId(), XHandler::_KDE_NET_WM_SHADOW, XHandler::Long, shadows, 12);
    }
    else
        installShadows(m->winId());
}

void
ShadowHandler::removeShadows(WId w)
{
    if (w != QX11Info::appRootWindow())
        XHandler::deleteXProperty(w, XHandler::_KDE_NET_WM_SHADOW);
}

void
ShadowHandler::manage(QWidget *w)
{
    w->removeEventFilter(instance());
    w->installEventFilter(instance());
}

void
ShadowHandler::release(QWidget *w)
{
    w->removeEventFilter(instance());
    XHandler::deleteXProperty(w->winId(), XHandler::_KDE_NET_WM_SHADOW);
}

void
ShadowHandler::removeDelete()
{
    for (int a = 0; a < 2; ++a)
    for (int i = 0; i < 8; ++i)
    if (pix[a][i])
    {
        XHandler::freePix(pix[a][i]);
        pix[a][i] = 0;
    }
    XHandler::deleteXProperty(QX11Info::appRootWindow(), XHandler::StoreActiveShadow);
    XHandler::deleteXProperty(QX11Info::appRootWindow(), XHandler::StoreInActiveShadow);
}
