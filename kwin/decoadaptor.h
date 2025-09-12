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
#ifndef DECOADAPTOR_H
#define DECOADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QtGlobal>
#include "kwinclient2.h"

namespace NSE
{
class DecoAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.syndromatic.atmo.deco")

public:
    DecoAdaptor(AdaptorManager *parent = 0) : QDBusAbstractAdaptor(parent), m_manager(parent){}

public slots:
    Q_NOREPLY void updateData(uint win) { m_manager->updateData(win); }
    Q_NOREPLY void updateDeco(uint win) { m_manager->updateDeco(win); }

signals:
     void windowActiveChanged(uint win, bool active);
     void dataChanged(uint win);

private:
    AdaptorManager *m_manager;
};
} //namespace

#endif //DECOADAPTOR_H
