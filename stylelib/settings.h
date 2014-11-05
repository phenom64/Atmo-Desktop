#ifndef SETTINGS_H
#define SETTINGS_H
#include <QStringList>
#include <QPair>
#include <QGradientStop>
//settings vars, these are the vars read from dsp.conf

#define OPACITY             "opacity"
#define BLACKLIST           "blacklist"
#define REMOVETITLE         "removetitlebars"
#define TITLEPOS            "titlepos"
#define HACKDIALOGS         "hackdialogs"
#define COMPACTMENU         "compactmenu"

#define DECOBUTTONS         "deco.buttons"
#define DECOICON            "deco.icon"
#define DECOSHADOWSIZE      "deco.shadowsize"

#define PUSHBTNRND          "pushbtn.rnd"
#define PUSHBTNSHADOW       "pushbtn.shadow"
#define PUSHBTNGRAD         "pushbtn.gradient"
#define PUSHBTNTINT         "pushbtn.tinthue"

#define TOOLBTNRND          "toolbtn.rnd"
#define TOOLBTNSHADOW       "toolbtn.shadow"
#define TOOLBTNGRAD         "toolbtn.gradient"
#define TOOLBTNTINT         "toolbtn.tinthue"
#define TOOLBTNFOLCOL       "toolbtn.followcolors"
#define TOOLBTNINVACT       "toolbtn.invertactive"

#define INPUTRND            "input.rnd"
#define INPUTSHADOW         "input.shadow"
#define INPUTGRAD           "input.gradient"
#define INPUTTINT           "input.tinthue"

#define TABRND              "tabs.rnd"
#define TABSHADOW           "tabs.shadow"
#define TABGRAD             "tabs.gradient"
#define SAFTABRND           "tabs.safrnd"           //safaritabs roundness capped at 8 atm, might change in the future if needed
#define TABCLOSER           "tabs.closebuttonside"

#define UNOGRAD             "uno.gradient"
#define UNOTINT             "uno.tinthue"
#define UNONOISE            "uno.noisefactor"
#define UNOHOR              "uno.horizontal"
#define UNOCONT             "uno.contentaware"
#define UNOOPACITY          "uno.contentopacity"
#define UNOCONTBLUR         "uno.contentblurradius"

#define MENUICONS           "menues.icons"

#define SLIDERSIZE          "sliders.size"
#define SLIDERDOT           "sliders.dot"
#define SLIDERGRAD          "sliders.slidergradient"
#define SLIDERGROOVE        "sliders.groovegradient"
#define SLIDERGROOVESHADOW  "sliders.grooveshadow"
#define SLIDERFILLGROOVE    "sliders.fillgroove"

#define SCROLLERSIZE        "scrollers.size"

#define PROGSHADOW          "progressbars.shadow"
#define PROGRND             "progressbars.rnd"

#define SHADOWOPACITY       "shadows.opacity"

//defaults

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

//these are the values used if no values

#define DEFOPACITY              100
#define DEFBLACKLIST            "smplayer"
#define DEFREMOVETITLE          false
#define DEFTITLEPOS             1
#define DEFHACKDIALOGS          false
#define DEFCOMPACTMENU          false

#define DEFDECOBUTTONS          0
#define DEFDECOICON             true
#define DEFDECOSHADOWSIZE       32

#define DEFPUSHBTNRND           8
#define DEFPUSHBTNSHADOW        3
#define DEFPUSHBTNGRAD          "0.0:5, 1.0:-5"
#define DEFPUSHBTNTINT          "-1:0"

#define DEFTOOLBTNRND           8
#define DEFTOOLBTNSHADOW        3
#define DEFTOOLBTNGRAD          "0.0:5, 1.0:-5"
#define DEFTOOLBTNTINT          "-1:0"
#define DEFTOOLBTNFOLCOL        false
#define DEFTOOLBTNINVACT        false

#define DEFINPUTRND             8
#define DEFINPUTSHADOW          3
#define DEFINPUTGRAD            "0.0:-5, 1.0:5"
#define DEFINPUTTINT            "-1:0"

#define DEFTABRND               4
#define DEFTABSHADOW            3
#define DEFTABGRAD              "0.0:5, 1.0:-5"
#define DEFSAFTABRND            4
#define DEFTABCLOSER            0

#define DEFUNOGRAD              "0.0:5, 1.0:-5"
#define DEFUNOTINT              "-1:0"
#define DEFUNONOISE             10
#define DEFUNOHOR               false
#define DEFUNOCONT              QStringList()
#define DEFUNOOPACITY           10
#define DEFUNOCONTBLUR          2

#define DEFMENUICONS            false

#define DEFSLIDERSIZE           16
#define DEFSLIDERDOT            true
#define DEFSLIDERGRAD           "0.0:5, 1.0:-5"
#define DEFSLIDERGROOVE         "0.0:-5, 1.0:5"
#define DEFSLIDERGROOVESHAD     0
#define DEFSLIDERFILLGROOVE     false


#define DEFSCROLLERSIZE         12

#define DEFPROGSHADOW           3
#define DEFPROGRND              4

#define DEFSHADOWOPACITY        33

#define READOPACITY             OPACITY, DEFOPACITY
#define READBLACKLIST           BLACKLIST, DEFBLACKLIST
#define READREMOVETITLE         REMOVETITLE, DEFREMOVETITLE
#define READTITLEPOS            TITLEPOS, DEFTITLEPOS
#define READHACKDIALOGS         HACKDIALOGS, DEFHACKDIALOGS
#define READCOMPACTMENU         COMPACTMENU, DEFCOMPACTMENU

