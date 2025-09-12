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
#ifndef SHADOWHANDLER_H
#define SHADOWHANDLER_H

#include <QWidget>
#include "xhandler.h"

class QMenu;
class QToolButton;
namespace NSE
{
class Q_DECL_EXPORT ShadowHandler : public QObject
{
//    Q_OBJECT
public:
    static XHandler::XPixmap *shadows(bool active);
    static void installShadows(WId w, bool active = false);
    static void removeShadows(WId w);
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static void removeDelete();
    static ShadowHandler *instance();
//public slots:
    void showShadow(QWidget *w);
    void showShadow(qulonglong widget);
protected:
    bool eventFilter(QObject *, QEvent *);
private:
    static ShadowHandler *m_instance;
};
} //namespace

#endif //SHADOWHANDLER_H
