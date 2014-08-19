#ifndef SETTINGS_H
#define SETTINGS_H

//defaults

#define PUSHBTNRND "pushbtn.rnd", 8
#define TOOLBTNRND "toolbtn.rnd", 8
#define INPUTRND "input.rnd", 8

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
} Settings;

#endif //SETTINGS_H
