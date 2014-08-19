#ifndef SETTINGS_H
#define SETTINGS_H

//settings vars...

#define PUSHBTNRND "pushbtn.rnd"
#define TOOLBTNRND "toolbtn.rnd"
#define INPUTRND "input.rnd"
#define SLIDERSIZE "sliders.size"
#define SCROLLERSIZE "scrollers.size"

//defaults

#define DEFPUSHBTNRND PUSHBTNRND, 8
#define DEFTOOLBTNRND TOOLBTNRND, 8
#define DEFINPUTRND INPUTRND, 8
#define DEFSLIDERSIZE SLIDERSIZE, 16
#define DEFSCROLLERSIZE SCROLLERSIZE, 12

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
    struct scroller
    {
        int size;
    } scroller;
} Settings;

#endif //SETTINGS_H
