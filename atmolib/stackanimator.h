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
#ifndef STACKANIMATOR_H
#define STACKANIMATOR_H

#include <QObject>
#include <QPixmap>

class QStackedLayout;
class QTimer;
class QWidget;
namespace NSE
{
class Q_DECL_EXPORT StackAnimator : public QObject
{
    Q_OBJECT
public:
    enum Info { Steps = 10 };
    StackAnimator(QObject *parent = 0);
    ~StackAnimator();
    static void manage(QStackedLayout *l);

protected:
    bool eventFilter(QObject *o, QEvent *e);
    static void dumpWidget(QPixmap &pix, QWidget *w);
    static void drawRecursive(QPainter *p, QWidget *w);

protected slots:
    void currentChanged(int i);
    void widgetRemoved(int i);
    void animate();
    void activate();

private:
    QTimer *m_timer;
    QStackedLayout *m_stack;
    QPixmap m_prevPix, m_activePix, m_pix;
    QWidget *m_widget;
    int m_step, m_prevIndex;
    bool m_isActivated;
};
} //namespace

#endif //STACKANIMATOR_H
