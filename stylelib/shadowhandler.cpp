#include <QPixmap>
#include <QX11Info>
#include <QPainter>
#include <QEvent>
#include <QImage>

#include "shadowhandler.h"
#include "../styleproject.h"
#include "ops.h"

class ShowCatcher : public QObject
{
protected:
    bool eventFilter(QObject *o, QEvent *e)
    {
        if (e->type() == QEvent::Show)
            if (QWidget *w = qobject_cast<QWidget *>(o))
                if (w->testAttribute(Qt::WA_WState_Created) || w->internalWinId())
                    ShadowHandler::installShadows(w->winId());
        return false;
    }
};

static ShowCatcher *showCatcher = 0;
static QPixmap *pix[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

static QRect part(int part, int size)
{
    const int d(1);
    switch (part)
    {
    case 0: return QRect(size, 0, d, size);
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
*ShadowHandler::shadows(const int size)
{
    unsigned long *shadows = XHandler::getXProperty<unsigned long>(QX11Info::appRootWindow(), XHandler::StoreShadow);
    if (!shadows)
    {
        qDebug() << "regenerating shadow data...";
        unsigned long *data = new unsigned long[12];

        int s(size*2+1);
        QImage img(s, s, QImage::Format_ARGB32);
        img.fill(Qt::transparent);

        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::NoBrush);
        QRadialGradient rg(QRectF(img.rect()).center()+QPointF(0.0f, 0.5f), size);
        rg.setColorAt(0.0f, QColor(0, 0, 0, 255));
        rg.setColorAt(0.33f, QColor(0, 0, 0, 64));
        rg.setColorAt(0.66f, QColor(0, 0, 0, 16));
        rg.setColorAt(1.0f, Qt::transparent);
        p.fillRect(img.rect(), rg);
        const int sd[4] = { size*0.75f, size*0.8f, size*0.9f, size*0.8f };
        QRect r(0, 0, s, s);
        r.adjust(sd[3], sd[0], -sd[1], -sd[2]);
        p.setBrush(QColor(0, 0, 0, 96));
        p.drawRoundedRect(r.adjusted(-1, -1, 1, 1), 5, 5);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setBrush(Qt::black);
        p.drawRoundedRect(r, 4, 4);
        p.end();

        for (int i = 0; i < 12; ++i)
        {
            if (i < 8)
            {
                QRect r(part(i, size));
                Pixmap x11Pix = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), r.width(), r.height(), 32);
                pix[i] = new QPixmap(QPixmap::fromX11Pixmap(x11Pix, QPixmap::ExplicitlyShared));
                pix[i]->fill(Qt::transparent);
                QPainter pt(pix[i]);
                pt.drawTiledPixmap(pix[i]->rect(), QPixmap::fromImage(img).copy(r));
                pt.end();
                data[i] = pix[i]->handle();
            }
            else
                data[i] = sd[i-8];
        }
        XHandler::setXProperty<unsigned long>(QX11Info::appRootWindow(), XHandler::StoreShadow, data, 12);
        shadows = data;
    }
    return shadows;
}

void
ShadowHandler::installShadows(WId w)
{
    if (w != QX11Info::appRootWindow())
        XHandler::setXProperty<unsigned long>(w, XHandler::KwinShadows, shadows(), 12);
}

void
ShadowHandler::manage(QWidget *w)
{
    if (!showCatcher)
        showCatcher = new ShowCatcher();
    w->removeEventFilter(showCatcher);
    w->installEventFilter(showCatcher);
}

void
ShadowHandler::removeDelete()
{
    if (showCatcher)
    {
        delete showCatcher;
        showCatcher = 0;
    }
    for (int i = 0; i < 8; ++i)
    {
        if (pix[i])
        {
            XFreePixmap(QX11Info::display(), pix[i]->handle());
            delete pix[i];
            pix[i] = 0;
        }
    }
    XHandler::deleteXProperty(QX11Info::appRootWindow(), XHandler::StoreShadow);
}


