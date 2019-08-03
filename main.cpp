#include "graphics.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Graphics w;
    w.show();

    return a.exec();
}
