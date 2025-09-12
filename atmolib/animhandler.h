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
#ifndef ANIMHANDLER_H
#define ANIMHANDLER_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QTabBar>
#include <QStyle>
#include <QToolButton>

class QAbstractScrollArea;

namespace NSE
{

namespace Anim
{

class Q_DECL_EXPORT Basic : public QObject
{
    Q_OBJECT
public:
    explicit Basic(QObject *parent = 0);
    static Basic *instance();
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static int level(const QWidget *widget) { return instance()->hoverLevel(widget); }

protected:
    bool eventFilter(QObject *, QEvent *);
    int hoverLevel(const QWidget *widget);
    void remove(QWidget *w);

protected slots:
    void removeSender();
    void animate();

private:
    static Basic *s_instance;
    QMap<QWidget *, int> m_vals;
    QTimer *m_timer;
};

class Q_DECL_EXPORT Tabs : public QObject
{
    Q_OBJECT
public:
    typedef int Tab, Level; //for readability only...
    explicit Tabs(QObject *parent = 0);
    static Tabs *instance();
    static void manage(QTabBar *tb);
    static void release(QTabBar *tb);
    static int level(const QTabBar *tb, Tab tab) { return instance()->hoverLevel(tb, tab); }

protected:
    bool eventFilter(QObject *, QEvent *);
    int hoverLevel(const QTabBar *tb, Tab tab);
    void remove(QTabBar *tb);

protected slots:
    void removeSender();
    void animate();

private:
    static Tabs *s_instance;
    QMap<QTabBar *, QMap<Tab, Level> > m_vals;
    QTimer *m_timer;
};

class Q_DECL_EXPORT ToolBtns : public QObject
{
    Q_OBJECT
public:
    typedef int Level, ArrowLevel; //for readability only...
    explicit ToolBtns(QObject *parent = 0);
    static ToolBtns *instance();
    static void manage(QToolButton *tb);
    static void release(QToolButton *tb);
    static int level(const QToolButton *tb, bool arrow = false) { return instance()->hoverLevel(tb, arrow); }

protected:
    bool eventFilter(QObject *, QEvent *);
    int hoverLevel(const QToolButton *tb, bool arrow);
    void remove(QToolButton *tb);

protected slots:
    void removeSender();
    void animate();

private:
    static ToolBtns *s_instance;
    QToolButton *m_hovered;
    QMap<QToolButton *, QPair<Level, ArrowLevel> > m_vals;
    QTimer *m_timer;
};

class Q_DECL_EXPORT ScrollAnimator : public QObject
{
    Q_OBJECT
public:
    static void manage(QAbstractScrollArea *area);

protected:
    ScrollAnimator(QObject *parent = 0);
    bool eventFilter(QObject *o, QEvent *e);
    bool processWheelEvent(QWheelEvent *e);

protected slots:
    void updateScrollValue();

private:
    QTimer *m_timer;
    bool m_up;
    int m_delta, m_step;
};

} //namespace Anim
} //namespace NSE

#endif //ANIMHANDLER_H
