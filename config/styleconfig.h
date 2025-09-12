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
#ifndef STYLECONFIG_H
#define STYLECONFIG_H

#include "ui_config.h"
#include <QWidget>

namespace NSE
{
class StyleConfig : public QWidget
{
    Q_OBJECT
public:
    explicit StyleConfig(QWidget *parent = 0);

public slots:
    //slots that the systemsettings tries and connect to
    void save();
    void defaults();

signals:
    //signals used by the systemsettings
    void changed(bool changed);

protected:
    void readSettings();

private:
    Ui::Config ui;
};
} //namespace

#endif //STYLECONFIG_H
