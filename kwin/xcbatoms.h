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
#ifndef XCBATOMS_H
#define XCBATOMS_H

#include <QDebug>
#include <QScopedPointer>
#include <QString>
#include <cstring>
#include <xcb/xcb_atom.h>
#include <xcb/xproto.h>
#include "../atmolib/qtx11extras_compat.h"
#include <qwindowdefs.h>

namespace Xcb
{

class Atom
{
public:
    Atom(const char *atomName)
    {
        xcb_connection_t *connection = QX11Info::connection();
        if (!connection)
            return;
        xcb_intern_atom_cookie_t c = xcb_intern_atom(connection, false, strlen(atomName), atomName);
        m_reply.reset(xcb_intern_atom_reply(connection, c, 0));
    }
    operator xcb_atom_t() const { return m_reply ? m_reply.data()->atom: 0; }

private:
    QScopedPointer<xcb_intern_atom_reply_t, QScopedPointerPodDeleter> m_reply;
};

class Property
{
public:
    Property(const char *atomName, const WId window)
    {
        xcb_connection_t *connection = QX11Info::connection();
        if (!connection)
            return;
        Atom atom(atomName);
        if (atom)
        {
            xcb_get_property_cookie_t cookie = xcb_get_property_unchecked(connection, false, window, atom, XCB_ATOM_ANY, 0, 0xffffffff);
            m_reply.reset(xcb_get_property_reply(connection, cookie, 0));
        }
    }
    operator bool () const { return length(); }
    int length() const { return m_reply ? xcb_get_property_value_length(m_reply.data()) : 0; }
    QString toString() const { return value() ? QString::fromLocal8Bit(static_cast<char *>(value()), length()) : QString(); }

protected:
    void *value() const {return m_reply ? xcb_get_property_value(m_reply.data()) : 0; }

private:
    QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter> m_reply;
};

} //namespace Xcb
#endif // XCBATOMS_H
