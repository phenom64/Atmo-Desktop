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
#define TITLEBUTTONS        "titlebuttons"

#define PUSHBTNRND          "pushbtn.rnd"
#define PUSHBTNSHADOW       "pushbtn.shadow"
#define PUSHBTNGRAD         "pushbtn.gradient"
#define PUSHBTNTINT         "pushbtn.tinthue"

#define TOOLBTNRND          "toolbtn.rnd"
#define TOOLBTNSHADOW       "toolbtn.shadow"
#define TOOLBTNGRAD         "toolbtn.gradient"
#define TOOLBTNTINT         "toolbtn.tinthue"

#define INPUTRND            "input.rnd"
#define INPUTSHADOW         "input.shadow"
#define INPUTGRAD           "input.gradient"
#define INPUTTINT           "input.tinthue"

#define SLIDERSIZE          "sliders.size"
#define SCROLLERSIZE        "scrollers.size"
#define SHADOWOPACITY       "shadows.opacity"
#define TABRND              "tabs.rnd"
#define TABCLOSER           "tabs.closebuttonside"

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
#define DEFTITLEBUTTONS         0

#define DEFPUSHBTNRND           8
#define DEFPUSHBTNSHADOW        3
#define DEFPUSHBTNGRAD          "0.1:5, 1.0:-5"
#define DEFPUSHBTNTINT          "-1:0"

#define DEFTOOLBTNRND           8
#define DEFTOOLBTNSHADOW        3
#define DEFTOOLBTNGRAD          "0.1:5, 1.0:-5"
#define DEFTOOLBTNTINT          "-1:0"

#define DEFINPUTRND             8
#define DEFINPUTSHADOW          3
#define DEFINPUTGRAD            "0.1:-5, 1.0:5"
#define DEFINPUTTINT            "-1:0"

#define DEFSLIDERSIZE           16
#define DEFSCROLLERSIZE         12
#define DEFSHADOWOPACITY        33
#define DEFTABRND               5
#define DEFTABCLOSER            0

#define READOPACITY             OPACITY, DEFOPACITY
#define READBLACKLIST           BLACKLIST, DEFBLACKLIST
#define READREMOVETITLE         REMOVETITLE, DEFREMOVETITLE
#define READTITLEPOS            TITLEPOS, DEFTITLEPOS
#define READHACKDIALOGS         HACKDIALOGS, DEFHACKDIALOGS
#define READTITLEBUTTONS        TITLEBUTTONS, DEFTITLEBUTTONS

#define READPUSHBTNRND          PUSHBTNRND, DEFPUSHBTNRND
#define READPUSHBTNSHADOW       PUSHBTNSHADOW, DEFPUSHBTNSHADOW
#define READPUSHBTNGRAD         PUSHBTNGRAD, DEFPUSHBTNGRAD
#define READPUSHBTNTINT         PUSHBTNTINT, DEFPUSHBTNTINT

#define READTOOLBTNRND          TOOLBTNRND, DEFTOOLBTNRND
#define READTOOLBTNSHADOW       TOOLBTNSHADOW, DEFTOOLBTNSHADOW
#define READTOOLBTNGRAD         TOOLBTNGRAD, DEFTOOLBTNGRAD
#define READTOOLBTNTINT         TOOLBTNTINT, DEFTOOLBTNTINT

#define READINPUTRND            INPUTRND, DEFINPUTRND
#define READINPUTSHADOW         INPUTSHADOW, DEFINPUTSHADOW
#define READINPUTGRAD           INPUTGRAD, DEFINPUTGRAD
#define READINPUTTINT           INPUTTINT, DEFINPUTTINT

#define READSLIDERSIZE          SLIDERSIZE, DEFSLIDERSIZE
#define READSCROLLERSIZE        SCROLLERSIZE, DEFSCROLLERSIZE
#define READSHADOWOPACITY       SHADOWOPACITY, DEFSHADOWOPACITY
#define READTABRND              TABRND, DEFTABRND
#define READTABCLOSER           TABCLOSER, DEFTABCLOSER

class Q_DECL_EXPORT Settings
{
public:
    float opacity;
    QStringList blackList;
    bool removeTitleBars, hackDialogs;
    int titleButtons, titlePos;
    struct pushbtn
    {
        int rnd, shadow;
        QList<QPair<float, int> > gradient;
        QPair<QColor, int> tint;
    } pushbtn;
    struct toolbtn
    {
        int rnd, shadow;
        QList<QPair<float, int> > gradient;
        QPair<QColor, int> tint;
    } toolbtn;
    struct input
    {
        int rnd, shadow;
        QList<QPair<float, int> > gradient;
        QPair<QColor, int> tint;
    } input;
    struct sliders
    {
        int size;
    } sliders;
    struct scrollers
    {
        int size;
    } scrollers;
    struct shadows
    {
        float opacity;
    } shadows;
    struct tabs
    {
        int rnd, closeButtonSide;
    } tabs;
    static Settings conf;

    static QGradientStops gradientStops(const QList<QPair<float, int> > pairs, const QColor &c);
    static QGradientStop pairToStop(const QPair<float, int> pair, const QColor &c);
    static void read();
};

#endif //SETTINGS_H
