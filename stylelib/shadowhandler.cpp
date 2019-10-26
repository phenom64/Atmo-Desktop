#include <qtextstream.h>
#include <QDebug>
#include <QPixmap>
#include <QPainter>
#include <QEvent>
#include <QImage>
#include <QMenu>
#include <QToolButton>
#include <QApplication>

#include "shadowhandler.h"
#include "xhandler.h"
#include "ops.h"
#include "gfx.h"
#include "../config/settings.h"
#include "macros.h"
#include "fx.h"

#if HASX11 || HASXCB
#include <QX11Info>
static ulong s_root = QX11Info::appRootWindow();
#else
static ulong s_root = 0;
#endif


using namespace DSP;

Q_DECL_EXPORT ShadowHandler *ShadowHandler::m_instance(0);

ShadowHandler
*ShadowHandler::instance()
{
    if (!m_instance)
        m_instance = new ShadowHandler();
    return m_instance;
}

void
ShadowHandler::showShadow(QWidget *w)
{
    if (w->isWindow()
            && w->testAttribute(Qt::WA_WState_Created)
            && w->internalWinId())
        ShadowHandler::installShadows(XHandler::windowId(w));
}

void
ShadowHandler::showShadow(qulonglong widget)
{
    if (QWidget *w = Ops::getChild<QWidget *>(widget))
        showShadow(w);
}

bool
ShadowHandler::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || !o->isWidgetType())
        return false;
    if (e->type() == QEvent::Show
            || e->type() == QEvent::WinIdChange
            || e->type() == QEvent::Create
            || e->type() == QEvent::ShowWindowRequest
            || e->type() == QEvent::WindowStateChange)
        showShadow(static_cast<QWidget *>(o));
    return false;
}

static XHandler::XPixmap pix[2][8] = {{ 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0 }};

static QRect part(int part, int size, int d = 1)
{
    switch (part)
    {
    case 0: return QRect(size,      0,      1,      size);
    case 1: return QRect(size+d,    0,      size,   size);
    case 2: return QRect(size+d,    size,   size,   1);
    case 3: return QRect(size+d,    size+d, size,   size);
    case 4: return QRect(size,      size+d, 1,      size);
    case 5: return QRect(0,         size+d, size,   size);
    case 6: return QRect(0,         size,   size,   1);
    case 7: return QRect(0,         0,      size,   size);
    default: return QRect();
    }
}


XHandler::XPixmap
*ShadowHandler::shadows(bool active)
{
    static XHandler::Value atom[2] = { XHandler::StoreInActiveShadow, XHandler::StoreActiveShadow };
    XHandler::XPixmap *shadows = XHandler::getXProperty<XHandler::XPixmap>(s_root, atom[active]);
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
        rg.setColorAt(0.8f, QColor(0, 0, 0, 15));
        rg.setColorAt(1.0f, Qt::transparent);
        p.fillRect(img.rect(), rg);
        const quint32 sd[4] = { size*0.5f, size*0.7f, size*0.9f, size*0.7f };
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.setBrush(Qt::black);
        p.drawRoundedRect(img.rect().adjusted(sd[3], sd[0], -sd[1], -sd[2]), dConf.deco.shadowRnd, dConf.deco.shadowRnd);
        p.end();

        for (int i = 0; i < 12; ++i)
        {
            if (i < 8)
                data[i] = pix[active][i] = XHandler::x11Pixmap(img.copy(part(i, size)));
            else
                data[i] = sd[i-8];
        }
        XHandler::setXProperty<XHandler::XPixmap>(s_root, atom[active], XHandler::Long, data, 12);
    }
    return XHandler::getXProperty<XHandler::XPixmap>(s_root, atom[active]);
}

void
ShadowHandler::installShadows(WId w, bool active)
{
    if (w != s_root)
        XHandler::setXProperty<XHandler::XPixmap>(w, XHandler::_KDE_NET_WM_SHADOW, XHandler::Long, shadows(active), 12);
}

void
ShadowHandler::removeShadows(WId w)
{
    if (w != s_root)
        XHandler::deleteXProperty(w, XHandler::_KDE_NET_WM_SHADOW);
}

void
ShadowHandler::manage(QWidget *w)
{
    w->removeEventFilter(instance());
    if (w->isWindow())
        w->installEventFilter(instance());
//    if (w->isVisible())
//    {
//        w->hide();
//        QMetaObject::invokeMethod(w, "show", Qt::QueuedConnection);
//    }
//    QMetaObject::invokeMethod(m_instance, "showShadow", Qt::QueuedConnection, Q_ARG(qulonglong, (qulonglong)w));
//    if (w->testAttribute(Qt::WA_WState_Created))
//        installShadows(w->winId(), false);
}

void
ShadowHandler::release(QWidget *w)
{
    w->removeEventFilter(instance());
    if (w->isWindow())
        removeShadows(XHandler::windowId(w));
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
    XHandler::deleteXProperty(s_root, XHandler::StoreActiveShadow);
    XHandler::deleteXProperty(s_root, XHandler::StoreInActiveShadow);
}
