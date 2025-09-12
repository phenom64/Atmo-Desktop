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
#ifndef SETTINGS_H
#define SETTINGS_H
#include <QStringList>
#include <QPair>
#include <QGradientStop>
#include <QPalette>
#include <QVariant>

class QSettings;
namespace NSE
{

typedef QList<QPair<float, int> > Gradient;
typedef QPair<float, int> GradientStop;
typedef QPair<QColor, int> Tint;

/**
  * About the gradients....
  * The basic theory is that they are stops in pairs,
  * so you can add as many stops as you will between
  * 0.00 (start) and 1.00 (end). The stops are separated
  * by a comma "," and a stop is grouped by a value in
  * percentage as to make the color darker or lighter
  * then the color set for the widget in systemsettings.
  * so for example a stop 0.1:10 would make the color
  * at the top of the gradient 10 percent lighter and
  * a stop 1.0:-10 would make the color at the bottom (end)
  * of the gradient 10 percent darker. So a complete example
  * of a soft gradient would be "0.1:10, 1.0:-10".
  */

#define dConf NSE::Settings::conf

class Q_DECL_EXPORT Settings
{
public:
    enum Key { ///TODO: convert to camel case
        Opacity = 0,
        Blacklist,
        Removetitle,
        Titlepos,
        Hackdialogs,
        Compactmenu,
        Splitterext,
        Arrowsize,
        Simplearrows,
        Balloontips,
        Palette,
        Animatestack,
        Animatescroll,
        Lockdocks,
        Differentinactive,
        Icontheme,
        Iconpaths,
        Framernd,

        Decobuttons,
        Decoicon,
        Decoshadowsize,
        Decoshadowrnd,
        Decoframe,
        Decoembedded,
        Decomincolor,
        Decomaxcolor,
        Decoclosecolor,

        Pushbtnrnd,
        Pushbtnshadow,
        Pushbtngrad,
        Pushbtntint,

        Toolbtnrnd,
        Toolbtnshadow,
        Toolbtngrad,
        Toolbtnactivegrad,
        Toolbtntint,
        Toolbtnfolcol,
        Toolbtninvact,
        Toolbtnflat,
        Toolbtnmorph,
        Toolbtnnormal,
        Toolbtnmask,

        Inputrnd,
        Inputunornd,
        Inputshadow,
        Inputgrad,
        Inputtint,

        Tabdocstyle,
        Tabselectors,
        Tabsaf,
        Tabrnd,
        Tabshadow,
        Tabgrad,
        Tabbargrad,
        Tabsafrnd,       //safaritabs Roundness Capped At 8 Atm, Might Change In The Future If Needed
        Tabcloser,
        Tabcloserrnd,
        Tabdocwidth,
        Tabinvdoc,
        Tabinvdoccol,

        Unoenabled,
        Unograd,
        Unotint,
        Unonoise,
        Unonoisefile,
        Unonoisestyle,
        Unohor,
        Unocont,
        Unoopacity,
        Unocontblur,
        Unooverlay,

        Menuicons,
        Menugrad,
        Menuitemgrad,
        Menuitemshadow,

        Slidersize,
        Sliderdot,
        Slidermetal,
        Slidergrad,
        Slidergroove,
        Slidergrooveshadow,
        Sliderfillgroove,
        Sliderinvert,

        Scrollersize,
        Scrollerstyle,
        Scrollergrad,
        Scrollergroove,
        Scrollerinvert,
        Scrollergshadow,
        Scrollseparator,

        Viewtreelines,
        Viewitemgrad,
        Viewitemshadow,
        Viewitemrnd,
        Viewheadergrad,
        Viewheadershadow,
        Viewheaderrnd,
        Viewopacity,
        Viewtrad,
        Viewrnd,
        Viewshadow,

        Progshadow,
        Progrnd,
        Progtxthover,
        Progtxtpos,
        Proggrad,
        Progstripe,

        Wingrad,
        Winnoise,
        Winnoisefile,
        Winnoisestyle,
        Winhor,

        Shadowopacity,
        Shadowillumination,
        Shadowdarkraised,
        Shadowontextopacity,

        Keycount,
        Invalid
    };

