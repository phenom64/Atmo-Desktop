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
#ifndef OPS_H
#define OPS_H

#include <QColor>
#include <QWidget>
#include <QQueue>
#include <QTimer>
#include "gfx.h"
#include <QMainWindow>
#include <QApplication>

/**
 * This class is a mess atm, I just add stuff here
 * that I need outta the way... most of this stuff
 * need to be properly categorised in some classes
 * that actually makes sense, for now, deal w/ it.
 */

class QToolBar;
class QTabBar;
class QToolButton;
class QStyle;
class QStyleOption;
class QStyleOptionToolButton;

namespace NSE
{

class Q_DECL_EXPORT Ops
{
public:
    static QWidget *window(QWidget *w);
    static bool isSafariTabBar(const QTabBar *tabBar);
    static bool isUnoTabBar(const QTabBar *tabBar);
    static QPalette::ColorRole opposingRole(const QPalette::ColorRole &role);
    static QPalette::ColorRole bgRole(const QWidget *w, const QPalette::ColorRole fallBack = QPalette::Window);
    static QPalette::ColorRole fgRole(const QWidget *w, const QPalette::ColorRole fallBack = QPalette::WindowText);
    static void swap(int &t1, int &t2);
    template<typename T> static void swap(T &t1, T &t2)
    {
        const T tmp(t1);
        t1 = t2;
        t2 = tmp;
    }

    template<typename T> static inline bool isOrInsideA(const QWidget *widget)
    {
        if (!widget)
            return false;
        QWidget *w = const_cast<QWidget *>(widget);
        while (w)
        {
            if (qobject_cast<T>(w))
                return true;
            w = w->parentWidget();
        }
        return false;
    }
    template<typename T> static inline T getAncestor(const QWidget *widget)
    {
        if (!widget)
            return 0;
        QWidget *w = const_cast<QWidget *>(widget);
        while (w->parentWidget())
        {
            if (T t = qobject_cast<T>(w))
                return t;
            w = w->parentWidget();
        }
        return 0;
    }

    template<typename T>
    static T getChild(const qulonglong child)
    {
        const QList<QWidget *> widgets = qApp->allWidgets();
        for (int i = 0; i < widgets.count(); ++i)
        {
            QWidget *w(widgets.at(i));
            if (reinterpret_cast<qulonglong>(w) == child)
                return static_cast<T>(w);
        }
        return 0;
    }
};
} //namespace

#endif // OPS_H
