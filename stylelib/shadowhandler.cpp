#include <QPixmap>
#include <QX11Info>
#include <QPainter>
#include <QEvent>
#include <QImage>
#include <QMenu>
#include <QToolButton>
#include <X11/Xlib.h>
#include <fixx11h.h>
#include <QApplication>

#include "shadowhandler.h"
#include "xhandler.h"
#include "ops.h"
#include "render.h"
#include "../config/settings.h"
#include "macros.h"

Q_DECL_EXPORT ShadowHandler ShadowHandler::m_instance;

ShadowHandler
*ShadowHandler::instance()
{
    return &m_instance;
}

bool
ShadowHandler::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;
    if (e->type() == QEvent::Show)
    {
        QWidget *w = static_cast<QWidget *>(o);
        if (!ISBALLOONTIP(w) && w->testAttribute(Qt::WA_WState_Created) || w->internalWinId())
        {
            if (QMenu *m = qobject_cast<QMenu *>(w))
                ShadowHandler::installShadows(m);
            else
                ShadowHandler::installShadows(w->winId());
        }
    }
    return false;
}

static QPixmap *pix[2][8] = {{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }};
//static QPixmap *menupix[2][8] = {{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }};
//static QPixmap *(menupix[2])[8] = {{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }};
static QPixmap *menupix[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

static QRect part(int part, int size, int d = 1)
{
    switch (part)
    {
    case 0: return QRect(size, 0, d, size); //top?
    case 1: return QRect(size+d, 0, size, size);
    case 2: return QRect(size+d, size, size, d);
    case 3: return QRect(size+d, size+d, size, size);
    case 4: return QRect(size, size+d, d, size);
    case 5: return QRect(0, size+d, size, size);
    case 6: return QRect(0, size, size, d);
    case 7: return QRect(0, 0, size, size);
    default: return QRect();
    }
}

unsigned long
*ShadowHandler::shadows(bool active)
{
    static XHandler::Value atom[2] = { XHandler::StoreInActiveShadow, XHandler::StoreActiveShadow };
    unsigned long *shadows = XHandler::getXProperty<unsigned long>(QX11Info::appRootWindow(), atom[active]);
    if (!shadows)
    {
        int size(dConf.deco.shadowSize);
        if (!active)
            size/=2;
        unsigned long *data = new unsigned long[12];

        int s(size*2+1);
        QImage img(s, s, QImage::Format_ARGB32);
        img.fill(Qt::transparent);

        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::NoBrush);
        QRadialGradient rg(QRectF(img.rect()).center(), size);
        rg.setColorAt(0.0f, QColor(0, 0, 0, 160));
        rg.setColorAt(0.3f, QColor(0, 0, 0, 85));
        rg.setColorAt(0.6f, QColor(0, 0, 0, 31));
        rg.setColorAt(0.9f, Qt::transparent);
        p.fillRect(img.rect(), rg);
        const int sd[4] = { size*0.75f, size*0.8f, size*0.9f, size*0.8f };
        QRect r(0, 0, s, s);
        r.adjust(sd[3], sd[0], -sd[1], -sd[2]);
        p.setBrush(QColor(0, 0, 0, 85));
        p.drawRoundedRect(r.adjusted(-1, -1, 1, 1), 5, 5);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setBrush(Qt::black);
        p.drawRoundedRect(r, 5, 5);
        p.end();

        for (int i = 0; i < 12; ++i)
        {
            if (i < 8)
            {
                QRect r(part(i, size));
                Pixmap x11Pix = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), r.width(), r.height(), 32);
                pix[active][i] = new QPixmap(QPixmap::fromX11Pixmap(x11Pix, QPixmap::ExplicitlyShared));
                pix[active][i]->fill(Qt::transparent);
                QPainter pt(pix[active][i]);
                pt.drawPixmap(pix[active][i]->rect(), QPixmap::fromImage(img).copy(r));
                pt.end();
                data[i] = pix[active][i]->handle();
            }
            else
                data[i] = sd[i-8];
        }
        XHandler::setXProperty<unsigned long>(QX11Info::appRootWindow(), atom[active], XHandler::Long, data, 12);
        shadows = data;
    }
    return shadows;
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

unsigned long
*ShadowHandler::menuShadow(bool up, QMenu *menu, QToolButton *tb)
{
    unsigned long *data = new unsigned long[12];
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

    QPixmap pix(mask.size());
    pix.fill(Qt::transparent);
    p.begin(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Qt::black);
    p.setPen(Qt::NoPen);
    const QPixmap &sh(QPixmap::fromImage(Render::blurred(mask.toImage(), mask.rect(), 8)));
    p.drawTiledPixmap(pix.rect(), Render::colorized(sh, Qt::black));
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.drawTiledPixmap(pix.rect(), mask);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawTiledPixmap(pix.rect(), mask);
    p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
    p.drawRoundedRect(pix.rect().adjusted(sd[3], sd[0], -sd[1], -sd[2]), rnd, rnd);
    p.end();

    for (int i = 0; i < 12; ++i)
    {
        if (i < 8)
        {
            if (menupix[i])
            {
                XFreePixmap(QX11Info::display(), menupix[i]->handle());
                delete menupix[i];
                menupix[i] = 0;
            }
            QRect r(menupart(i, 32, menu->width(), menu->height()));
            Pixmap x11Pix = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), r.width(), r.height(), 32);
            menupix[i] = new QPixmap(QPixmap::fromX11Pixmap(x11Pix, QPixmap::ExplicitlyShared));
            menupix[i]->fill(Qt::transparent);
            QPainter pt(menupix[i]);
            pt.drawPixmap(menupix[i]->rect(), pix.copy(r));
            pt.end();
            data[i] = menupix[i]->handle();
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
        XHandler::setXProperty<unsigned long>(w, XHandler::KwinShadows, XHandler::Long, shadows(active), 12);
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
        XHandler::setXProperty<unsigned long>(m->winId(), XHandler::KwinShadows, XHandler::Long, menuShadow(up, m, tb), 12);
    else
        installShadows(m->winId());
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
    XHandler::deleteXProperty(w->winId(), XHandler::KwinShadows);
}

void
ShadowHandler::removeDelete()
{
    for (int a = 0; a < 2; ++a)
    for (int i = 0; i < 8; ++i)
    {
        if (pix[a][i])
        {
            XFreePixmap(QX11Info::display(), pix[a][i]->handle());
            delete pix[a][i];
            pix[a][i] = 0;
        }
    }
    XHandler::deleteXProperty(QX11Info::appRootWindow(), XHandler::StoreActiveShadow);
    XHandler::deleteXProperty(QX11Info::appRootWindow(), XHandler::StoreInActiveShadow);
}