#define READDECOBUTTONS         DECOBUTTONS, DEFDECOBUTTONS
#define READDECOICON            DECOICON, DEFDECOICON
#define READDECOSHADOWSIZE      DECOSHADOWSIZE, DEFDECOSHADOWSIZE

#define READPUSHBTNRND          PUSHBTNRND, DEFPUSHBTNRND
#define READPUSHBTNSHADOW       PUSHBTNSHADOW, DEFPUSHBTNSHADOW
#define READPUSHBTNGRAD         PUSHBTNGRAD, DEFPUSHBTNGRAD
#define READPUSHBTNTINT         PUSHBTNTINT, DEFPUSHBTNTINT

#define READTOOLBTNRND          TOOLBTNRND, DEFTOOLBTNRND
#define READTOOLBTNSHADOW       TOOLBTNSHADOW, DEFTOOLBTNSHADOW
#define READTOOLBTNGRAD         TOOLBTNGRAD, DEFTOOLBTNGRAD
#define READTOOLBTNTINT         TOOLBTNTINT, DEFTOOLBTNTINT
#define READTOOLBTNFOLCOL       TOOLBTNFOLCOL, DEFTOOLBTNFOLCOL
#define READTOOLBTNINVACT       TOOLBTNINVACT, DEFTOOLBTNINVACT

#define READINPUTRND            INPUTRND, DEFINPUTRND
#define READINPUTSHADOW         INPUTSHADOW, DEFINPUTSHADOW
#define READINPUTGRAD           INPUTGRAD, DEFINPUTGRAD
#define READINPUTTINT           INPUTTINT, DEFINPUTTINT

#define READTABRND              TABRND, DEFTABRND
#define READTABSHADOW           TABSHADOW, DEFTABSHADOW
#define READTABGRAD             TABGRAD, DEFTABGRAD
#define READSAFTABRND           SAFTABRND, DEFSAFTABRND
#define READTABCLOSER           TABCLOSER, DEFTABCLOSER

#define READUNOGRAD             UNOGRAD, DEFUNOGRAD
#define READUNOTINT             UNOTINT, DEFUNOTINT
#define READUNONOISE            UNONOISE, DEFUNONOISE
#define READUNOHOR              UNOHOR, DEFUNOHOR
#define READUNOCONT             UNOCONT, DEFUNOCONT
#define READUNOOPACITY          UNOOPACITY, DEFUNOOPACITY
#define READUNOCONTBLUR         UNOCONTBLUR, DEFUNOCONTBLUR

#define READMENUICONS           MENUICONS, DEFMENUICONS

#define READSLIDERSIZE          SLIDERSIZE, DEFSLIDERSIZE
#define READSLIDERDOT           SLIDERDOT, DEFSLIDERDOT
#define READSLIDERGRAD          SLIDERGRAD, DEFSLIDERGRAD
#define READSLIDERGROOVE        SLIDERGROOVE, DEFSLIDERGROOVE
#define READSLIDERGROOVESHAD    SLIDERGROOVESHADOW, DEFSLIDERGROOVESHAD
#define READSLIDERFILLGROOVE    SLIDERFILLGROOVE, DEFSLIDERGROOVE

#define READSCROLLERSIZE        SCROLLERSIZE, DEFSCROLLERSIZE

#define READPROGSHADOW          PROGSHADOW, DEFPROGSHADOW
#define READPROGRND             PROGRND, DEFPROGRND

#define READSHADOWOPACITY       SHADOWOPACITY, DEFSHADOWOPACITY

typedef QList<QPair<float, int> > Gradient;
typedef QPair<float, int> GradientStop;
typedef QPair<QColor, int> Tint;

class Q_DECL_EXPORT Settings
{
public:
    float opacity;
    QStringList blackList;
    bool removeTitleBars, hackDialogs, compactMenu;
    int titlePos;
    struct deco
    {
        int buttons, shadowSize;
        bool icon;
    } deco;

    struct pushbtn
    {
        int rnd, shadow;
        Gradient gradient;
        Tint tint;
    } pushbtn;
    struct toolbtn
    {
        int rnd, shadow;
        bool folCol, invAct;
        Gradient gradient;
        Tint tint;
    } toolbtn;
    struct input
    {
        int rnd, shadow;
        Gradient gradient;
        Tint tint;
    } input;
    struct sliders
    {
        bool dot, fillGroove;
        int size, grooveShadow;
        Gradient grooveGrad, sliderGrad;
    } sliders;
    struct scrollers
    {
        int size;
    } scrollers;
    struct progressbars
    {
        int shadow, rnd;
    } progressbars;
    struct shadows
    {
        float opacity;
    } shadows;
    struct tabs
    {
        int rnd, safrnd, closeButtonSide, shadow;
        Gradient gradient;
    } tabs;
    struct uno
    {
        Gradient gradient;
        Tint tint;
        unsigned int noise, blur;
        float opacity;
        bool hor, contAware;
    } uno;
    struct menues
    {
        bool icons;
    } menues;

    static Settings conf;

    static QGradientStops gradientStops(const QList<QPair<float, int> > pairs, const QColor &c);
    static QGradientStop pairToStop(const QPair<float, int> pair, const QColor &c);
    static void read();
};

#endif //SETTINGS_H
