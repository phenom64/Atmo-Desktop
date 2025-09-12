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
#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>
#include "defines.h"
class QToolBar;
#if HASDBUS
#include <QDBusMessage>
#endif
namespace NSE
{
class Q_DECL_EXPORT TitleWidget : public QWidget
{
    Q_OBJECT
public:
    enum TitlePos { Left = 0, Center, Right };
    static void manage(QToolBar *toolBar);
    static bool isManaging(const QToolBar *toolBar);

protected:
    explicit TitleWidget(QToolBar *parent = 0);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void unembedLater();
    void embedLater();
    int visibleKids();
    QWidget *getCenterWidget();
    bool shouldEmbed();

    bool eventFilter(QObject *o, QEvent *e);

protected slots:
    void toolBarMovableChanged(const bool movable);
    void toolBarFloatingChagned(const bool floating);
    void toolBarOrientationChagned(const Qt::Orientation o);
    void toolBarVisibilityChanged(const bool visible);
    void embed();
    void unembed();
    void logicalToggle();
#if HASDBUS
    void macMenuChanged();
    void dataChanged(QDBusMessage msg);
#endif

private:
    qulonglong m_time;
    QToolBar *m_toolBar;
    QAction *m_action;
    bool m_unembedQueued, m_embedQueued, m_logicalQueued;
    int m_visibleCount;
};
}

#endif // TITLEWIDGET_H
