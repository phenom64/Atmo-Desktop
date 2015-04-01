
#include <QApplication>
#include <QDebug>
#include "styleconfig.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    StyleConfig sc;
    sc.show();
    return app.exec();
}
