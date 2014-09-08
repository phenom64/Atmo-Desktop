#ifndef SETTINGS_H
#define SETTINGS_H
#include <QStringList>
#include <QPair>
#include <QGradientStop>
//settings vars...

#define OPACITY             "opacity"
#define BLACKLIST           "blacklist"
#define REMOVETITLE         "removetitlebars"
#define HACKDIALOGS         "hackdialogs"
#define TITLEBUTTONS        "titlebuttons"
#define PUSHBTNRND          "pushbtn.rnd"
#define PUSHBTNSHADOW       "pushbtn.shadow"
#define PUSHBTNGRAD         "pushbtn.gradient"
#define TOOLBTNRND          "toolbtn.rnd"
#define TOOLBTNSHADOW       "toolbtn.shadow"
#define TOOLBTNGRAD         "toolbtn.gradient"
#define INPUTRND            "input.rnd"
#define INPUTSHADOW         "input.shadow"
#define INPUTGRAD           "input.gradient"
#define SLIDERSIZE          "sliders.size"
#define SCROLLERSIZE        "scrollers.size"
#define SHADOWOPACITY       "shadows.opacity"
#define TABRND              "tabs.rnd"

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

#define DEFOPACITY              100
#define DEFBLACKLIST            "smplayer"
#define DEFREMOVETITLE          false
#define DEFHACKDIALOGS          false
#define DEFTITLEBUTTONS         0
#define DEFPUSHBTNRND           8
#define DEFPUSHBTNSHADOW        3
#define DEFPUSHBTNGRAD          "0.1:5, 1.0:-5"
#define DEFTOOLBTNRND           8
#define DEFTOOLBTNSHADOW        3
#define DEFTOOLBTNGRAD          "0.1:5, 1.0:-5"
#define DEFINPUTRND             8
#define DEFINPUTSHADOW          3
#define DEFINPUTGRAD            "0.1:-5, 1.0:5"
#define DEFSLIDERSIZE           16
#define DEFSCROLLERSIZE         12
#define DEFSHADOWOPACITY        33
#define DEFTABRND               5

#define READOPACITY             OPACITY, DEFOPACITY
#define READBLACKLIST           BLACKLIST, DEFBLACKLIST
#define READREMOVETITLE         REMOVETITLE, DEFREMOVETITLE
#define READHACKDIALOGS         HACKDIALOGS, DEFHACKDIALOGS
#define READTITLEBUTTONS        TITLEBUTTONS, DEFTITLEBUTTONS
#define READPUSHBTNRND          PUSHBTNRND, DEFPUSHBTNRND
#define READPUSHBTNSHADOW       PUSHBTNSHADOW, DEFPUSHBTNSHADOW
#define READPUSHBTNGRAD         PUSHBTNGRAD, DEFPUSHBTNGRAD
#define READTOOLBTNRND          TOOLBTNRND, DEFTOOLBTNRND
#define READTOOLBTNSHADOW       TOOLBTNSHADOW, DEFTOOLBTNSHADOW
#define READTOOLBTNGRAD         TOOLBTNGRAD, DEFTOOLBTNGRAD
#define READINPUTRND            INPUTRND, DEFINPUTRND
#define READINPUTSHADOW         INPUTSHADOW, DEFINPUTSHADOW
#define READINPUTGRAD           INPUTGRAD, DEFINPUTGRAD
#define READSLIDERSIZE          SLIDERSIZE, DEFSLIDERSIZE
#define READSCROLLERSIZE        SCROLLERSIZE, DEFSCROLLERSIZE
#define READSHADOWOPACITY       SHADOWOPACITY, DEFSHADOWOPACITY
#define READTABRND              TABRND, DEFTABRND

class Q_DECL_EXPORT Settings
{
public:
    float opacity;
    QStringList blackList;
    bool removeTitleBars, hackDialogs;
    int titleButtons;
    struct pushbtn
    {
        int rnd;
        int shadow;
        QList<QPair<float, int> > gradient;
    } pushbtn;
    struct toolbtn
    {
        int rnd;
        int shadow;
        QList<QPair<float, int> > gradient;
    } toolbtn;
    struct input
    {
        int rnd;
        int shadow;
        QList<QPair<float, int> > gradient;
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
        int rnd;
    } tabs;
    static Settings conf;

    static QGradientStops gradientStops(const QList<QPair<float, int> > pairs, const QColor &c);
    static QGradientStop pairToStop(const QPair<float, int> pair, const QColor &c);
    static void read();
};

#endif //SETTINGS_H