    enum AppName { Eiskalt = 0, Konversation, Konsole, KWin, BEShell, Yakuake, Plasma, DFM, SystemSettings, Unspecific }; //app specific hacks should be avoided when possible.
    typedef uint App;
    typedef struct _Conf
    {
        QString appName, iconTheme;
        App app;
        int opacity;
        QStringList blackList, iconPaths;
        bool removeTitleBars, hackDialogs, compactMenu, splitterExt, balloonTips, lockDocks, differentInactive, dfmHacks, animateStack, animateScroll, simpleArrows;
        quint8 titlePos, arrowSize, baseSize, frameRnd;
        QPalette *palette;
        struct deco
        {
            quint8 buttons, shadowSize, shadowRnd, frameSize;
            bool icon, embed;
            uint min, max, close;
        } deco;

        struct pushbtn
        {
            quint8 rnd, shadow;
            Gradient gradient;
            Tint tint;
        } pushbtn;
        struct toolbtn
        {
            quint8 rnd, shadow;
            bool folCol, invAct, flat, morph, normal, mask;
            Gradient gradient, activeGradient;
            Tint tint;
        } toolbtn;
        struct input
        {
            quint8 rnd, shadow, unoRnd;
            Gradient gradient;
            Tint tint;
        } input;
        struct sliders
        {
            bool dot, fillGroove, metallic;
            qint8 grooveShadow;
            quint8 size;
            qint16 grooveStyle;
            Gradient grooveGrad, sliderGrad;
        } sliders;
        struct scrollers
        {
            quint8 size, style, grooveShadow;
            qint16 grooveStyle;
            Gradient grooveGrad, sliderGrad;
            bool separator;
        } scrollers;
        struct progressbars
        {
            quint8 shadow, rnd, textPos, stripeSize;
            bool txtHover;
            Gradient gradient;
        } progressbars;
        struct shadows
        {
            quint8 opacity, illumination, onTextOpacity;
            bool darkRaisedEdges;
        } shadows;
        struct tabs
        {
            quint8 rnd, safrnd, closeButtonSide, shadow, docStyle, closeRnd;
            quint16 docWidth;
            Gradient gradient, barGrad;
            bool safari, regular, invDoc, invDocCol;
        } tabs;
        struct uno
        {
            QString noiseFile;
            Gradient gradient;
            Tint tint;
            quint8 noise, blur, overlay;
            qint8 noiseStyle;
            float opacity;
            bool enabled, hor, contAware;
        } uno;
        struct windows
        {
            QString noiseFile;
            Gradient gradient;
            quint8 noise;
            qint8 noiseStyle;
            bool hor;
        } windows;
        struct menues
        {
            bool icons;
            Gradient gradient, itemGradient;
            quint8 itemShadow;
        } menues;
        struct views
        {
            bool treelines, traditional;
            Gradient itemGradient, headerGradient;
            quint8 itemShadow, itemRnd, opacity, viewRnd, viewShadow, headerShadow, headerRnd;
        } views;
    } Conf;
    static Conf conf;
    static QGradientStops gradientStops(const QList<QPair<float, int> > pairs, const QColor &c);
    static QGradientStops gradientStops(const QList<QPair<float, int> > pairs);
    static QGradientStop pairToStop(const QPair<float, int> pair, const QColor &c);
    static QGradientStop pairToStop(const QPair<float, int> pair);
    static void edit();

    static void read();
    static void readPalette();
    static Gradient stringToGrad(const QString &string);

    static QSettings *settings();
    static QSettings *paletteSettings();

    static QStringList availableIconThemes();
//    static QStrignList availableIconPaths();

    static void setFileName(const QString &file);
    static void restoreFileName();

    static void writePalette();
    static void writeDefaults();
    static void writeVal(const Key k, const QVariant v);
    static QVariant readVal(const Key k);
    static const char *description(const Key k);
    static const char *shadowName(const int shadow);
    static const char *shadowDescription(const int shadow);
    static Key key(const QString k);
    static const char *key(const Key k);
    static const QVariant defaultValue(const Key k);
    static void formatIconPathsList(QStringList &paths);
    template<typename T> static inline const T readValue(const Key k) { return readVal(k).value<T>(); }
#define READ(_TYPE_, _METHOD_) static inline const _TYPE_ read##_METHOD_(const Key k) { return readValue<_TYPE_>(k); }
    READ(bool, Bool) READ(int, Int) READ(float, Float) READ(QString, String) READ(QStringList, StringList)
#undef READ
    ~Settings();

protected:
    Settings();
    static void writePaletteColor(QPalette::ColorGroup g, QPalette::ColorRole r, QColor c);
    static QColor readPaletteColor(QPalette::ColorGroup g, QPalette::ColorRole r);
    static Settings *instance();
    static unsigned int readColor(const Key k);

private:
    QSettings *m_settings, *m_presetSettings, *m_paletteSettings, *m_overrideSettings;
    static Settings *s_instance;
};

} //namespace

#endif //SETTINGS_H
