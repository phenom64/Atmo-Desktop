#include <kwindowinfo.h>
#include <QPainter>
#include <QHBoxLayout>

#include "kwinclient.h"

KwinClient::KwinClient(KDecorationBridge *bridge, Factory *factory)
    : KDecoration(bridge, factory)
{
    setParent(factory);
}

void
KwinClient::init()
{
    createMainWidget();
    widget()->installEventFilter(this);
    KWindowInfo info(windowId(), NET::WMWindowType, NET::WM2WindowClass);
    widget()->setLayout(new QHBoxLayout());
}

void
KwinClient::resize(const QSize &s)
{
    widget()->resize(s);
}

void
KwinClient::borders(int &left, int &right, int &top, int &bottom) const
{
    left = 5;
    right = 5;
    top = 20;
    bottom = 5;
}

void
KwinClient::captionChange()
{
    widget()->update();
}

bool
KwinClient::eventFilter(QObject *o, QEvent *e)
{
    if (o != widget())
        return false;
    switch (e->type())
    {
    case QEvent::Paint:
    {
        QPainter p(widget());
        paint(p);
        p.end();
        return true;
    }
    }
    return KDecoration::eventFilter(o, e);
}

void
KwinClient::paint(QPainter &p)
{

}

QSize
KwinClient::minimumSize() const
{
    return QSize(256, 128);
}
