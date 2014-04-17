#include <QPixmap>
#include <QX11Info>
#include <QPainter>
#include <QEvent>
#include <QImage>

#include "shadowhandler.h"

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
        unsigned long *data = new unsigned long[12];

        int s(size*2+1);
        QImage img(s, s, QImage::Format_ARGB32);
        img.fill(Qt::transparent);

        QPainter p(&img);
        QRadialGradient rg(img.rect().center()+QPoint(1, 1), size);
        rg.setColorAt(0.0f, Qt::black);
        rg.setColorAt(0.5f, QColor(0, 0, 0, 64));
        rg.setColorAt(0.7f, QColor(0, 0, 0, 16));
        rg.setColorAt(0.9f, Qt::transparent);
        p.fillRect(img.rect(), rg);
        p.end();

        const int sd[4] = { size*0.75f, size*0.8f, size*0.9f, size*0.8f };
        for (int i = 0; i < 12; ++i)
        {
            if (i < 8)
            {
                QRect r(part(i, size));
                Pixmap x11Pix = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), r.width(), r.height(), 32);
                pix[i] = new QPixmap();
                *pix[i] = QPixmap::fromX11Pixmap(x11Pix, QPixmap::ExplicitlyShared);
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
    XHandler::deleteXProperty(QX11Info::appRootWindow(), XHandler::StoreShadow);
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
}


