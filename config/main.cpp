
#include <QCoreApplication>
#include <QDebug>
#include "styleconfig.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    if (argc == 2 && !qstrcmp(argv[1], "--writedefaults"))
    {
        qDebug() << "attempting to write default values...";
        Settings::init();
        Settings::writeDefaults();
    }
    app.quit();
    return 0;
}
