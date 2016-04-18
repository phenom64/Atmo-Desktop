
#include <QApplication>
#include <QDebug>
#include <typeinfo>
#include <QImageReader>
#include <QList>
#include <QString>
#include "styleconfig.h"
#include "settings.h"
#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setw

enum Task { WriteDefaults = 0, Edit, IconPaths, PrintIconThemes, PrintInfo, ListVars, ShadowInfo, GenHighlight, Invalid };

Task getTask(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (!qstrcmp(argv[1], "--writedefaults"))
            return WriteDefaults;
        if (!qstrcmp(argv[1], "--edit"))
            return Edit;
        if (!qstrcmp(argv[1], "--listiconpaths"))
            return IconPaths;
        if (!qstrcmp(argv[1], "--listiconthemes"))
            return PrintIconThemes;
        if (!qstrcmp(argv[1], "--printinfo") && argc > 2)
            return PrintInfo;
        if (!qstrcmp(argv[1], "--listvars"))
            return ListVars;
        if (!qstrcmp(argv[1], "--shadowinfo") && argc > 2)
            return ShadowInfo;
        if (!qstrcmp(argv[1], "--genhighlight") && argc > 2)
            return GenHighlight;

    }
    return Invalid;
}

static void printShadowInfo(const int shadow)
{
    qDebug() << DSP::Settings::shadowDescription(shadow);
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
    qDebug()  << "Description: "     << DSP::Settings::description(k)
              << "\nValue:       "   << DSP::Settings::readVal(k).value<T>()
              << "\nDefault:     "   << DSP::Settings::defaultValue(k).value<T>()
              << "\nType:        "   << stylishedTypeName(typeName);
}

static void printHelp()
{
    qDebug()  << "Options:\n"
              << "--writedefaults                 Write default values to dsp.conf\n"
              << "--edit                          Open dsp.conf for editing in your default text editor\n"
              << "--printinfo <var>               Print information about <var>\n"
              << "--listvars                      List available variables\n"
              << "--listiconpaths                 List paths where qt looks for icons\n"
              << "--listiconthemes                List available icon themes\n"
              << "--shadowinfo <int>              print info about shadow <int>";
}

static void printVars()
{
    int colW(0);
    for (int i = 0; i < DSP::Settings::Keycount; ++i)
    {
        int col = strlen(DSP::Settings::key((DSP::Settings::Key)i));
        if (col > colW)
            colW = col;
    }
    colW += 4; //some nice spacing...
    for (int i = 0; i < DSP::Settings::Keycount; ++i)
    {
#if 0
        QString line;
        line.append(DSP::Settings::key((DSP::Settings::Key)i));
        line.append("\t\t");
        line.append("Value: ");
        line.append(DSP::Settings::readVal((DSP::Settings::Key)i).toString());
        line.append("\t\t");
        line.append("Default: ");
        line.append(DSP::Settings::defaultValue((DSP::Settings::Key)i).toString());
        line.append("\t\t");
        line.append("Description: ");
        line.append(DSP::Settings::description((DSP::Settings::Key)i));
        line.append("\n");
        std::cout << line.toLatin1().data();
#endif
        std::cout << std::left
                  << std::setw(colW)
                  << DSP::Settings::key((DSP::Settings::Key)i)
//                  << "Value: "
//                  << std::setw(40)
//                  << DSP::Settings::readVal((DSP::Settings::Key)i).toString().toLatin1().data()
//                  << "Default: "
//                  << std::setw(30)
//                  << DSP::Settings::defaultValue((DSP::Settings::Key)i).toString().toLatin1().data()
//                  << "Description: "
                  << DSP::Settings::description((DSP::Settings::Key)i)
                  << std::endl;
    }
    std::cout << std::endl
              << "use --printinfo <varname> for more info"
              << std::endl;
}

static void genHighlight(const QString &file)
{
    QImageReader ir(file);
    if (!ir.canRead())
    {
        std::cout << "image" << file.toLocal8Bit().data() << "is null or doesnt exist, exiting...";
        return;
    }
//    static const int sz(256);
//    QSize scaledSize(ir.size());
//    if (scaledSize.isValid() && qMax(scaledSize.width(), scaledSize.height()) > sz)
//        scaledSize.scale(sz, sz, Qt::KeepAspectRatio);
//    ir.setScaledSize(scaledSize);
    const QImage img(ir.read());
    const QRgb *pixel = reinterpret_cast<const QRgb *>(img.bits());
    const unsigned int size(img.width()*img.height());

    //rgb average
//    enum { Red = 0, Green, Blue };
//    double rgb[3] = { 0, 0, 0 };
//    for (unsigned int i = 0; i < size; ++i)
//    {
//        const QColor c = QColor::fromRgba(pixel[i]);
//        rgb[Red]    += c.red();
//        rgb[Green]  += c.green();
//        rgb[Blue]   += c.blue();
//    }
//    QColor c(rgb[Red]/size, rgb[Green]/size, rgb[Blue]/size);

    quint8 sat(0), val(0);
    QList<quint16> hues;
    QList<quint8> sats;

    for (unsigned int i = 0; i < size; ++i)
    {
        const QColor c = QColor::fromRgba(pixel[i]);
        if (!c.saturation() || !c.value())
            continue;
        if (c.saturation() > sat)
            sat = c.saturation();
        if (c.value() > val)
            val = c.value();
//        if (c.value()+c.saturation() > satVal)
//            satVal = c.value()+c.saturation();
//        hue += c.hue()*(c.value()/255.0f)*(c.saturation()/255.0f);
        if (c.saturation()+c.value() > 256)
        {
            hues << c.hue();
            sats << c.saturation();
        }
    }
    int activeHue(-1), currentHueCount(0);
    for (int i = 0; i < 360; ++i)
        if (hues.count(i) > currentHueCount)
        {
            currentHueCount = hues.count(i);
            activeHue = i;

        }

    QColor c = QColor::fromHsv(activeHue, sat, val);
    const QString color = QString("#%1").arg(QString::number(c.rgba(), 16).mid(2));
    std::cout << std::endl << "Generated color: " << std::endl << color.toLocal8Bit().data() << std::endl;
}

static void printIconPaths()
{
    const QStringList icons = QIcon::themeSearchPaths();
    std::cout << std::endl << "Qt looks for icons in:" << std::endl << std::endl;
    for (int i = 0; i < icons.size(); ++i)
        std::cout << "\t" << QString(icons.at(i)).toLocal8Bit().data() << std::endl;
}

static void printIconThemes()
{
    const QStringList themes = DSP::Settings::availableIconThemes();
    std::cout << std::endl << "Available icon themes:" << std::endl << std::endl;
    for (int i = 0; i < themes.size(); ++i)
        std::cout << "\t" << QString(themes.at(i)).toLocal8Bit().data() << std::endl;
}

int main(int argc, char *argv[])
{
    const Task t = getTask(argc, argv);
    QApplication app(argc, argv);
    switch (t)
    {
    case WriteDefaults: DSP::Settings::writeDefaults(); break;
    case Edit: DSP::Settings::edit(); break;
    case IconPaths: printIconPaths(); break;
    case PrintIconThemes: printIconThemes(); break;
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
                std::cout << "cant print info for type:" << typeName;
        }
        break;
    }
    case ShadowInfo: printShadowInfo(atoi(argv[2])); break;
    case GenHighlight: genHighlight(argv[2]); break;
    case ListVars: printVars(); break;
    default: printHelp(); break;
    }
    app.quit();
    return 0;
}
