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
/*
 *   Virtuality Style for Qt4 and Qt5
 *   Copyright 2009-2014 by Thomas Lübking <thomas.luebking@gmail.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef MAC_MENU_ADAPTOR_H
#define MAC_MENU_ADAPTOR_H

#include <QtDBus/QDBusAbstractAdaptor>
#include "macmenu.h"

namespace BE
{

class MacMenuAdaptor : public QDBusAbstractAdaptor
{
   Q_OBJECT
   Q_CLASSINFO("D-Bus Interface", "org.kde.XBarClient")

private:
   MacMenu *mm;

public:
   MacMenuAdaptor(MacMenu *macMenu) : QDBusAbstractAdaptor(macMenu), mm(macMenu) { }

public slots:
   Q_NOREPLY void activate() { mm->activate(); }
   Q_NOREPLY void deactivate() { mm->deactivate(); }
   Q_NOREPLY void popup(qlonglong key, int idx, int x, int y)
   { mm->popup(key, idx, x, y); }
   Q_NOREPLY void hover(qlonglong key, int idx, int x, int y)
   { mm->hover(key, idx, x, y); }
   Q_NOREPLY void popDown(qlonglong key) { mm->popDown(key); }
   Q_NOREPLY void raise(qlonglong key) { mm->raise(key); }
};
} // namespace

#endif //MAC_MENU_ADAPTOR_H
