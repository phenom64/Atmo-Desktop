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
#ifndef WINDOWHELPERS_H
#define WINDOWHELPERS_H

#include <QObject>
#include "defines.h"

#if HASDBUS
#include <QDBusMessage>
#endif

namespace NSE
{
class Q_DECL_EXPORT WindowHelpers : public QObject
{
    Q_OBJECT
public:
    static WindowHelpers *instance();
    static void updateWindowDataLater(QWidget *win);
    static bool isActiveWindow(const QWidget *w);
    static bool inUno(const QWidget *w);
    static int unoHeight(QWidget *win, bool includeClientPart = true, bool includeTitleBar = false);

protected:
    explicit WindowHelpers(QObject *parent = 0);
    static bool scheduleWindow(const qulonglong w);
    static unsigned int getHeadHeight(QWidget *win, bool &separator, const int dataHeight);
    static void unoBg(QWidget *win, int &w, int h, const QPalette &pal, uchar *data);

signals:
    void windowDataChanged(QWidget *win);

protected slots:
    void updateWindowData(qulonglong window);
#if HASDBUS
    void dataChanged(QDBusMessage msg);
#endif

private:
    static WindowHelpers *s_instance;
    QList<qulonglong> m_scheduled;
    QMap<QWidget *, int> m_uno;
};
}

#endif // WINDOWHELPERS_H
