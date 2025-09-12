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
#ifndef OVERLAY_H
#define OVERLAY_H

#include <QWidget>
#include <QTimer>
#include "namespace.h"

class QFrame;
class QScrollBar;

namespace NSE
{
class Overlay;
class OverlayHandler : public QObject
{
    Q_OBJECT
public:
    OverlayHandler();
    static OverlayHandler *instance();
    static void manage(Overlay *o);

protected:
    bool eventFilter(QObject *, QEvent *);
    void scheduleUpdate();

protected slots:
    void overlayDeleted();
    void updateOverlays();

private:
    bool m_hasScheduled;
    static OverlayHandler *s_instance;
};

class Overlay : public QWidget
{
    Q_OBJECT
public:
    Overlay(QWidget *parent = 0, int opacity = 0);
    ~Overlay();
    static bool manage(QWidget *frame, int opacity);
    static bool release(QWidget *frame);
    static Overlay *overlay(const QWidget *frame, const bool recursive = false);
    static bool isSupported(const QWidget *frame);
    inline Sides &sides() { return m_sides; }

protected:
    void paintEvent(QPaintEvent *);
    QRegion mask() const;
    bool eventFilter(QObject *o, QEvent *e);
    void removeSide(const Side s);
    void addSide(const Side s);
    bool isSplitter(QWidget *w, const Position p);

private slots:
    void updateOverlay();

private:
    int m_alpha;
    bool m_hasFocus, m_shown;
    Sides m_sides;
    QWidget *m_frame;
    QWidget *m_window;
    QPoint m_position[PosCount];
};

class Restorer : public QTimer
{
    Q_OBJECT
public:
    Restorer(qulonglong widget);
public slots:
    void restore();
private:
    qulonglong m_widget;

};

}

#endif //OVERLAY_H
