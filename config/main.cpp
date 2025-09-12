/* This file is a part of the Atmo desktop experience framework project for SynOS .
 * Copyright (C) 2025 Syndromatic Ltd. All rights reserved
 * Designed by Kavish Krishnakumar in Manchester.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITH ABSOLUTELY NO WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**************************************************************************
*   Based on styleproject, Copyright (C) 2013 by Robert Metsaranta        *
*   therealestrob@gmail.com                                              *
***************************************************************************/

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
#include <QIcon>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QFileInfo>

#ifndef ATMO_VERSION_STR
#define ATMO_VERSION_STR "1.0.0"
#endif
static const char *ATMO_VERSION = ATMO_VERSION_STR;

enum Task { WriteDefaults = 0, Edit, IconPaths, PrintIconThemes, PrintInfo, ListVars, ShadowInfo, GenHighlight, PrintVersion, NSEConfigOptions, Invalid };

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
        if (!qstrcmp(argv[1], "-v") || !qstrcmp(argv[1], "--version"))
            return PrintVersion;
        if (!qstrcmp(argv[1], "--nseconfig-options"))
            return NSEConfigOptions;

    }
    return Invalid;
}

static void printShadowInfo(const int shadow)
{
    qDebug() << NSE::Settings::shadowDescription(shadow);
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
static void printInfo(NSE::Settings::Key k, const char *typeName)
{
    qDebug()  << "Description: "     << NSE::Settings::description(k)
              << "\nValue:       "   << NSE::Settings::readVal(k).value<T>()
              << "\nDefault:     "   << NSE::Settings::defaultValue(k).value<T>()
              << "\nType:        "   << stylishedTypeName(typeName);
}

static void printHelp()
{
    qDebug()  << "Options:\n"
              << "--writedefaults                 Write default values to NSE.conf\n"
              << "--edit                          Open NSE.conf for editing in your default text editor\n"
              << "--printinfo <var>               Print information about <var>\n"
              << "--listvars                      List available variables\n"
              << "--listiconpaths                 List paths where qt looks for icons\n"
              << "--listiconthemes                List available icon themes\n"
              << "--shadowinfo <int>              print info about shadow <int>\n"
              << "--nseconfig-options             Elaborate guidance for NSE.conf\n"
              << "-v, --version                   Print version";
}

static void printVars()
{
    int colW(0);
    for (int i = 0; i < NSE::Settings::Keycount; ++i)
    {
        int col = strlen(NSE::Settings::key((NSE::Settings::Key)i));
        if (col > colW)
            colW = col;
    }
    colW += 4; //some nice spacing...
    for (int i = 0; i < NSE::Settings::Keycount; ++i)
    {
#if 0
        QString line;
        line.append(NSE::Settings::key((NSE::Settings::Key)i));
        line.append("\t\t");
        line.append("Value: ");
        line.append(NSE::Settings::readVal((NSE::Settings::Key)i).toString());
        line.append("\t\t");
        line.append("Default: ");
        line.append(NSE::Settings::defaultValue((NSE::Settings::Key)i).toString());
        line.append("\t\t");
        line.append("Description: ");
        line.append(NSE::Settings::description((NSE::Settings::Key)i));
        line.append("\n");
        std::cout << line.toLatin1().data();
#endif
        std::cout << std::left
                  << std::setw(colW)
                  << NSE::Settings::key((NSE::Settings::Key)i)
//                  << "Value: "
//                  << std::setw(40)
//                  << NSE::Settings::readVal((NSE::Settings::Key)i).toString().toLatin1().data()
//                  << "Default: "
//                  << std::setw(30)
//                  << NSE::Settings::defaultValue((NSE::Settings::Key)i).toString().toLatin1().data()
//                  << "Description: "
                  << NSE::Settings::description((NSE::Settings::Key)i)
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
    const QStringList themes = NSE::Settings::availableIconThemes();
    std::cout << std::endl << "Available icon themes:" << std::endl << std::endl;
    for (int i = 0; i < themes.size(); ++i)
        std::cout << "\t" << QString(themes.at(i)).toLocal8Bit().data() << std::endl;
}

static void printHeader()
{
    std::cout << "Configuration tool for the Atmo Desktop Experience Framework (NSE)" << std::endl;
    std::cout << "TM & (C) 2025. Syndromatic Ltd. All rights reserved." << std::endl;
    std::cout << "Designed by Kavish Krishnakumar in Manchester." << std::endl << std::endl;
}

static void printNSEConfigOptions()
{
    std::cout << "The file" << std::endl << std::endl;
    std::cout << "The configuration is done by editing the file ~/.config/NSE/NSE.conf.\n";
    std::cout << "You'll need to create it (or run --writedefaults)." << std::endl << std::endl;
    std::cout << "Variables" << std::endl << std::endl;
    std::cout << "These all come under the [General] section in the file." << std::endl << std::endl;
    std::cout << "opacity=100 (integer)\n  Opacity of the UNO parts of the window, 0 fully translucent, 100 fully opaque\n\n";
    std::cout << "blacklist=smplayer (stringlist)\n  Applications that should not get translucency (e.g. video players).\n\n";
    std::cout << "removetitlebars=false (boolean)\n  Mac-like hack to hide titlebars and show window buttons inside toolbars.\n\n";
    std::cout << "titlepos=1 (integer)\n  Title alignment when removetitlebars=true. 0=Left 1=Center 2=Right\n\n";
    std::cout << "hackdialogs=false (boolean)\n  Mac-like dialog alignment and appearance without titlebars.\n\n";
    std::cout << "compactmenu=false (boolean)\n  Make the menu into a toolbutton in the toolbar.\n\n";
    std::cout << "splitterext=false (boolean)\n  Extends tiny splitter handles when hovered to make them easier to grab.\n\n";
    std::cout << "maxarrowsize=9 (integer)\n  Maximum size of arrows in various places.\n\n";
    std::cout << "Window Decoration (deco.*)\n  deco.buttons=0 (integer: 0..3)\n    0=Yosemite flat, 1=Lion/Mavericks glassy, 2=Sunken, 3=Carved\n  deco.icon=false (boolean)\n  deco.shadowsize=64 (integer)\n\n";
    std::cout << "Pushbuttons (pushbtn.*)\n  pushbtn.rnd, pushbtn.shadow, pushbtn.gradient, pushbtn.tinthue\n\n";
    std::cout << "ToolButtons (toolbtn.*)\n  toolbtn.rnd, toolbtn.shadow, toolbtn.gradient, toolbtn.activegradient,\n  toolbtn.tinthue, toolbtn.invertactive, toolbtn.followcolors, toolbtn.flat\n\n";
    std::cout << "Inputs (input.*)\n  input.rnd, input.shadow, input.gradient, input.tinthue\n\n";
    std::cout << "Tabs (tabs.*)\n  tabs.safrnd, tabs.rnd, tabs.shadow, tabs.gradient, tabs.closebuttonside\n\n";
    std::cout << "UNO (uno.*)\n  uno.gradient, uno.tinthue, uno.noisefactor, uno.noisestyle, uno.horizontal,\n  uno.contentaware, uno.contentopacity, uno.contentblurradius\n\n";
    std::cout << "Menues (menues.*), ScrollBars (scrollers.*), Sliders (sliders.*),\n  ProgressBars (progressbars.*), Shadows (shadows.*) are similarly structured.\n\n";
    std::cout << "For full details, see atmo_config --listvars and --printinfo <var>." << std::endl;
}

static bool copyDefaultNSEConf()
{
    const QString userDir = QDir::homePath() + "/.config/NSE";
    QDir().mkpath(userDir);
    const QString userFile = userDir + "/NSE.conf";
    const QString templateFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("atmo/NSE.conf"));
    if (templateFile.isEmpty())
        return false;
    // Overwrite with template defaults
    QFile::remove(userFile);
    return QFile::copy(templateFile, userFile);
}

int main(int argc, char *argv[])
{
    const Task t = getTask(argc, argv);
    QApplication app(argc, argv);
    printHeader();
    switch (t)
    {
    case WriteDefaults:
    {
        if (!copyDefaultNSEConf())
        {
            qDebug() << "Falling back to internal defaults (could not locate /usr/share/atmo/NSE.conf).";
            NSE::Settings::writeDefaults();
        }
        else
            qDebug() << "Wrote default NSE.conf from template.";
        break;
    }
    case Edit: NSE::Settings::edit(); break;
    case IconPaths: printIconPaths(); break;
    case PrintIconThemes: printIconThemes(); break;
    case PrintVersion: std::cout << "Atmo Config version " << ATMO_VERSION << std::endl; break;
    case NSEConfigOptions: printNSEConfigOptions(); break;
    case PrintInfo:
    {
        NSE::Settings::Key k = NSE::Settings::key(argv[2]);
        if (k != NSE::Settings::Invalid)
        {
            const char *typeName = NSE::Settings::defaultValue(k).typeName();
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
