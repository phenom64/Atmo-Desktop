
#include <QApplication>
#include <QDebug>
#include "styleconfig.h"

int main(int argc, char *argv[])
{
    char c[256] = { "asdfasdfasdfasdfasdf" };
    printf("%s\n", c);


    QApplication app(argc, argv);
    StyleConfig sc;
    sc.show();
    return app.exec();
}
