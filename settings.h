#ifndef SETTINGS_H
#define SETTINGS_H

//settings vars...

#define PUSHBTNRND "pushbtn.rnd"
#define TOOLBTNRND "toolbtn.rnd"
#define INPUTRND "input.rnd"
#define SLIDERSIZE "sliders.size"
#define SCROLLERSIZE "scrollers.size"
#define SHADOWOPACITY "shadows.opacity"
#define TABRND "tabs.rnd"

//defaults

#define DEFPUSHBTNRND 8
#define DEFTOOLBTNRND 8
#define DEFINPUTRND 8
#define DEFSLIDERSIZE 16
#define DEFSCROLLERSIZE 12
#define DEFSHADOWOPACITY 33
#define DEFTABRND 5

#define READPUSHBTNRND PUSHBTNRND, DEFPUSHBTNRND
#define READTOOLBTNRND TOOLBTNRND, DEFTOOLBTNRND
#define READINPUTRND INPUTRND, DEFINPUTRND
#define READSLIDERSIZE SLIDERSIZE, DEFSLIDERSIZE
#define READSCROLLERSIZE SCROLLERSIZE, DEFSCROLLERSIZE
#define READSHADOWOPACITY SHADOWOPACITY, DEFSHADOWOPACITY
#define READTABRND TABRND, DEFTABRND


typedef struct Settings
{
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
} Settings;

#endif //SETTINGS_H
