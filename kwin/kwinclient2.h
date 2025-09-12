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
#ifndef KWINCLIENT2_H
#define KWINCLIENT2_H

#include <QDBusAbstractAdaptor>
#include <KCModule>
#include <KDecoration2/Decoration>
#include <KDecoration2/DecoratedClient>
#include <QVariant>
#include <QDebug>
#include <netwm_def.h>
#include <KPluginFactory>
#include "../config/settings.h"
#include "../atmolib/windowdata.h"
namespace KDecoration2 { class DecorationButtonGroup; }
class WindowData;
class QPixmap;
class QSharedMemory;

class DSPDecoFactory;
class DSPDecoFactory : public KPluginFactory
{
    Q_OBJECT
    Q_INTERFACES(KPluginFactory)
    Q_PLUGIN_METADATA(IID KPluginFactory_iid FILE "atmo.json")
public:
    explicit DSPDecoFactory();
    ~DSPDecoFactory();

protected slots:
    void shapeCorners();

//protected:
//    QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
//    {
//        qDebug() << iface << parentWidget << parent << args << keyword;
//        return KPluginFactory::create(iface, parentWidget, parent, args, keyword);
//    }
};

namespace NSE
{

class ConfigModule : public KCModule
{
    Q_OBJECT
public:
    ConfigModule(QWidget *parent = 0, const QVariantList &args = QVariantList());
    ~ConfigModule() {}
public slots:
    /**
     * Load the configuration data into the module.
     *
     * The load method sets the user interface elements of the
     * module to reflect the current settings stored in the
     * configuration files.
     *
     * This method is invoked whenever the module should read its configuration
     * (most of the times from a config file) and update the user interface.
     * This happens when the user clicks the "Reset" button in the control
     * center, to undo all of his changes and restore the currently valid
     * settings. It is also called right after construction.
     */
    void load() {}

    /**
     * Save the configuration data.
     *
     * The save method stores the config information as shown
     * in the user interface in the config files.
     *
     * If necessary, this method also updates the running system,
     * e.g. by restarting applications. This normally does not apply for
     * KSettings::Dialog modules where the updating is taken care of by
     * KSettings::Dispatcher.
     *
     * save is called when the user clicks "Apply" or "Ok".
     *
     * If you use KConfigXT, saving is taken care off automatically and
     * you do not need to load manually. However, if you for some reason reimplement it and
     * also are using KConfigXT, you must call this function, otherwise the saving of KConfigXT
     * options will not work. Call it at the very end of your reimplementation, to avoid
     * changed() signals getting emitted when you modify widgets.
     */
    void save() {}

    /**
     * Sets the configuration to sensible default values.
     *
     * This method is called when the user clicks the "Default"
     * button. It should set the display to useful values.
     *
     * If you use KConfigXT, you do not have to reimplement this function since
     * the fetching and settings of default values is done automatically. However, if you
     * reimplement and also are using KConfigXT, remember to call the base function at the
     * very end of your reimplementation.
     */
    void defaults() {}
};
class Grip;
class EmbedHandler;
class ButtonGroup;
class ButtonGroupBase;
class MenuBar;

class Deco : public KDecoration2::Decoration
{
    friend class Button;
    friend class Grip;
    friend class EmbeddedWidget;
    friend class EmbeddedButton;
    friend class Data;
    Q_OBJECT
public:
    class Data
    {
    public:
        int noise, btnStyle, illumination, noiseStyle;
        Gradient grad;
        QColor bg, fg;
        bool separator, icon, menubar;
        void operator =(const Data &d)
        {
            noise = d.noise;
            grad = d.grad;
            bg = d.bg;
            fg = d.fg;
            separator = d.separator;
            btnStyle = d.btnStyle;
            illumination = d.illumination;
            noiseStyle = d.noiseStyle;
            icon = d.icon;
            menubar = d.menubar;
        }
        static void addDataForWinClass(const QString &winClass, QSettings &s);
        static void readWindowData();
        static void decoData(const QString &winClass, Deco *d);
        static QMap<QString, Data> s_data;
    };
    explicit Deco(QObject *parent = 0, const QVariantList &args = QVariantList());
    ~Deco();
    void paint(QPainter *painter, const QRect &repaintArea);
    bool event(QEvent *event);
    const int titleHeight() const;
    void setTitleHeight(const int h);
    const int border();
    const quint32 winId() const { return client().data()->windowId(); }
//    QSharedData<KDecoration2::DecorationShadow> shadow() const;

public slots:
    /**
     * This method gets invoked from the framework once the Decoration is created and
     * completely setup.
     *
     * An inheriting class should override this method and perform all initialization in
     * this method instead of the constructor.
     **/
    void init();

