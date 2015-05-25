#ifndef WINDOWDATA_H
#define WINDOWDATA_H

#include "../config/settings.h"

class QSharedMemory;

class WindowData
{
public:
    enum DataType {     //data is a 32 bit integer
        Separator =     1<<0,       //first 8 bits reserved for booleans...
        ContAware =     1<<1,
        Uno =           1<<2,
        Horizontal =    1<<3,
        Opacity =       0x0000ff00,
        UnoHeight =     0x00ff0000, //the height of the head is never more then 255 right? ..right?
        Buttons =       0x0f000000, //enough long as we dont have more then 15 buttons styles
        Frame =         0xf0000000  //same here... long as we dont set framesize over 15, unsure if this is enough....
    };
    typedef unsigned int Type;
    unsigned int fg, bg, data, pixw, pixh;
    unsigned long pix;
    WindowData() : data(0), fg(0), bg(0), pixw(0), pixh(0), pix(0) {}
//    explicit WindowData(const WindowData &wd) : data(wd.data), fg(wd.fg), bg(wd.bg) {}
    template<typename T> void setValue(const Type type, const T value = T())
    {
        switch (type)
        {
        case Separator:
        case ContAware:
        case Uno:
        case Horizontal: data |= (type*value); break; //just boolean vals...
        case Opacity: data |= ((value << 8) & Opacity); break;
        case UnoHeight: data |= ((value << 16) & UnoHeight); break;
        case Buttons: data |= ((value + 1) << 24); break;
        case Frame: data |= (value << 28); break;
        default: break;
        }
    }
    template<typename T> const T value(const Type type) const
    {
        switch (type)
        {
        case Separator:
        case ContAware:
        case Uno:
        case Horizontal: return T(data & type);
        case Opacity: return T((data & Opacity) >> 8);
        case UnoHeight: return T((data & UnoHeight) >> 16);
        case Buttons: return T(((data & Buttons) >> 24) - 1);
        case Frame: return T((data & Frame) >> 28);
        default: return T();
        }
    }
    inline void operator=(const WindowData &wd) { this->data = wd.data; this->fg = wd.fg; this->bg = wd.bg; }
    inline bool operator==(const WindowData &wd) const { return (this->data == wd.data && this->fg == wd.fg && this->bg == wd.bg); }
    inline bool operator!=(const WindowData &wd) const { return !operator==(wd); }
    inline const bool isValid() const { return fg&&bg; }

    static bool hasData(QObject *parent);
    static QSharedMemory *sharedMemory(const unsigned int wid, QObject *parent, const bool create = true);
    static void setData(const unsigned int wid, QObject *parent, WindowData *wd, const bool force = false);
};

class SharedBgImage
{
public:
    static bool hasData(QObject *parent);
    static QSharedMemory *sharedMemory(const unsigned int wid, QObject *parent, const bool create = true);
    static uchar *data(void *fromData);
    static QSize size(void *fromData);
};

#endif //WINDOWDATA_H
