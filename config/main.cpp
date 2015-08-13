
#include <QApplication>
#include <QDebug>
#include <typeinfo>
#include "styleconfig.h"
#include "settings.h"

enum Task { WriteDefaults = 0, Edit, PrintInfo, ListVars, Invalid };

Task getTask(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (!qstrcmp(argv[1], "--writedefaults"))
            return WriteDefaults;
        if (!qstrcmp(argv[1], "--edit"))
            return Edit;
        if (!qstrcmp(argv[1], "--printinfo") && argc > 2)
            return PrintInfo;
        if (!qstrcmp(argv[1], "--listvars"))
            return ListVars;
    }
    return Invalid;
}

static const char *stylishedTypeName(const char *typeName)
{
    if (!qstrcmp(typeName, "QStringList"))
        return "stringList";
    else if (!qstrcmp(typeName, "QString"))
        return "string";
    else if (!qstrcmp(typeName, "int"))
        return "integer";
    else if (!qstrcmp(typeName, "float"))
        return "float";
    else if (!qstrcmp(typeName, "bool"))
        return "boolean";
    else
        return "unknown";
}

template<typename T>
static void printInfo(DSP::Settings::Key k, const char *typeName)
{
    qDebug() << "Description: " << DSP::Settings::description(k)
             << "\nValue:       " << DSP::Settings::readVal(k).value<T>()
             << "\nDefault:     " << DSP::Settings::defaultValue(k).value<T>()
             << "\nType:        " << stylishedTypeName(typeName);
}

static void printHelp()
{
    qDebug() << "Options:\n"
             << "--writedefaults                 Write default values to dsp.conf\n"
             << "--edit                          Open dsp.conf for editing in your default text editor\n"
             << "--printinfo <var>               Print information about <var>\n"
             << "--listvars                      List available variables";
}

static void printVars()
{
    for (int i = 0; i < DSP::Settings::Keycount; ++i)
        qDebug() << DSP::Settings::key((DSP::Settings::Key)i);
}

int main(int argc, char *argv[])
{
    const Task t = getTask(argc, argv);
    QApplication app(argc, argv);
    switch (t)
    {
    case WriteDefaults:
        DSP::Settings::writeDefaults();
        break;
    case Edit:
        DSP::Settings::edit();
        break;
    case PrintInfo:
    {
        DSP::Settings::Key k = DSP::Settings::key(argv[2]);
        if (k != DSP::Settings::Invalid)
        {
            const char *typeName = DSP::Settings::defaultValue(k).typeName();
            if (!qstrcmp(typeName, "QStringList"))
                printInfo<QStringList>(k, typeName);
            else if (!qstrcmp(typeName, "QString"))
                printInfo<QString>(k, typeName);
            else if (!qstrcmp(typeName, "int"))
                printInfo<int>(k, typeName);
            else if (!qstrcmp(typeName, "float"))
                printInfo<float>(k, typeName);
            else if (!qstrcmp(typeName, "bool"))
                printInfo<bool>(k, typeName);
            else
                qDebug() << "cant print info for type:" << typeName;
        }
        break;
    }
    case ListVars:
        printVars();
        break;
    default: printHelp(); break;
    }
    app.quit();
    return 0;
}
