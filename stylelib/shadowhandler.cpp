#include <QPixmap>
#include <QX11Info>
#include <QPainter>
#include <QEvent>

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

QPixmap
ShadowHandler::x11Pixmap(const QPixmap &pix)
{
    Pixmap x11Pix = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), pix.width(), pix.height(), 32);
    QPixmap qPix(QPixmap::fromX11Pixmap(x11Pix, QPixmap::ExplicitlyShared));
    qPix.fill(Qt::red);
    QPainter p(&qPix);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawPixmap(0, 0, pix);
    p.end();
    return qPix;
}

static QPixmap *pix[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

unsigned long
*ShadowHandler::shadows(const int size)
{
    int n(0);
    unsigned long *shadows = XHandler::getXProperty<unsigned long>(QX11Info::appRootWindow(), XHandler::StoreShadow, n);
    if (!shadows)
    {
        unsigned long *data = new unsigned long[12];
        for (int i = 0; i < 12; ++i)
        {
            if (i < 8)
            {
                pix[i] = new QPixmap();
                Pixmap x11Pix = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), size, size, 32);
                *pix[i] = QPixmap::fromX11Pixmap(x11Pix, QPixmap::ExplicitlyShared);
                pix[i]->fill(Qt::red);
                data[i] = pix[i]->handle();
            }
            else
                data[i] = size;
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
            delete [] pix[i];
            pix[i] = 0;
        }
    }
}


