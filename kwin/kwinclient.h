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
#ifndef KWINCLIENT_H
#define KWINCLIENT_H

#include <QWidget>
#include <kdecoration.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSharedMemory>

#include "sizegrip.h"
#include "factory.h"
#include "../atmolib/widgets.h"
#include "../config/settings.h"

namespace NSE
{
class Factory;
class KwinClient;
class SizeGrip;

class DButton : public ButtonBase, public QSpacerItem
{
public:
    DButton(const ButtonBase::Type &t, KwinClient *client = 0);
    ~DButton(){}

    QSize sizeHint() const { return QSize(16, 16); }
    const QRect buttonRect() const { return geometry(); }

protected:
    void onClick(const Qt::MouseButton &button);
    void hoverChanged();

    const bool isActive() const;
    const bool isMaximized() const;
    const bool onAllDesktops() const;
    const bool keepAbove() const;
    const bool keepBelow() const;
    const bool shade() const;

    const QColor color(const ColorRole &c) const;
    const bool isDark() const;

private:
    KwinClient *m_client;
};

class SharedWindowData;
class KwinClient : public KDecoration
{
    Q_OBJECT
public:
    class Data
    {
    public:
        int noise;
        Gradient grad;
        QColor bg, fg;
        bool separator;
        void operator =(const Data &d)
        {
            noise = d.noise;
            grad = d.grad;
            bg = d.bg;
            fg = d.fg;
            separator = d.separator;
        }
        static void decoData(const QString &winClass, KwinClient *d);
        static QMap<QString, Data> s_data;
    };

    enum CustomColors { Text = 0, Bg, CustColCount };
    typedef QList<DButton *> Buttons;
    KwinClient(KDecorationBridge *bridge, Factory *factory);
    ~KwinClient();

    // functions not implemented in KDecoration
    void init();
    void activeChange();
    void borders(int &left, int &right, int &top, int &bottom) const;
    void captionChange();
    void desktopChange() {}
    void iconChange() {}
    void maximizeChange();
    QSize minimumSize() const;
    KDecorationDefines::Position mousePosition(const QPoint &point) const;
    void resize(const QSize &s);
    void shadeChange() {}
    void reset(unsigned long changed);
    void update();
    void updateButtons();
    void updateBgPixmap();
    void updateData();

protected:
    bool eventFilter(QObject *, QEvent *);
    void paint(QPainter &p);
    void populate(const QString &buttons, int &size);
    void updateMask();
    void checkForDataFromWindowClass();
    QColor bgColor() const;
    QColor fgColor() const;
    const QRect positionRect(const KDecorationDefines::Position pos) const;
    const int titleHeight() const;

protected slots:
    void readCompositing();
    void memoryDestroyed(QObject *);

private:
    QHBoxLayout *m_titleLayout;
    QLinearGradient m_unoGradient;
    QColor m_bg, m_fg;
    Factory *m_factory;
    QPixmap m_bevelCorner[3], m_pix;
    int m_leftButtons, m_rightButtons, m_frameSize, m_prevLum, m_opacity;
    unsigned int m_noise;
    bool m_separator, m_contAware, m_uno, m_compositingActive, m_hor;
    friend class SizeGrip;
    SizeGrip *m_sizeGrip;
    friend class DButton;
    Buttons m_buttons;
    QSharedMemory *m_mem;
    Gradient m_gradient;
    SharedWindowData *m_wd;
};

class AdaptorManager : public QObject
{
    Q_OBJECT
public:
    static AdaptorManager *instance();
    inline void addDeco(KwinClient *d) { m_decos << d; }
    inline void removeDeco(KwinClient *d) { m_decos.removeOne(d); }
    inline const bool hasDecos() const { return !m_decos.isEmpty(); }
    inline void updateData(uint win)
    {
        for (int i = 0; i < m_decos.count(); ++i)
        {
            KwinClient *d = m_decos.at(i);
            if (d->windowId() == win)
            {
                d->updateData();
                return;
            }
        }
    }
    inline void updateDeco(uint win)
    {
        for (int i = 0; i < m_decos.count(); ++i)
        {
            KwinClient *d = m_decos.at(i);
            if (d->windowId() == win)
            {
                d->update();
                return;
            }
        }
    }

protected:
    AdaptorManager();
    ~AdaptorManager();

private:
    static AdaptorManager *s_instance;
    QList<KwinClient *> m_decos;
};
} //NSE

#endif //KWINCLIENT_H
