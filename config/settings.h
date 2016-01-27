#ifndef SETTINGS_H
#define SETTINGS_H
#include <QStringList>
#include <QPair>
#include <QGradientStop>
#include <QPalette>
#include <QVariant>

class QSettings;
namespace DSP
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

#define dConf DSP::Settings::conf

class Q_DECL_EXPORT Settings
{
public:
    enum Key {
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
        Dfmhacks,
        Decobuttons,
        Decoicon,
        Decoshadowsize,
        Decoframe,
        Decoembedded,
        Pushbtnrnd,
        Pushbtnshadow,
        Pushbtngrad,
        Pushbtntint,
        Toolbtnrnd,
        Toolbtnshadow,
        Toolbtngrad,
        Toolbtntint,
        Toolbtnfolcol,
        Toolbtninvact,
        Toolbtnflat,
        Inputrnd,
        Inputshadow,
        Inputgrad,
        Inputtint,
        Tabsaf,
        Tabrnd,
        Tabshadow,
        Tabgrad,
        Saftabrnd,       //safaritabs Roundness Capped At 8 Atm, Might Change In The Future If Needed
        Tabcloser,
        Unoenabled,
        Unograd,
        Unotint,
        Unonoise,
        Unonoisestyle,
        Unohor,
        Unocont,
        Unoopacity,
        Unocontblur,
        Menuicons,
        Menugrad,
        Menuitemgrad,
        Menuitemshadow,
        Slidersize,
        Sliderdot,
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
        Viewtreelines,
        Viewitemgrad,
        Viewitemshadow,
        Viewitemrnd,
        Viewheadergrad,
        Progshadow,
        Progrnd,
        Progtxthover,
        Progtxtpos,
        Wingrad,
        Winnoise,
        Winnoisestyle,
        Winhor,
        Shadowopacity,
        Shadowillumination,
        Shadowdarkraised,
        Keycount,
        Invalid
    };

    enum AppName { Eiskalt = 0, Konversation, Konsole, KWin, BEShell, Yakuake, Plasma, DFM, SystemSettings, Unspecific }; //app specific hacks should be avoided when possible.
    typedef uint App;
    typedef struct _Conf
    {
        QString appName;
        App app;
        float opacity;
        QStringList blackList;
        bool removeTitleBars, hackDialogs, compactMenu, splitterExt, balloonTips, lockDocks, differentInactive, dfmHacks, animateStack, animateScroll, simpleArrows;
        quint8 titlePos, arrowSize;
        QPalette *palette;
        struct deco
        {
            quint8 buttons, shadowSize, frameSize;
            bool icon, embed;
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
            bool folCol, invAct, flat;
            Gradient gradient;
            Tint tint;
        } toolbtn;
        struct input
        {
            quint8 rnd, shadow;
            Gradient gradient;
            Tint tint;
        } input;
        struct sliders
        {
            bool dot, fillGroove;
            quint8 size, grooveShadow;
            qint16 grooveStyle;
            Gradient grooveGrad, sliderGrad;
        } sliders;
        struct scrollers
        {
            quint8 size, style, grooveShadow;
            qint16 grooveStyle;
            Gradient grooveGrad, sliderGrad;
        } scrollers;
        struct progressbars
        {
            quint8 shadow, rnd, textPos;
            bool txtHover;
        } progressbars;
        struct shadows
        {
            quint8 opacity, illumination;
            bool darkRaisedEdges;
        } shadows;
        struct tabs
        {
            quint8 rnd, safrnd, closeButtonSide, shadow;
            Gradient gradient;
            bool safari;
        } tabs;
        struct uno
        {
            Gradient gradient;
            Tint tint;
            quint8 noise, blur, noiseStyle;
            float opacity;
            bool enabled, hor, contAware;
        } uno;
        struct windows
        {
            Gradient gradient;
            quint8 noise, noiseStyle;
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
            bool treelines;
            Gradient itemGradient, headerGradient;
            quint8 itemShadow, itemRnd;
        } views;
    } Conf;
    static Conf conf;
    static QGradientStops gradientStops(const QList<QPair<float, int> > pairs, const QColor &c);
    static QGradientStop pairToStop(const QPair<float, int> pair, const QColor &c);
    static void edit();

    static void read();
    static void readPalette();
    static Gradient stringToGrad(const QString &string);

    static QSettings *settings();
    static QSettings *paletteSettings();

    static void setFileName(const QString &file);
    static void restoreFileName();

    static void writePalette();
    static void writeDefaults();
    static void writeVal(const Key k, const QVariant v);
    static QVariant readVal(const Key k);
    static const QString description(const Key k);
    static Key key(const QString k);
    static const char *key(const Key k);
    static const QVariant defaultValue(const Key k);
    template<typename T> static inline T readValue(const Key k) { return readVal(k).value<T>(); }
#define READ(_TYPE_, _METHOD_) static const _TYPE_ read##_METHOD_(const Key k) { return readValue<_TYPE_>(k); }
    READ(bool, Bool) READ(int, Int) READ(float, Float) READ(QString, String) READ(QStringList, StringList)
#undef READ
    ~Settings();

protected:
    Settings();
    static void writePaletteColor(QPalette::ColorGroup g, QPalette::ColorRole r, QColor c);
    static QColor readPaletteColor(QPalette::ColorGroup g, QPalette::ColorRole r);
    static Settings *instance();

private:
    QSettings *m_settings, *m_paletteSettings, *m_overrideSettings;
    static Settings *s_instance;
};

} //namespace

#endif //SETTINGS_H