    void updateData();

signals:
    void dataChanged();

protected slots:
    void updateLayout();
    void activeChanged(const bool active);
    void captionChanged(const QString &caption) { update(); }
    void maximizedChanged(const bool max);
    void checkForDataFromWindowClass();

protected:
    void setButtonsVisible(const bool visible);
    void recalculate();
    void reconfigure();
    void hoverEnter();
    void hoverLeave();

    void updateBgPixmap();
    WindowData getShm();
    void paintBevel(QPainter *painter, const int bgLum);
    void paintBling(QPainter *painter, const QRect &r);
    const QRect titleTextArea() const;
    const QPainterPath blingPath(const quint8 style, const QRectF &r, const int radius) const;

    void wheelEvent(QWheelEvent *event);

    void removeEmbedder();

private:
    KDecoration2::DecorationButtonGroup *m_leftButtons, *m_rightButtons;
    ButtonGroupBase *m_buttonManager;
    EmbedHandler *m_embedder;
    QPixmap m_pix, m_bevelCorner[3], *m_bling, m_bgPix;
    QSharedMemory *m_mem;
    QColor m_bg, m_fg, m_textBg, m_textFg, m_minColor, m_maxColor, m_closeColor;
    QFont m_font;
    Gradient m_gradient, m_buttonGradient, m_windowGradient;
    QGradientStops m_winGradient;
    Grip *m_grip;
    int m_prevLum, m_noise, m_buttonStyle, m_tries, m_bevel;
    QRect m_textRect;
    quint8 m_illumination, m_textBevOpacity, m_shadowOpacity, m_opacity, m_uno, m_titleHeight, m_noiseStyle;
    bool m_separator
    , m_isHovered
    , m_contAware
    , m_blingEnabled
    , m_icon
    , m_isDark
    , m_hor
    , m_embedButtons
    , m_followDecoShadow
    , m_showMenuBar
    , m_hasSharedMem;
#if HASDBUSMENU
    MenuBar *m_menuBar;
    friend class MenuBar;
    friend class MenuBarItem;
#endif
};

class DecoAdaptor;
class AdaptorManager : public QObject
{
    Q_OBJECT
public:
    static AdaptorManager *instance();
    inline void addDeco(NSE::Deco *d) { m_decos << d; }
    inline void removeDeco(NSE::Deco *d) { m_decos.removeOne(d); }
    inline void updateData(uint win)
    {
        for (int i = 0; i < m_decos.count(); ++i)
        {
            NSE::Deco *d = m_decos.at(i);
            if (d->client().data()->windowId() == win)
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
            NSE::Deco *d = m_decos.at(i);
            if (d->client().data()->windowId() == win)
            {
                d->update();
                return;
            }
        }
    }
    void windowChanged(uint win, bool active);
    void dataChanged(uint win);

protected:
    AdaptorManager();
    ~AdaptorManager();

private:
    static AdaptorManager *s_instance;
    QList<NSE::Deco *> m_decos;
    DecoAdaptor *m_adaptor;
};

class Grip : public QWidget
{
    friend class Deco;
    Q_OBJECT
public:
    Grip(Deco *d);
    enum Data { Margin = -1, Size = 13 };

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void restack();
    void setColor(const QColor &c);
    void regenPix();
    static QPolygon shape();

protected slots:
    void updatePosition();

private:
    Deco *m_deco;
    QPixmap m_pix;
};

} //NSE
#endif //KWINCLIENT2_H
