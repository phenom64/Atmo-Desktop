/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**************************************************************************
*   Based on styleproject, Copyright (C) 2013 by Robert Metsaranta        *
*   therealestrob@gmail.com                                              *
***************************************************************************/
#include "sizegrip.h"
#include <QX11Info>
#include <X11/Xlib.h>
#include "fixx11h.h"
#include <QEvent>
#include <QPainter>
#include <QTimer>

#include "../atmolib/color.h"
#include "../atmolib/xhandler.h"

#define SZ 8
#define SM 4

using namespace NSE;

SizeGrip::SizeGrip(KwinClient *client) : QWidget(/*client->widget()*/0), m_client(client) //if I parent the widget I get garbled painting when compositing active.... weird.
{
    hide();
    if (!client || client->isPreview() || !client->widget() || !client->windowId())
    {
        deleteLater();
        return;
    }
    setCursor(Qt::SizeFDiagCursor);
    setFixedSize(SZ, SZ);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    const int points[] = { SZ,0, /*SZ,SZ/2, SZ/2,SZ,*/ SZ,SZ, 0,SZ };
    setMask(QPolygon(3, points));
    restack();
    m_client->widget()->installEventFilter(this);
    installEventFilter(this);
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
    if (WId w = m_client->windowId())
    {
        WId root_return(0);
        WId parent_return(0);
        WId *children_return = 0;
        unsigned int nchildren_return;
        XQueryTree(QX11Info::display(), w, &root_return, &parent_return, &children_return, &nchildren_return);
        if (parent_return)
        {
            XReparentWindow(QX11Info::display(), winId(), parent_return, 0, 0);
            repos();
//            XMapWindow(QX11Info::display(), winId());
            show();
        }
    }
    else
    {
        hide();
        deleteLater();
    }
}

QPoint
SizeGrip::thePos() const
{
    int l,t,r,b;
    m_client->borders(l, r, t, b);
    int right(m_client->width()-(l+r)), bottom(m_client->height()-(t+b));
    return QPoint(right-(SZ+SM), bottom-(SZ+SM));
}

int
SizeGrip::xPos() const
{
    return thePos().x();
}

int
SizeGrip::yPos() const
{
    return thePos().y();
}

void
SizeGrip::repos()
{
    move(thePos());
}


bool
SizeGrip::eventFilter(QObject *o, QEvent *e)
{
    if (o == this && e->type() == QEvent::ZOrderChange)
    {
        removeEventFilter(this);
        restack();
        installEventFilter(this);
        return false;
    }
    if (o == m_client->widget() && e->type() == QEvent::Resize)
        repos();
    return false;
}

void
SizeGrip::mousePressEvent(QMouseEvent *e)
{
    XHandler::mwRes(e->pos(), e->globalPos(), m_client->windowId(), true);
}

void
SizeGrip::mouseReleaseEvent(QMouseEvent *)
{
    m_client->performWindowOperation(KDecoration::NoOp);
}

void
SizeGrip::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.fillRect(rect(), Color::mid(m_client->fgColor(), m_client->bgColor()));
    p.end();
}
