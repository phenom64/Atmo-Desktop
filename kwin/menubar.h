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
#ifndef MENUBAR_H
#define MENUBAR_H

#include <KDecoration2/DecorationButtonGroup>
#include <KDecoration2/DecorationButton>

class QAction;
class QTimer;
class DBusMenuImporter;
class QMenu;
namespace NSE
{
class Deco;
class MenuBarItem;
class MenuBar : public KDecoration2::DecorationButtonGroup
{
    Q_OBJECT
public:
    MenuBar(Deco *deco, const QString &service, const QString &path);
    ~MenuBar();
    bool hasShownMenues() const;
    void hideAllMenues();
    void startMousePolling();
    void stopMousePolling();
    QAction *actionFromText(const QString &text) const;
    void paint(QPainter *painter, const QRect &repaintArea);

    Deco *deco;
    DBusMenuImporter *importer;
    QTimer *timer;
    MenuBarItem *prevHovered;
    QStringList tbActions;
    int active;

public Q_SLOTS:
    void updateMenu();

Q_SIGNALS:
    void activeChanged();

protected Q_SLOTS:
    void pollMouse();
    void menuUpdated();
};

class MenuBarItem : public KDecoration2::DecorationButton
{
    Q_OBJECT
    friend class MenuBar;
public:
    MenuBarItem(MenuBar *menu, int index);
    ~MenuBarItem();
    void paint(QPainter *painter, const QRect &repaintArea);
    QString text() const;
    virtual QAction *action() const;
    QMenu *menu() const;
    MenuBar *m;
    QRect topLevelGeo() const;
    bool hasGeo;
    int idx;
    void hoverEnter();
    void hoverLeave();

protected:
    void hoverEnterEvent(QHoverEvent *event);
    void hoverLeaveEvent(QHoverEvent *event);

protected slots:
    virtual void click();
};

} //namespace NSE

#endif //MENUBAR_H
