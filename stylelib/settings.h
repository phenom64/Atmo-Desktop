#ifndef SETTINGS_H
#define SETTINGS_H
#include <QStringList>
//settings vars...

#define OPACITY "opacity"
#define BLACKLIST "blacklist"
#define PUSHBTNRND "pushbtn.rnd"
#define TOOLBTNRND "toolbtn.rnd"
#define INPUTRND "input.rnd"
#define SLIDERSIZE "sliders.size"
#define SCROLLERSIZE "scrollers.size"
#define SHADOWOPACITY "shadows.opacity"
#define TABRND "tabs.rnd"

//defaults

#define DEFOPACITY 100
#define DEFBLACKLIST "smplayer"
#define DEFPUSHBTNRND 8
#define DEFTOOLBTNRND 8
#define DEFINPUTRND 8
#define DEFSLIDERSIZE 16
#define DEFSCROLLERSIZE 12
#define DEFSHADOWOPACITY 33
#define DEFTABRND 5

#define READOPACITY OPACITY, DEFOPACITY
#define READBLACKLIST BLACKLIST, DEFBLACKLIST
#define READPUSHBTNRND PUSHBTNRND, DEFPUSHBTNRND
#define READTOOLBTNRND TOOLBTNRND, DEFTOOLBTNRND
#define READINPUTRND INPUTRND, DEFINPUTRND
#define READSLIDERSIZE SLIDERSIZE, DEFSLIDERSIZE
#define READSCROLLERSIZE SCROLLERSIZE, DEFSCROLLERSIZE
#define READSHADOWOPACITY SHADOWOPACITY, DEFSHADOWOPACITY
#define READTABRND TABRND, DEFTABRND

class Q_DECL_EXPORT Settings
{
public:
    float opacity;
    QStringList blackList;
    struct pushbtn
    {
        int rnd;
    } pushbtn;
    struct toolbtn
    {
        int rnd;
    } toolbtn;
    struct input
    {
        int rnd;
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
};

#endif //SETTINGS_H
