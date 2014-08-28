#include "widgets.h"
#include <QX11Info>
#include <X11/Xlib.h>
#include "fixx11h.h"
#include <QEvent>
#include <QPainter>

#include "../stylelib/color.h"
#include "../stylelib/xhandler.h"

#define SZ 16
SizeGrip::SizeGrip(KwinClient *parent) : QWidget(0), m_client(parent)
{
    if (!parent || parent->isPreview())
    {
        deleteLater();
        return;
    }
    setCursor( Qt::SizeFDiagCursor );
    setFixedSize(SZ, SZ);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    m_client->widget()->installEventFilter(this);
    const int points[] = { SZ,0, SZ,SZ, 0,SZ };
    setMask(QPolygon(3, points));
    restack();
    repos();
    show();
}

/**
Status XQueryTree(display, w, root_return, parent_return, children_return, nchildren_return)
      Display *display;
      Window w;
      Window *root_return;
      Window *parent_return;
      Window **children_return;
      unsigned int *nchildren_return;

Arguments
display 	Specifies the connection to the X server.
w 	Specifies the window whose list of children, root, parent, and number of children you want to obtain.
root_return 	Returns the root window.
parent_return 	Returns the parent window.
children_return 	Returns the list of children.
nchildren_return 	Returns the number of children.
 */

/**
  Syntax
    XReparentWindow(display, w, parent, x, y)
      Display *display;
      Window w;
      Window parent;
      int x, y;
*/

void
SizeGrip::restack()
{
    WId w(m_client->windowId());
    if (!w)
        return;
    WId root_return(0);
    WId parent_return(0);

    WId *children_return = 0;
    unsigned int nchildren_return;
    XQueryTree(QX11Info::display(), w, &root_return, &parent_return, &children_return, &nchildren_return);
    if (parent_return)
        if (parent_return != root_return)
            if (parent_return != w)
                w = parent_return;

    XReparentWindow(QX11Info::display(), winId(), w, 0, 0);
}

void
SizeGrip::repos()
{
    move(m_client->width()-SZ, m_client->height()-(m_client->m_titleLayout->geometry().height()+SZ));
}

bool
SizeGrip::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::ZOrderChange)
        restack();
    if (o == m_client->widget() && e->type() == QEvent::Resize || e->type() == QEvent::Show)
        repos();
    return QWidget::eventFilter(o, e);
}

void
SizeGrip::mousePressEvent(QMouseEvent *e)
{
    e->accept();
//    m_client->performWindowOperation(KDecorationDefines::ResizeOp);
    XHandler::mwRes(e->globalPos(), m_client->windowId(), true);
}

void
SizeGrip::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
}

void
SizeGrip::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), Color::mid(m_client->options()->color(KDecoration::ColorFont), m_client->options()->color(KDecoration::ColorTitleBar)));
    p.end();
}
