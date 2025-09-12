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
#ifndef HANDLERS_H
#define HANDLERS_H

#include <QWidget>
#include <new>
#include <QSharedMemory>
#include <QApplication>
#include <QMap>
#include <QLabel>
#include "gfx.h"
#include "widgets.h"

#if HASDBUS
#include <QDBusMessage>
#endif

class QToolBar;
class QMainWindow;
class QAbstractScrollArea;
class QTabBar;
class QToolButton;
class QDockWidget;

#define HASSTRETCH  "NSE_hasstretch"
#define TOOLPADDING "NSE_toolpadder"
#define TOOLTIMER   "NSE_toolbartimer"
#define TIMERNAME   "NSE_windowupdater"
#define MENUPRESS   "NSE_menupress"

namespace NSE
{

namespace Handlers
{

class Q_DECL_EXPORT ToolBar : public QObject
{
    Q_OBJECT
public:
    ~ToolBar(){}
    static ToolBar *instance();
    static void manageToolBar(QToolBar *tb);
    static void manage(QWidget *child);
    static bool isArrowPressed(const QToolButton *tb);
    static Sides sides(const QToolButton *btn);
    static void queryToolBarLater(QToolBar *bar);
    static bool isDirty(QToolBar *bar);
    static void setDirty(QToolBar *bar);
    static void changeLater(QWidget *w);

protected:
    ToolBar(QObject *parent = 0):QObject(parent){}
    void checkForArrowPress(QToolButton *tb, const QPoint pos);
    bool eventFilter(QObject *, QEvent *);

protected slots:
    void queryToolBar(qulonglong toolbar);
    void toolBarDeleted(QObject *toolBar);
    void toolBtnDeleted(QObject *toolBtn);
    void change(const qulonglong w);

private:
    static ToolBar *s_instance;
    static QMap<QToolButton *, Sides> s_sides;
    static QMap<QToolBar *, bool> s_dirty;
    static QMap<QToolBar *, QAction *> s_spacers;
};

class Q_DECL_EXPORT Window : public QObject
{
    Q_OBJECT
public:
    ~Window(){}
    static Window *instance();
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static void addCompactMenu(QWidget *w);
    static bool drawUnoPart(QPainter *p, QRect r, const QWidget *w, QPoint offset = QPoint());

    static void applyTranslucency(QWidget *widget);
    static void applyBlur(QWidget *widget);

public slots:
    void updateDecoBg(QWidget *w);

signals:
    void windowDataChanged(QWidget *win);

protected:
    Window(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *);

protected slots:
    void menuShow();

private:
    static Window *s_instance;
    QList<QWidget *> m_menuWins;
};

class Q_DECL_EXPORT Drag : public QObject
{
    Q_OBJECT
public:
    static Drag *instance();
    static void manage(QWidget *w);
    static void release(QWidget *w);
    static bool canDrag(QWidget *w);
//    Drag(QObject *parent);
    ~Drag(){}
protected:
    Drag(QObject *parent = 0):QObject(parent){}
    bool eventFilter(QObject *, QEvent *);
private:
    static Drag s_instance;
};

class Q_DECL_EXPORT ScrollWatcher : public QObject
{
    Q_OBJECT
public:
    ~ScrollWatcher(){}
    static void watch(QAbstractScrollArea *area);
    static inline ScrollWatcher *instance() { return &s_instance; }
    static void drawContBg(QPainter *p, QWidget *w, const QRect r, const QPoint &offset = QPoint());
    static void detachMem(QMainWindow *win);

protected:
    ScrollWatcher(){}
    bool eventFilter(QObject *, QEvent *);
    void regenBg(QMainWindow *win);
    static QSharedMemory *mem(QMainWindow *win);

protected slots:
    void updateWin(QWidget *mainWin);
    void vpDeleted(QObject *vp);

private:
    static ScrollWatcher s_instance;
    static QMap<QMainWindow *, QSharedMemory *> s_mem;
};

class Q_DECL_EXPORT Balloon : public QWidget
{
    Q_OBJECT
public:
    Balloon();
    ~Balloon();
    void setToolTipText(const QString &text);
    inline QString toolTipText() const { return m_label->text(); }

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
    void updateShadow();
    void genPixmaps();

private:
    QLabel *m_label;
};

class Q_DECL_EXPORT BalloonHelper : public QObject
{
    Q_OBJECT
public:
    enum Mode { Left = 0, Top, Right, Bottom };
    static BalloonHelper *instance() { return &s_instance; }
    static void manage(QWidget *w);
    static void release(QWidget *w) { w->removeEventFilter(instance()); }
    static Mode mode();
    static void updateBallon();
    static Balloon *balloon();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    static BalloonHelper s_instance;
};

class Q_DECL_EXPORT Dock : public QObject
{
    Q_OBJECT
public:
    static void manage(QWidget *win);
    static void release(QWidget *win);
    static Dock *instance();

protected:
    void lockDock(QDockWidget *dock);
    void unlockDock(QDockWidget *dock);

protected slots:
    void lockWindowLater(const qulonglong w);
    void lockDocks(const bool locked);

private:
    static Dock *s_instance;
};

} //namespace Handlers
} //namespace NSE

#endif //HANDLERS_H
