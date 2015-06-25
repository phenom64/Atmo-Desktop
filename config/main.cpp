
#include <QApplication>
#include <QDebug>
#include "styleconfig.h"
#include "settings.h"

enum Task { WriteDefaults = 0, Edit, Invalid };

Task getTask(int argc, char *argv[])
{
    Task t(Invalid);
    if (argc == 2)
    {
        if (!qstrcmp(argv[1], "--writedefaults"))
            t = WriteDefaults;
        else if (!qstrcmp(argv[1], "--edit"))
            t = Edit;
    }
    return t;
}

int main(int argc, char *argv[])
{
    const Task t = getTask(argc, argv);
    if (t == Invalid)
        return 0;
    QApplication app(argc, argv);
    switch (t)
    {
    case WriteDefaults:
        Settings::initiate();
        Settings::writeDefaults();
        break;
    case Edit:
        Settings::initiate();
        Settings::edit();
        break;
    default: break;
    }
    app.quit();
    return 0;
}
